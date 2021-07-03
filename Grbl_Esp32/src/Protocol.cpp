/*
  Protocol.cpp - controls Grbl execution protocol and procedures
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

    2018 -	Bart Dring This file was modifed for use on the ESP32
                    CPU. Do not use this with Grbl for atMega328P

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Protocol.h"

#include "Machine/MachineConfig.h"
#include "Report.h"         // report_feedback_message
#include "Limits.h"         // limits_get_state
#include "Planner.h"        // plan_get_current_block
#include "MotionControl.h"  // PARKING_MOTION_LINE_NUMBER

static void protocol_exec_rt_suspend();

static char    line[LINE_BUFFER_SIZE];     // Line to be executed. Zero-terminated.
static char    comment[LINE_BUFFER_SIZE];  // Line to be executed. Zero-terminated.
static uint8_t line_flags           = 0;
static uint8_t char_counter         = 0;
static uint8_t comment_char_counter = 0;

struct client_line_t {
    char buffer[LINE_BUFFER_SIZE];
    int  len;
    int  line_number;
};

client_line_t client_lines[CLIENT_COUNT];

static void empty_line(uint8_t client) {
    client_line_t* cl = &client_lines[client];
    cl->len           = 0;
    cl->buffer[0]     = '\0';
}
static void empty_lines() {
    for (uint8_t client = 0; client < CLIENT_COUNT; client++) {
        empty_line(client);
    }
}

Error add_char_to_line(char c, uint8_t client) {
    client_line_t* cl = &client_lines[client];
    // Simple editing for interactive input
    if (c == '\b') {
        // Backspace erases
        if (cl->len) {
            --cl->len;
            cl->buffer[cl->len] = '\0';
        }
        return Error::Ok;
    }
    if (cl->len == (LINE_BUFFER_SIZE - 1)) {
        return Error::Overflow;
    }
    if (c == '\r' || c == '\n') {
        cl->len = 0;
        cl->line_number++;
        return Error::Eol;
    }
    cl->buffer[cl->len++] = c;
    cl->buffer[cl->len]   = '\0';
    return Error::Ok;
}

Error execute_line(char* line, uint8_t client, WebUI::AuthenticationLevel auth_level) {
    Error result = Error::Ok;
    // Empty or comment line. For syncing purposes.
    if (line[0] == 0) {
        return Error::Ok;
    }
    // Grbl '$' or WebUI '[ESPxxx]' system command
    if (line[0] == '$' || line[0] == '[') {
        return system_execute_line(line, client, auth_level);
    }
    // Everything else is gcode. Block if in alarm or jog mode.
    if (sys.state == State::Alarm || sys.state == State::ConfigAlarm || sys.state == State::Jog) {
        return Error::SystemGcLock;
    }
    return gc_execute_line(line, client);
}

bool can_park() {
    if (config->_enableParkingOverrideControl) {
        return sys.override_ctrl == Override::ParkingMotion && homingAxes && !config->_laserMode;
    } else {
        return homingAxes && !config->_laserMode;
    }
}

void protocol_reset() {
    sys_probe_state                      = ProbeState::Off;
    rtStatusReport                       = false;
    rtCycleStart                         = false;
    rtFeedHold                           = false;
    rtReset                              = false;
    rtSafetyDoor                         = false;
    rtMotionCancel                       = false;
    rtSleep                              = false;
    rtCycleStop                          = false;
    sys_rt_exec_accessory_override.value = 0;
    sys_rt_exec_alarm                    = ExecAlarm::None;
    sys_rt_f_override                    = FeedOverride::Default;
    sys_rt_r_override                    = RapidOverride::Default;
    sys_rt_s_override                    = SpindleSpeedOverride::Default;
}

/*
  GRBL PRIMARY LOOP:
*/
void protocol_main_loop() {
    client_reset_read_buffer(CLIENT_ALL);
    empty_lines();
    //uint8_t client = CLIENT_SERIAL; // default client

    // Check for and report alarm state after a reset, error, or an initial power up.
    // NOTE: Sleep mode disables the stepper drivers and position can't be guaranteed.
    // Re-initialize the sleep state as an ALARM mode to ensure user homes or acknowledges.
    if (sys.state == State::ConfigAlarm) {
        report_feedback_message(Message::ConfigAlarmLock);
    } else {
        // Perform some machine checks to make sure everything is good to go.
        if (config->_checkLimitsAtInit && config->_axes->hasHardLimits()) {
            if (limits_get_state()) {
                sys.state = State::Alarm;  // Ensure alarm state is active.
                report_feedback_message(Message::CheckLimits);
            }
        }

        if (sys.state == State::Alarm || sys.state == State::Sleep) {
            report_feedback_message(Message::AlarmLock);
            sys.state = State::Alarm;  // Ensure alarm state is set.
        } else {
            // Check if the safety door is open.
            sys.state = State::Idle;
            if (config->_control->system_check_safety_door_ajar()) {
                rtSafetyDoor = true;
                protocol_execute_realtime();  // Enter safety door mode. Should return as IDLE state.
            }
            // All systems go!
            system_execute_startup();  // Execute startup script.
        }
    }

    // ---------------------------------------------------------------------------------
    // Primary loop! Upon a system abort, this exits back to main() to reset the system.
    // This is also where Grbl idles while waiting for something to do.
    // ---------------------------------------------------------------------------------
    int c;
    for (;;) {
        auto sdcard = config->_sdCard;

        // _readyNext means a line *might* be available from the _SDCard
        // and implies SDCard::State == Busy (which is the same value as BusyPrinting).

        if (sdcard->_readyNext) {
            char fileLine[255];

            sdcard->_readyNext = false;
                // _readyNext is invariantly set to false here.
                // either readFileLine() will succeed, in which case
                // it is up to report_status_message() to reset it,
                // or we are done with EOF or an error and we want to
                // leave it as false.

            SDCard::ReadResult_t rslt = sdcard->readFileLine(fileLine, 255);

            #define DEBUG_PROTOCOL_READFILELINE 0

            if (rslt == SDCard::ReadResult_t::OK) {
                #if DEBUG_PROTOCOL_READFILELINE
                    debug_serial("protocol loop readFileLine returned OK with '%s'",fileLine);
                #endif
                report_status_message(execute_line(fileLine, sdcard->_client, sdcard->_auth_level), sdcard->_client);
                    // will call sdcard->closefile()

            } else if (rslt == SDCard::ReadResult_t::EndOfFile) {
                #if DEBUG_PROTOCOL_READFILELINE
                    debug_serial("protocol loop readFileLine returned EndOfFile");
                #endif
                char temp[50];
                sdcard->get_current_filename(temp);
                grbl_notifyf("SD print done", "%s print is successful", temp);
                sdcard->closeFile();  // close file and clear SD ready/running flags

            } else {
                // there was a problem and the error was already reported
                #if DEBUG_PROTOCOL_READFILELINE
                    debug_serial("protocol loop readFileLine() returned %d for some kind of problem",rslt);
                #endif
                sdcard->closeFile();
            }
        }

        // Receive one line of incoming serial data, as the data becomes available.
        // Filtering, if necessary, is done later in gc_execute_line(), so the
        // filtering is the same with serial and file input.
        uint8_t client = CLIENT_SERIAL;
        char*   line;
        for (client = 0; client < CLIENT_COUNT; client++) {
            while ((c = client_read(client)) != -1) {
                Error res = add_char_to_line(c, client);
                switch (res) {
                    case Error::Ok:
                        break;
                    case Error::Eol:
                        protocol_execute_realtime();  // Runtime command check point.
                        if (sys.abort) {
                            return;  // Bail to calling function upon system abort
                        }
                        line = client_lines[client].buffer;
#ifdef DEBUG_REPORT_ECHO_RAW_LINE_RECEIVED
                        report_echo_line_received(line, client);
#endif
                        // auth_level can be upgraded by supplying a password on the command line
                        report_status_message(execute_line(line, client, WebUI::AuthenticationLevel::LEVEL_GUEST), client);
                        empty_line(client);
                        break;
                    case Error::Overflow:
                        report_status_message(Error::Overflow, client);
                        empty_line(client);
                        break;
                    default:
                        break;
                }
            }  // while serial read
        }      // for clients
        // If there are no more characters in the serial read buffer to be processed and executed,
        // this indicates that g-code streaming has either filled the planner buffer or has
        // completed. In either case, auto-cycle start, if enabled, any queued moves.
        protocol_auto_cycle_start();
        protocol_execute_realtime();  // Runtime command check point.
        if (sys.abort) {
            return;  // Bail to main() program loop to reset system.
        }
        // check to see if we should disable the stepper drivers ... esp32 work around for disable in main loop.
        if (stepper_idle && config->_idleTime != 255) {
            if (esp_timer_get_time() > stepper_idle_counter) {
                config->_axes->set_disable(true);
            }
        }
    }
    return; /* Never reached */
}

// Block until all buffered steps are executed or in a cycle state. Works with feed hold
// during a synchronize call, if it should happen. Also, waits for clean cycle end.
void protocol_buffer_synchronize() {
    // If system is queued, ensure cycle resumes if the auto start flag is present.
    protocol_auto_cycle_start();
    do {
        protocol_execute_realtime();  // Check and execute run-time commands
        if (sys.abort) {
            return;  // Check for system abort
        }
    } while (plan_get_current_block() || (sys.state == State::Cycle));
}

// Auto-cycle start triggers when there is a motion ready to execute and if the main program is not
// actively parsing commands.
// NOTE: This function is called from the main loop, buffer sync, and mc_line() only and executes
// when one of these conditions exist respectively: There are no more blocks sent (i.e. streaming
// is finished, single commands), a command that needs to wait for the motions in the buffer to
// execute calls a buffer sync, or the planner buffer is full and ready to go.
void protocol_auto_cycle_start() {
    if (plan_get_current_block() != NULL) {  // Check if there are any blocks in the buffer.
        rtCycleStart = true;                 // If so, execute them!
    }
}

// This function is the general interface to Grbl's real-time command execution system. It is called
// from various check points in the main program, primarily where there may be a while loop waiting
// for a buffer to clear space or any point where the execution time from the last check point may
// be more than a fraction of a second. This is a way to execute realtime commands asynchronously
// (aka multitasking) with grbl's g-code parsing and planning functions. This function also serves
// as an interface for the interrupts to set the system realtime flags, where only the main program
// handles them, removing the need to define more computationally-expensive volatile variables. This
// also provides a controlled way to execute certain tasks without having two or more instances of
// the same task, such as the planner recalculating the buffer upon a feedhold or overrides.
// NOTE: The sys_rt_exec_state.bit variable flags are set by any process, step or serial interrupts, pinouts,
// limit switches, or the main program.
void protocol_execute_realtime() {
    protocol_exec_rt_system();
    if (sys.suspend.value) {
        protocol_exec_rt_suspend();
    }
}

// Executes run-time commands, when required. This function primarily operates as Grbl's state
// machine and controls the various real-time features Grbl has to offer.
// NOTE: Do not alter this unless you know exactly what you are doing!
static void protocol_do_alarm() {
    switch (sys_rt_exec_alarm) {
        case ExecAlarm::None:
            return;
        // System alarm. Everything has shutdown by something that has gone severely wrong. Report
        case ExecAlarm::HardLimit:
        case ExecAlarm::SoftLimit:
            sys.state = State::Alarm;  // Set system alarm state
            report_alarm_message(sys_rt_exec_alarm);
            report_feedback_message(Message::CriticalEvent);
            rtReset = false;  // Disable any existing reset
            do {
                // Block everything, except reset and status reports, until user issues reset or power
                // cycles. Hard limits typically occur while unattended or not paying attention. Gives
                // the user and a GUI time to do what is needed before resetting, like killing the
                // incoming stream. The same could be said about soft limits. While the position is not
                // lost, continued streaming could cause a serious crash if by chance it gets executed.
                vTaskDelay(1);  // give serial task some time
            } while (!rtReset);
            break;
        default:
            sys.state = State::Alarm;  // Set system alarm state
            report_alarm_message(sys_rt_exec_alarm);
            break;
    }
    sys_rt_exec_alarm = ExecAlarm::None;
}

static void protocol_start_holding() {
    if (!(sys.suspend.bit.motionCancel || sys.suspend.bit.jogCancel)) {  // Block, if already holding.
        st_update_plan_block_parameters();                               // Notify stepper module to recompute for hold deceleration.
        sys.step_control             = {};
        sys.step_control.executeHold = true;  // Initiate suspend state with active flag.
    }
}

static void protocol_cancel_jogging() {
    if (!sys.suspend.bit.motionCancel) {
        sys.suspend.bit.jogCancel = true;
    }
}

static void protocol_hold_complete() {
    sys.suspend.value            = 0;
    sys.suspend.bit.holdComplete = true;
}

static void protocol_do_motion_cancel() {
    // Execute and flag a motion cancel with deceleration and return to idle. Used primarily by probing cycle
    // to halt and cancel the remainder of the motion.
    rtMotionCancel = false;
    rtCycleStart   = false;  // Cancel any pending start

    // MOTION_CANCEL only occurs during a CYCLE, but a HOLD and SAFETY_DOOR may have been initiated
    // beforehand. Motion cancel affects only a single planner block motion, while jog cancel
    // will handle and clear multiple planner block motions.
    switch (sys.state) {
        case State::Alarm:
        case State::ConfigAlarm:
        case State::CheckMode:
            return;  // Do not set motionCancel

        case State::Idle:
            protocol_hold_complete();
            break;

        case State::Cycle:
            protocol_start_holding();
            break;

        case State::Jog:
            protocol_start_holding();
            protocol_cancel_jogging();
            // When jogging, we do not set motionCancel, hence return not break
            return;

        case State::Sleep:
        case State::Hold:
        case State::Homing:
        case State::SafetyDoor:
            break;
    }
    sys.suspend.bit.motionCancel = true;
}

static void protocol_do_feedhold() {
    // Execute a feed hold with deceleration, if required. Then, suspend system.
    rtFeedHold   = false;
    rtCycleStart = false;  // Cancel any pending start
    switch (sys.state) {
        case State::ConfigAlarm:
        case State::Alarm:
        case State::CheckMode:
        case State::SafetyDoor:
        case State::Sleep:
            return;  // Do not change the state to Hold

        case State::Hold:
        case State::Homing:
            break;

        case State::Idle:
            protocol_hold_complete();
            break;

        case State::Cycle:
            protocol_start_holding();
            break;

        case State::Jog:
            protocol_start_holding();
            protocol_cancel_jogging();
            return;  // Do not change the state to Hold
    }
    sys.state = State::Hold;
}

static void protocol_do_safety_door() {
    // Execute a safety door stop with a feed hold and disable spindle/coolant.
    // NOTE: Safety door differs from feed holds by stopping everything no matter state, disables powered
    // devices (spindle/coolant), and blocks resuming until switch is re-engaged.

    rtCycleStart = false;  // Cancel any pending start
    report_feedback_message(Message::SafetyDoorAjar);
    switch (sys.state) {
        case State::ConfigAlarm:
            return;
        case State::Alarm:
        case State::CheckMode:
        case State::Sleep:
            rtSafetyDoor = false;
            return;  // Do not change the state to SafetyDoor

        case State::Hold:
            break;
        case State::Homing:
            sys_rt_exec_alarm = ExecAlarm::HomingFailDoor;
            break;
        case State::SafetyDoor:
            if (!sys.suspend.bit.jogCancel && sys.suspend.bit.initiateRestore) {  // Actively restoring
                // Set hold and reset appropriate control flags to restart parking sequence.
                if (sys.step_control.executeSysMotion) {
                    st_update_plan_block_parameters();  // Notify stepper module to recompute for hold deceleration.
                    sys.step_control                  = {};
                    sys.step_control.executeHold      = true;
                    sys.step_control.executeSysMotion = true;
                    sys.suspend.bit.holdComplete      = false;
                }  // else NO_MOTION is active.

                sys.suspend.bit.retractComplete = false;
                sys.suspend.bit.initiateRestore = false;
                sys.suspend.bit.restoreComplete = false;
                sys.suspend.bit.restartRetract  = true;
            }
            break;
        case State::Idle:
            protocol_hold_complete();
            break;
        case State::Cycle:
            protocol_start_holding();
            break;
        case State::Jog:
            protocol_start_holding();
            protocol_cancel_jogging();
            break;
    }
    if (!sys.suspend.bit.jogCancel) {
        // If jogging, leave the safety door event pending until the jog cancel completes
        rtSafetyDoor = false;
        sys.state    = State::SafetyDoor;
    }
    // NOTE: This flag doesn't change when the door closes, unlike sys.state. Ensures any parking motions
    // are executed if the door switch closes and the state returns to HOLD.
    sys.suspend.bit.safetyDoorAjar = true;
}

static void protocol_do_sleep() {
    rtSleep = false;
    switch (sys.state) {
        case State::ConfigAlarm:
        case State::Alarm:
            sys.suspend.bit.retractComplete = true;
            sys.suspend.bit.holdComplete    = true;
            break;

        case State::Idle:
            protocol_hold_complete();
            break;

        case State::Cycle:
        case State::Jog:
            protocol_start_holding();
            // Unlike other hold events, sleep does not set jogCancel
            break;

        case State::CheckMode:
        case State::Sleep:
        case State::Hold:
        case State::Homing:
        case State::SafetyDoor:
            break;
    }
    sys.state = State::Sleep;
}

static void protocol_do_initiate_cycle() {
    // Start cycle only if queued motions exist in planner buffer and the motion is not canceled.
    sys.step_control = {};  // Restore step control to normal operation
    if (plan_get_current_block() && !sys.suspend.bit.motionCancel) {
        sys.suspend.value = 0;  // Break suspend state.
        sys.state         = State::Cycle;
        st_prep_buffer();  // Initialize step segment buffer before beginning cycle.
        st_wake_up();
    } else {                    // Otherwise, do nothing. Set and resume IDLE state.
        sys.suspend.value = 0;  // Break suspend state.
        sys.state         = State::Idle;
    }
}

// The handlers for rtFeedHold, rtMotionCancel, and rtsDafetyDoor clear rtCycleStart to
// ensure that auto-cycle-start does not resume a hold without explicit user input.
static void protocol_do_cycle_start() {
    rtCycleStart = false;

    // Execute a cycle start by starting the stepper interrupt to begin executing the blocks in queue.

    // Resume door state when parking motion has retracted and door has been closed.
    switch (sys.state) {
        case State::SafetyDoor:
            if (!sys.suspend.bit.safetyDoorAjar) {
                if (sys.suspend.bit.restoreComplete) {
                    sys.state = State::Idle;  // Set to IDLE to immediately resume the cycle.
                } else if (sys.suspend.bit.retractComplete) {
                    // Flag to re-energize powered components and restore original position, if disabled by SAFETY_DOOR.
                    // NOTE: For a safety door to resume, the switch must be closed, as indicated by HOLD state, and
                    // the retraction execution is complete, which implies the initial feed hold is not active. To
                    // restore normal operation, the restore procedures must be initiated by the following flag. Once,
                    // they are complete, it will call CYCLE_START automatically to resume and exit the suspend.
                    sys.suspend.bit.initiateRestore = true;
                }
            }
            break;
        case State::Idle:
            protocol_do_initiate_cycle();
            break;
        case State::Hold:
            // Cycle start only when IDLE or when a hold is complete and ready to resume.
            if (sys.suspend.bit.holdComplete) {
                if (sys.spindle_stop_ovr.value) {
                    sys.spindle_stop_ovr.bit.restoreCycle = true;  // Set to restore in suspend routine and cycle start after.
                } else {
                    protocol_do_initiate_cycle();
                }
            }
            break;
        case State::ConfigAlarm:
        case State::Alarm:
        case State::CheckMode:
        case State::Sleep:
        case State::Cycle:
        case State::Homing:
        case State::Jog:
            break;
    }
}

static void protocol_do_cycle_stop() {
    rtCycleStop = false;

    switch (sys.state) {
        case State::Hold:
        case State::SafetyDoor:
        case State::Sleep:
            // Reinitializes the cycle plan and stepper system after a feed hold for a resume. Called by
            // realtime command execution in the main program, ensuring that the planner re-plans safely.
            // NOTE: Bresenham algorithm variables are still maintained through both the planner and stepper
            // cycle reinitializations. The stepper path should continue exactly as if nothing has happened.
            // NOTE: rtCycleStop is set by the stepper subsystem when a cycle or feed hold completes.
            if (!sys.soft_limit && !sys.suspend.bit.jogCancel) {
                // Hold complete. Set to indicate ready to resume.  Remain in HOLD or DOOR states until user
                // has issued a resume command or reset.
                plan_cycle_reinitialize();
                if (sys.step_control.executeHold) {
                    sys.suspend.bit.holdComplete = true;
                }
                sys.step_control.executeHold      = false;
                sys.step_control.executeSysMotion = false;
                break;
            }
            // Fall through
        case State::ConfigAlarm:
        case State::Alarm:
        case State::CheckMode:
        case State::Idle:
        case State::Cycle:
        case State::Homing:
        case State::Jog:
            // Motion complete. Includes CYCLE/JOG/HOMING states and jog cancel/motion cancel/soft limit events.
            // NOTE: Motion and jog cancel both immediately return to idle after the hold completes.
            if (sys.suspend.bit.jogCancel) {  // For jog cancel, flush buffers and sync positions.
                sys.step_control = {};
                plan_reset();
                st_reset();
                gc_sync_position();
                plan_sync_position();
            }
            if (sys.suspend.bit.safetyDoorAjar) {  // Only occurs when safety door opens during jog.
                sys.suspend.bit.jogCancel    = false;
                sys.suspend.bit.holdComplete = true;
                sys.state                    = State::SafetyDoor;
            } else {
                sys.suspend.value = 0;
                sys.state         = State::Idle;
            }
            break;
    }
}

static void protocol_execute_overrides() {
    // Execute overrides.
    if ((sys_rt_f_override != sys.f_override) || (sys_rt_r_override != sys.r_override)) {
        sys.f_override         = sys_rt_f_override;
        sys.r_override         = sys_rt_r_override;
        sys.report_ovr_counter = 0;  // Set to report change immediately
        plan_update_velocity_profile_parameters();
        plan_cycle_reinitialize();
    }

    // NOTE: Unlike motion overrides, spindle overrides do not require a planner reinitialization.
    if (sys_rt_s_override != sys.spindle_speed_ovr) {
        sys.step_control.updateSpindleSpeed = true;
        sys.spindle_speed_ovr               = sys_rt_s_override;
        sys.report_ovr_counter              = 0;  // Set to report change immediately

        // XXX this might not be necessary if the override is processed at the right level
        // If spindle is on, tell it the RPM has been overridden
        if (gc_state.modal.spindle != SpindleState::Disable) {
            spindle->setState(gc_state.modal.spindle, gc_state.spindle_speed);
        }
    }

    if (sys_rt_exec_accessory_override.bit.spindleOvrStop) {
        sys_rt_exec_accessory_override.bit.spindleOvrStop = false;
        // Spindle stop override allowed only while in HOLD state.
        // NOTE: Report counters are set in spindle->spinDown()
        if (sys.state == State::Hold) {
            if (sys.spindle_stop_ovr.value == 0) {
                sys.spindle_stop_ovr.bit.initiate = true;
            } else if (sys.spindle_stop_ovr.bit.enabled) {
                sys.spindle_stop_ovr.bit.restore = true;
            }
        }
    }

    // NOTE: Since coolant state always performs a planner sync whenever it changes, the current
    // run state can be determined by checking the parser state.
    if (sys_rt_exec_accessory_override.bit.coolantFloodOvrToggle) {
        sys_rt_exec_accessory_override.bit.coolantFloodOvrToggle = false;
        if (config->_coolant->hasFlood() && (sys.state == State::Idle || sys.state == State::Cycle || sys.state == State::Hold)) {
            gc_state.modal.coolant.Flood = !gc_state.modal.coolant.Flood;
            config->_coolant->set_state(gc_state.modal.coolant);  // Report counter set in coolant_set_state().
        }
    }
    if (sys_rt_exec_accessory_override.bit.coolantMistOvrToggle) {
        sys_rt_exec_accessory_override.bit.coolantMistOvrToggle = false;

        if (config->_coolant->hasMist() && (sys.state == State::Idle || sys.state == State::Cycle || sys.state == State::Hold)) {
            gc_state.modal.coolant.Mist = !gc_state.modal.coolant.Mist;
            config->_coolant->set_state(gc_state.modal.coolant);  // Report counter set in coolant_set_state().
        }
    }
}

void protocol_do_macro(int macro_num) {
    // must be in Idle
    if (sys.state != State::Idle) {
        info_serial("Macro button only permitted in idle");
        return;
    }

    config->_macros->run_macro(macro_num);
}

void protocol_exec_rt_system() {
    protocol_do_alarm();  // If there is a hard or soft limit, this will block until rtReset is set

    if (rtReset) {
        if (sys.state == State::Homing) {
            sys_rt_exec_alarm = ExecAlarm::HomingFailReset;
        }
        // Execute system abort.
        sys.abort = true;  // Only place this is set true.
        return;            // Nothing else to do but exit.
    }

    if (rtStatusReport) {
        rtStatusReport = false;
        report_realtime_status(CLIENT_ALL);
    }

    if (rtMotionCancel) {
        protocol_do_motion_cancel();
    }

    if (rtFeedHold) {
        protocol_do_feedhold();
    }

    if (rtSafetyDoor) {
        protocol_do_safety_door();
    }

    if (rtSleep) {
        protocol_do_sleep();
    }

    if (rtCycleStart) {
        protocol_do_cycle_start();
    }

    if (rtCycleStop) {
        protocol_do_cycle_stop();
    }

    if (rtButtonMacro0) {
        rtButtonMacro0 = false;
        protocol_do_macro(0);
    }
    if (rtButtonMacro1) {
        rtButtonMacro1 = false;
        protocol_do_macro(1);
    }
    if (rtButtonMacro2) {
        rtButtonMacro0 = false;
        protocol_do_macro(2);
    }
    if (rtButtonMacro3) {
        rtButtonMacro3 = false;
        protocol_do_macro(3);
    }

    protocol_execute_overrides();

#ifdef DEBUG_PROTOCOL
    if (sys_rt_exec_debug) {
        sys_rt_exec_debug = false;
        report_realtime_debug();
    }
#endif
    // Reload step segment buffer
    switch (sys.state) {
        case State::ConfigAlarm:
        case State::Alarm:
        case State::CheckMode:
        case State::Idle:
        case State::Sleep:
            break;
        case State::Cycle:
        case State::Hold:
        case State::SafetyDoor:
        case State::Homing:
        case State::Jog:
            st_prep_buffer();
            break;
    }
}

//
//void protocol_exec_rt_system() {
//    ExecAlarm alarm = sys_rt_exec_alarm;  // Temp variable to avoid calling volatile multiple times.
//    if (alarm != ExecAlarm::None) {       // Enter only if an alarm is pending
//        // System alarm. Everything has shutdown by something that has gone severely wrong. Report
//        // the source of the error to the user. If critical, Grbl disables by entering an infinite
//        // loop until system reset/abort.
//        sys.state = State::Alarm;  // Set system alarm state
//        report_alarm_message(alarm);
//        // Halt everything upon a critical event flag. Currently hard and soft limits flag this.
//        if ((alarm == ExecAlarm::HardLimit) || (alarm == ExecAlarm::SoftLimit)) {
//            report_feedback_message(Message::CriticalEvent);
//            rtReset = false;  // Disable any existing reset
//            do {
//                // Block everything, except reset and status reports, until user issues reset or power
//                // cycles. Hard limits typically occur while unattended or not paying attention. Gives
//                // the user and a GUI time to do what is needed before resetting, like killing the
//                // incoming stream. The same could be said about soft limits. While the position is not
//                // lost, continued streaming could cause a serious crash if by chance it gets executed.
//            } while (!sys_rt_exec_state.bit.reset);
//        }
//        sys_rt_exec_alarm = ExecAlarm::None;
//    }
//    ExecState rt_exec_state;
//    rt_exec_state.value = sys_rt_exec_state.value;  // Copy volatile sys_rt_exec_state.
//    if (rt_exec_state.value != 0 || cycle_stop) {   // Test if any bits are on
//        // Execute system abort.
//        if (rt_exec_state.bit.reset) {
//            sys.abort = true;  // Only place this is set true.
//            return;            // Nothing else to do but exit.
//        }
//        // Execute and serial print status
//        if (rt_exec_state.bit.statusReport) {
//            report_realtime_status(CLIENT_ALL);
//            sys_rt_exec_state.bit.statusReport = false;
//        }
//        // NOTE: Once hold is initiated, the system immediately enters a suspend state to block all
//        // main program processes until either reset or resumed. This ensures a hold completes safely.
//        if (rt_exec_state.bit.motionCancel || rt_exec_state.bit.feedHold || rt_exec_state.bit.safetyDoor || rt_exec_state.bit.sleep) {
//            // State check for allowable states for hold methods.
//            if (!(sys.state == State::Alarm || sys.state == State::CheckMode)) {
//                // If in CYCLE or JOG states, immediately initiate a motion HOLD.
//                if (sys.state == State::Cycle || sys.state == State::Jog) {
//                    if (!(sys.suspend.bit.motionCancel || sys.suspend.bit.jogCancel)) {  // Block, if already holding.
//                        st_update_plan_block_parameters();  // Notify stepper module to recompute for hold deceleration.
//                        sys.step_control             = {};
//                        sys.step_control.executeHold = true;  // Initiate suspend state with active flag.
//                        if (sys.state == State::Jog) {        // Jog cancelled upon any hold event, except for sleeping.
//                            if (!rt_exec_state.bit.sleep) {
//                                sys.suspend.bit.jogCancel = true;
//                            }
//                        }
//                    }
//                }
//                // If IDLE, Grbl is not in motion. Simply indicate suspend state and hold is complete.
//                if (sys.state == State::Idle) {
//                    sys.suspend.value            = 0;
//                    sys.suspend.bit.holdComplete = true;
//                }
//                // Execute and flag a motion cancel with deceleration and return to idle. Used primarily by probing cycle
//                // to halt and cancel the remainder of the motion.
//                if (rt_exec_state.bit.motionCancel) {
//                    // MOTION_CANCEL only occurs during a CYCLE, but a HOLD and SAFETY_DOOR may been initiated beforehand
//                    // to hold the CYCLE. Motion cancel is valid for a single planner block motion only, while jog cancel
//                    // will handle and clear multiple planner block motions.
//                    if (sys.state != State::Jog) {
//                        sys.suspend.bit.motionCancel = true;  // NOTE: State is State::Cycle.
//                    }
//                    sys_rt_exec_state.bit.motionCancel = false;
//                }
//                // Execute a feed hold with deceleration, if required. Then, suspend system.
//                if (rt_exec_state.bit.feedHold) {
//                    // Block SAFETY_DOOR, JOG, and SLEEP states from changing to HOLD state.
//                    if (!(sys.state == State::SafetyDoor || sys.state == State::Jog || sys.state == State::Sleep)) {
//                        sys.state = State::Hold;
//                    }
//                    sys_rt_exec_state.bit.feedHold = false;
//                }
//                // Execute a safety door stop with a feed hold and disable spindle/coolant.
//                // NOTE: Safety door differs from feed holds by stopping everything no matter state, disables powered
//                // devices (spindle/coolant), and blocks resuming until switch is re-engaged.
//                if (rt_exec_state.bit.safetyDoor) {
//                    report_feedback_message(Message::SafetyDoorAjar);
//                    // If jogging, block safety door methods until jog cancel is complete. Just flag that it happened.
//                    if (!(sys.suspend.bit.jogCancel)) {
//                        // Check if the safety re-opened during a restore parking motion only. Ignore if
//                        // already retracting, parked or in sleep state.
//                        if (sys.state == State::SafetyDoor) {
//                            if (sys.suspend.bit.initiateRestore) {  // Actively restoring
//                                // Set hold and reset appropriate control flags to restart parking sequence.
//                                if (sys.step_control.executeSysMotion) {
//                                    st_update_plan_block_parameters();  // Notify stepper module to recompute for hold deceleration.
//                                    sys.step_control                  = {};
//                                    sys.step_control.executeHold      = true;
//                                    sys.step_control.executeSysMotion = true;
//                                    sys.suspend.bit.holdComplete      = false;
//                                }  // else NO_MOTION is active.
//                                sys.suspend.bit.retractComplete = false;
//                                sys.suspend.bit.initiateRestore = false;
//                                sys.suspend.bit.restoreComplete = false;
//                                sys.suspend.bit.restartRetract  = true;
//                            }
//                        }
//                        if (sys.state != State::Sleep) {
//                            sys.state = State::SafetyDoor;
//                        }
//                        sys_rt_exec_state.bit.safetyDoor = false;
//                    }
//                    // NOTE: This flag doesn't change when the door closes, unlike sys.state. Ensures any parking motions
//                    // are executed if the door switch closes and the state returns to HOLD.
//                    sys.suspend.bit.safetyDoorAjar = true;
//                }
//            }
//            if (rt_exec_state.bit.sleep) {
//                if (sys.state == State::Alarm) {
//                    sys.suspend.bit.retractComplete = true;
//                    sys.suspend.bit.holdComplete    = true;
//                }
//                sys.state                   = State::Sleep;
//                sys_rt_exec_state.bit.sleep = false;
//            }
//        }
//        // Execute a cycle start by starting the stepper interrupt to begin executing the blocks in queue.
//        if (rt_exec_state.bit.cycleStart) {
//            // Block if called at same time as the hold commands: feed hold, motion cancel, and safety door.
//            // Ensures auto-cycle-start doesn't resume a hold without an explicit user-input.
//            if (!(rt_exec_state.bit.feedHold || rt_exec_state.bit.motionCancel || rt_exec_state.bit.safetyDoor)) {
//                // Resume door state when parking motion has retracted and door has been closed.
//                if (sys.state == State::SafetyDoor && !(sys.suspend.bit.safetyDoorAjar)) {
//                    if (sys.suspend.bit.restoreComplete) {
//                        sys.state = State::Idle;  // Set to IDLE to immediately resume the cycle.
//                    } else if (sys.suspend.bit.retractComplete) {
//                        // Flag to re-energize powered components and restore original position, if disabled by SAFETY_DOOR.
//                        // NOTE: For a safety door to resume, the switch must be closed, as indicated by HOLD state, and
//                        // the retraction execution is complete, which implies the initial feed hold is not active. To
//                        // restore normal operation, the restore procedures must be initiated by the following flag. Once,
//                        // they are complete, it will call CYCLE_START automatically to resume and exit the suspend.
//                        sys.suspend.bit.initiateRestore = true;
//                    }
//                }
//                // Cycle start only when IDLE or when a hold is complete and ready to resume.
//                if (sys.state == State::Idle || (sys.state == State::Hold && sys.suspend.bit.holdComplete)) {
//                    if (sys.state == State::Hold && sys.spindle_stop_ovr.value) {
//                        sys.spindle_stop_ovr.bit.restoreCycle = true;  // Set to restore in suspend routine and cycle start after.
//                    } else {
//                        // Start cycle only if queued motions exist in planner buffer and the motion is not canceled.
//                        sys.step_control = {};  // Restore step control to normal operation
//                        if (plan_get_current_block() && !sys.suspend.bit.motionCancel) {
//                            sys.suspend.value = 0;  // Break suspend state.
//                            sys.state         = State::Cycle;
//                            st_prep_buffer();  // Initialize step segment buffer before beginning cycle.
//                            st_wake_up();
//                        } else {                    // Otherwise, do nothing. Set and resume IDLE state.
//                            sys.suspend.value = 0;  // Break suspend state.
//                            sys.state         = State::Idle;
//                        }
//                    }
//                }
//            }
//            sys_rt_exec_state.bit.cycleStart = false;
//        }
//        if (cycle_stop) {
//            // Reinitializes the cycle plan and stepper system after a feed hold for a resume. Called by
//            // realtime command execution in the main program, ensuring that the planner re-plans safely.
//            // NOTE: Bresenham algorithm variables are still maintained through both the planner and stepper
//            // cycle reinitializations. The stepper path should continue exactly as if nothing has happened.
//            // NOTE: cycle_stop is set by the stepper subsystem when a cycle or feed hold completes.
//            if ((sys.state == State::Hold || sys.state == State::SafetyDoor || sys.state == State::Sleep) && !(sys.soft_limit) &&
//                !(sys.suspend.bit.jogCancel)) {
//                // Hold complete. Set to indicate ready to resume.  Remain in HOLD or DOOR states until user
//                // has issued a resume command or reset.
//                plan_cycle_reinitialize();
//                if (sys.step_control.executeHold) {
//                    sys.suspend.bit.holdComplete = true;
//                }
//                sys.step_control.executeHold      = false;
//                sys.step_control.executeSysMotion = false;
//            } else {
//                // Motion complete. Includes CYCLE/JOG/HOMING states and jog cancel/motion cancel/soft limit events.
//                // NOTE: Motion and jog cancel both immediately return to idle after the hold completes.
//                if (sys.suspend.bit.jogCancel) {  // For jog cancel, flush buffers and sync positions.
//                    sys.step_control = {};
//                    plan_reset();
//                    st_reset();
//                    gc_sync_position();
//                    plan_sync_position();
//                }
//                if (sys.suspend.bit.safetyDoorAjar) {  // Only occurs when safety door opens during jog.
//                    sys.suspend.bit.jogCancel    = false;
//                    sys.suspend.bit.holdComplete = true;
//                    sys.state                    = State::SafetyDoor;
//                } else {
//                    sys.suspend.value = 0;
//                    sys.state         = State::Idle;
//                }
//            }
//            cycle_stop = false;
//        }
//    }
//    // Execute overrides.
//    if ((sys_rt_f_override != sys.f_override) || (sys_rt_r_override != sys.r_override)) {
//        sys.f_override         = sys_rt_f_override;
//        sys.r_override         = sys_rt_r_override;
//        sys.report_ovr_counter = 0;  // Set to report change immediately
//        plan_update_velocity_profile_parameters();
//        plan_cycle_reinitialize();
//    }
//
//    // NOTE: Unlike motion overrides, spindle overrides do not require a planner reinitialization.
//    if (sys_rt_s_override != sys.spindle_speed_ovr) {
//        sys.step_control.updateSpindleSpeed = true;
//        sys.spindle_speed_ovr             = sys_rt_s_override;
//        sys.report_ovr_counter            = 0;  // Set to report change immediately
//        // If spindle is on, tell it the speed has been overridden
//        if (gc_state.modal.spindle != SpindleState::Disable) {
//            spindle->setState(gc_state.modal.spindle, gc_state.spindle_speed);
//        }
//    }
//
//    if (sys_rt_exec_accessory_override.bit.spindleOvrStop) {
//        sys_rt_exec_accessory_override.bit.spindleOvrStop = false;
//        // Spindle stop override allowed only while in HOLD state.
//        // NOTE: Report counters are set in spindle->spinDown()
//        if (sys.state == State::Hold) {
//            if (sys.spindle_stop_ovr.value == 0) {
//                sys.spindle_stop_ovr.bit.initiate = true;
//            } else if (sys.spindle_stop_ovr.bit.enabled) {
//                sys.spindle_stop_ovr.bit.restore = true;
//            }
//        }
//    }
//
//    // NOTE: Since coolant state always performs a planner sync whenever it changes, the current
//    // run state can be determined by checking the parser state.
//    if (sys_rt_exec_accessory_override.bit.coolantFloodOvrToggle) {
//        sys_rt_exec_accessory_override.bit.coolantFloodOvrToggle = false;
//        if (config->_coolant->hasFlood()) {
//            if (sys.state == State::Idle || sys.state == State::Cycle || sys.state == State::Hold) {
//                gc_state.modal.coolant.Flood = !gc_state.modal.coolant.Flood;
//                config->_coolant->set_state(gc_state.modal.coolant);  // Report counter set in coolant_set_state().
//            }
//        }
//    }
//    if (sys_rt_exec_accessory_override.bit.coolantMistOvrToggle) {
//        sys_rt_exec_accessory_override.bit.coolantMistOvrToggle = false;
//        if (config->_coolant->hasMist()) {
//            if (sys.state == State::Idle || sys.state == State::Cycle || sys.state == State::Hold) {
//                gc_state.modal.coolant.Mist = !gc_state.modal.coolant.Mist;
//                config->_coolant->set_state(gc_state.modal.coolant);  // Report counter set in coolant_set_state().
//            }
//        }
//    }
//
//#ifdef DEBUG_REPORT_REALTIME
//    if (sys_rt_exec_debug) {
//        report_realtime_debug();
//        sys_rt_exec_debug = false;
//    }
//#endif
//    // Reload step segment buffer
//    switch (sys.state) {
//        case State::Cycle:
//        case State::Hold:
//        case State::SafetyDoor:
//        case State::Homing:
//        case State::Sleep:
//        case State::Jog:
//            st_prep_buffer();
//            break;
//        default:
//            break;
//    }
//}

// Handles Grbl system suspend procedures, such as feed hold, safety door, and parking motion.
// The system will enter this loop, create local variables for suspend tasks, and return to
// whatever function that invoked the suspend, such that Grbl resumes normal operation.
// This function is written in a way to promote custom parking motions. Simply use this as a
// template
static void protocol_exec_rt_suspend() {
    // Declare and initialize parking local variables
    float             restore_target[MAX_N_AXIS];
    float             retract_waypoint = PARKING_PULLOUT_INCREMENT;
    plan_line_data_t  plan_data;
    plan_line_data_t* pl_data = &plan_data;
    memset(pl_data, 0, sizeof(plan_line_data_t));
    pl_data->motion                = {};
    pl_data->motion.systemMotion   = 1;
    pl_data->motion.noFeedOverride = 1;
    pl_data->line_number           = PARKING_MOTION_LINE_NUMBER;

    plan_block_t* block = plan_get_current_block();
    CoolantState  restore_coolant;
    SpindleState  restore_spindle;
    SpindleSpeed  restore_spindle_speed;
    if (block == NULL) {
        restore_coolant       = gc_state.modal.coolant;
        restore_spindle       = gc_state.modal.spindle;
        restore_spindle_speed = gc_state.spindle_speed;
    } else {
        restore_coolant       = block->coolant;
        restore_spindle       = block->spindle;
        restore_spindle_speed = block->spindle_speed;
    }
    if (config->_disableLaserDuringHold && config->_laserMode) {
        sys_rt_exec_accessory_override.bit.spindleOvrStop = true;
    }

    while (sys.suspend.value) {
        if (sys.abort) {
            return;
        }
        // if a jogCancel comes in and we have a jog "in-flight" (parsed and handed over to mc_line()),
        //  then we need to cancel it before it reaches the planner.  otherwise we may try to move way out of
        //  normal bounds, especially with senders that issue a series of jog commands before sending a cancel.
        if (sys.suspend.bit.jogCancel) {
            mc_cancel_jog();
        }
        // Block until initial hold is complete and the machine has stopped motion.
        if (sys.suspend.bit.holdComplete) {
            // Parking manager. Handles de/re-energizing, switch state checks, and parking motions for
            // the safety door and sleep states.
            if (sys.state == State::SafetyDoor || sys.state == State::Sleep) {
                // Handles retraction motions and de-energizing.
                float* parking_target = system_get_mpos();
                if (!sys.suspend.bit.retractComplete) {
                    // Ensure any prior spindle stop override is disabled at start of safety door routine.
                    sys.spindle_stop_ovr.value = 0;  // Disable override

                    // Get current position and store restore location and spindle retract waypoint.
                    if (!sys.suspend.bit.restartRetract) {
                        memcpy(restore_target, parking_target, sizeof(restore_target[0]) * config->_axes->_numberAxis);
                        retract_waypoint += restore_target[PARKING_AXIS];
                        retract_waypoint = MIN(retract_waypoint, PARKING_TARGET);
                    }
                    // Execute slow pull-out parking retract motion. Parking requires homing enabled, the
                    // current location not exceeding the parking target location, and laser mode disabled.
                    // NOTE: State will remain DOOR, until the de-energizing and retract is complete.
                    if (can_park() && parking_target[PARKING_AXIS] < PARKING_TARGET) {
                        // Retract spindle by pullout distance. Ensure retraction motion moves away from
                        // the workpiece and waypoint motion doesn't exceed the parking target location.
                        if (parking_target[PARKING_AXIS] < retract_waypoint) {
                            parking_target[PARKING_AXIS] = retract_waypoint;
                            pl_data->feed_rate           = PARKING_PULLOUT_RATE;
                            pl_data->coolant             = restore_coolant;
                            pl_data->spindle             = restore_spindle;
                            pl_data->spindle_speed       = restore_spindle_speed;
                            mc_parking_motion(parking_target, pl_data);
                        }
                        // NOTE: Clear accessory state after retract and after an aborted restore motion.
                        pl_data->spindle               = SpindleState::Disable;
                        pl_data->coolant               = {};
                        pl_data->motion                = {};
                        pl_data->motion.systemMotion   = 1;
                        pl_data->motion.noFeedOverride = 1;
                        pl_data->spindle_speed         = 0.0;
                        spindle->spinDown();
                        config->_coolant->set_state(pl_data->coolant);
                        // Execute fast parking retract motion to parking target location.
                        if (parking_target[PARKING_AXIS] < PARKING_TARGET) {
                            parking_target[PARKING_AXIS] = PARKING_TARGET;
                            pl_data->feed_rate           = PARKING_RATE;
                            mc_parking_motion(parking_target, pl_data);
                        }
                    } else {
                        // Parking motion not possible. Just disable the spindle and coolant.
                        // NOTE: Laser mode does not start a parking motion to ensure the laser stops immediately.
                        spindle->spinDown();
                        config->_coolant->off();
                    }

                    sys.suspend.bit.restartRetract  = false;
                    sys.suspend.bit.retractComplete = true;
                } else {
                    if (sys.state == State::Sleep) {
                        report_feedback_message(Message::SleepMode);
                        // Spindle and coolant should already be stopped, but do it again just to be sure.
                        spindle->spinDown();
                        config->_coolant->off();
                        st_go_idle();  // Disable steppers
                        while (!(sys.abort)) {
                            protocol_exec_rt_system();  // Do nothing until reset.
                        }
                        return;  // Abort received. Return to re-initialize.
                    }
                    // Allows resuming from parking/safety door. Actively checks if safety door is closed and ready to resume.
                    if (sys.state == State::SafetyDoor) {
                        if (!config->_control->system_check_safety_door_ajar()) {
                            sys.suspend.bit.safetyDoorAjar = false;  // Reset door ajar flag to denote ready to resume.
                        }
                    }
                    // Handles parking restore and safety door resume.
                    if (sys.suspend.bit.initiateRestore) {
                        // Execute fast restore motion to the pull-out position. Parking requires homing enabled.
                        // NOTE: State is will remain DOOR, until the de-energizing and retract is complete.
                        if (can_park()) {
                            // Check to ensure the motion doesn't move below pull-out position.
                            if (parking_target[PARKING_AXIS] <= PARKING_TARGET) {
                                parking_target[PARKING_AXIS] = retract_waypoint;
                                pl_data->feed_rate           = PARKING_RATE;
                                mc_parking_motion(parking_target, pl_data);
                            }
                        }
                        // Delayed Tasks: Restart spindle and coolant, delay to power-up, then resume cycle.
                        if (gc_state.modal.spindle != SpindleState::Disable) {
                            // Block if safety door re-opened during prior restore actions.
                            if (!sys.suspend.bit.restartRetract) {
                                if (config->_laserMode) {
                                    // When in laser mode, defer turn on until cycle starts
                                    sys.step_control.updateSpindleSpeed = true;
                                } else {
                                    spindle->setState(restore_spindle, restore_spindle_speed);
                                }
                            }
                        }
                        if (gc_state.modal.coolant.Flood || gc_state.modal.coolant.Mist) {
                            // Block if safety door re-opened during prior restore actions.
                            if (!sys.suspend.bit.restartRetract) {
                                config->_coolant->set_state(restore_coolant);
                            }
                        }

                        // Execute slow plunge motion from pull-out position to resume position.
                        if (can_park()) {
                            // Block if safety door re-opened during prior restore actions.
                            if (!sys.suspend.bit.restartRetract) {
                                // Regardless if the retract parking motion was a valid/safe motion or not, the
                                // restore parking motion should logically be valid, either by returning to the
                                // original position through valid machine space or by not moving at all.
                                pl_data->feed_rate     = PARKING_PULLOUT_RATE;
                                pl_data->spindle       = restore_spindle;
                                pl_data->coolant       = restore_coolant;
                                pl_data->spindle_speed = restore_spindle_speed;
                                mc_parking_motion(restore_target, pl_data);
                            }
                        }
                        if (!sys.suspend.bit.restartRetract) {
                            sys.suspend.bit.restoreComplete = true;
                            rtCycleStart                    = true;  // Set to resume program.
                        }
                    }
                }
            } else {
                // Feed hold manager. Controls spindle stop override states.
                // NOTE: Hold ensured as completed by condition check at the beginning of suspend routine.
                if (sys.spindle_stop_ovr.value) {
                    // Handles beginning of spindle stop
                    if (sys.spindle_stop_ovr.bit.initiate) {
                        if (gc_state.modal.spindle != SpindleState::Disable) {
                            spindle->spinDown();
                            sys.spindle_stop_ovr.value       = 0;
                            sys.spindle_stop_ovr.bit.enabled = true;  // Set stop override state to enabled, if de-energized.
                        } else {
                            sys.spindle_stop_ovr.value = 0;  // Clear stop override state
                        }
                        // Handles restoring of spindle state
                    } else if (sys.spindle_stop_ovr.bit.restore || sys.spindle_stop_ovr.bit.restoreCycle) {
                        if (gc_state.modal.spindle != SpindleState::Disable) {
                            report_feedback_message(Message::SpindleRestore);
                            if (config->_laserMode) {
                                // When in laser mode, defer turn on until cycle starts
                                sys.step_control.updateSpindleSpeed = true;
                            } else {
                                spindle->setState(restore_spindle, restore_spindle_speed);
                            }
                        }
                        if (sys.spindle_stop_ovr.bit.restoreCycle) {
                            rtCycleStart = true;  // Set to resume program.
                        }
                        sys.spindle_stop_ovr.value = 0;  // Clear stop override state
                    }
                } else {
                    // Handles spindle state during hold. NOTE: Spindle speed overrides may be altered during hold state.
                    // NOTE: sys.step_control.updateSpindleSpeed is automatically reset upon resume in step generator.
                    if (sys.step_control.updateSpindleSpeed) {
                        spindle->setState(restore_spindle, restore_spindle_speed);
                        sys.step_control.updateSpindleSpeed = false;
                    }
                }
            }
        }
        protocol_exec_rt_system();
    }
}

/*
  Limits.cpp - code pertaining to limit-switches and performing the homing cycle
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P
  2018-12-29 - Wolfgang Lienbacher renamed file from limits.h to grbl_limits.h
          fixing ambiguation issues with limit.h in the esp32 Arduino Framework
          when compiling with VS-Code/PlatformIO.

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

#include "Grbl.h"
#include "MachineConfig.h"

#include <atomic>

xQueueHandle limit_sw_queue;  // used by limit switch debouncing

// Homing axis search distance multiplier. Computed by this value times the cycle travel.
#ifndef HOMING_AXIS_SEARCH_SCALAR
#    define HOMING_AXIS_SEARCH_SCALAR 1.1  // Must be > 1 to ensure limit switch will be engaged.
#endif
#ifndef HOMING_AXIS_LOCATE_SCALAR
#    define HOMING_AXIS_LOCATE_SCALAR 5.0  // Must be > 1 to ensure limit switch is cleared.
#endif

void IRAM_ATTR isr_limit_switches(void* /*unused */) {
    // Ignore limit switches if already in an alarm state or in-process of executing an alarm.
    // When in the alarm state, Grbl should have been reset or will force a reset, so any pending
    // moves in the planner and serial buffers are all cleared and newly sent blocks will be
    // locked out until a homing cycle or a kill lock command. Allows the user to disable the hard
    // limit setting if their limits are constantly triggering after a reset and move their axes.
    if (sys.state != State::Alarm && sys.state != State::Homing) {
        if (sys_rt_exec_alarm == ExecAlarm::None) {
#ifdef ENABLE_SOFTWARE_DEBOUNCE
            // we will start a task that will recheck the switches after a small delay
            int evt;
            xQueueSendFromISR(limit_sw_queue, &evt, NULL);
#else
#    ifdef HARD_LIMIT_FORCE_STATE_CHECK
            // Check limit pin state.
            if (limits_get_state()) {
                grbl_msg_sendf(CLIENT_ALL, MsgLevel::Debug, "Hard limits");
                mc_reset();                                // Initiate system kill.
                sys_rt_exec_alarm = ExecAlarm::HardLimit;  // Indicate hard limit critical event
            }
#    else
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Debug, "Hard limits");
            mc_reset();                                // Initiate system kill.
            sys_rt_exec_alarm = ExecAlarm::HardLimit;  // Indicate hard limit critical event
#    endif
#endif
        }
    }
}

AxisMask homingAxes() {
    AxisMask mask   = 0;
    auto     axes   = config->_axes;
    auto     n_axis = axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        auto homing = axes->_axis[axis]->_homing;
        if (homing) {
            bitnum_true(mask, axis);
        }
    }
    return mask;
}

// Homes the specified cycle axes, sets the machine position, and performs a pull-off motion after
// completing. Homing is a special motion case, which involves rapid uncontrolled stops to locate
// the trigger point of the limit switches. The rapid stops are handled by a system level axis lock
// mask, which prevents the stepper algorithm from executing step pulses. Homing motions typically
// circumvent the processes for executing motions in normal operation.
// NOTE: Only the abort realtime command can interrupt this process.
// TODO: Move limit pin-specific calls to a general function for portability.
void limits_go_home(uint8_t cycle_mask, uint n_locate_cycles) {
    if ((cycle_mask & homingAxes()) != cycle_mask) {
        grbl_msg_sendf(CLIENT_ALL, MsgLevel::Debug, "Homing is not configured for some requested axes");
    }
    if (sys.abort) {
        return;  // Block if system reset has been issued.
    }
    // Initialize plan data struct for homing motion. Spindle and coolant are disabled.

    // Put motors on axes listed in cycle_mask in homing mode and
    // replace cycle_mask with the list of motors that are ready for homing.
    // Motors with non standard homing can home during motors_set_homing_mode(...)
    cycle_mask = config->_axes->set_homing_mode(cycle_mask, true);  // tell motors homing is about to start

    // See if any motors are left.  This could be 0 if none of the motors specified
    // by the original value of cycle_mask is capable of standard homing.
    if (cycle_mask == 0) {
        return;
    }

    plan_line_data_t  plan_data;
    plan_line_data_t* pl_data = &plan_data;
    memset(pl_data, 0, sizeof(plan_line_data_t));
    pl_data->motion                = {};
    pl_data->motion.systemMotion   = 1;
    pl_data->motion.noFeedOverride = 1;
#ifdef USE_LINE_NUMBERS
    pl_data->line_number = HOMING_CYCLE_LINE_NUMBER;
#endif
    // Initialize variables used for homing computations.
    uint8_t n_cycle = (2 * n_locate_cycles + 1);

    auto n_axis = config->_axes->_numberAxis;

    // Set search mode with approach at seek rate to quickly engage the specified cycle_mask limit switches.
    bool     approach = true;
    uint8_t  n_active_axis;
    AxisMask limit_state, axislock;
    float    homing_rate = 0.0;
    do {
        float* target = system_get_mpos();

        // For multi-axis homing, we use the per-axis rates and travel limits to compute
        // a target vector and a feedrate as follows:
        // The goal is for each axis to travel at its specified rate, and for the
        // maximum travel to be enough for each participating axis to reach its limit.
        // For the rate goal, the axis components of the target vector must be proportional
        // to the per-axis rates, and the overall feed rate must be the magnitude of the
        // vector of per-axis rates.
        // For the travel goal, the axis components of the target vector must be scaled
        // according to the one that would take the longest.
        // The time to complete a maxTravel move for a given feedRate is maxTravel/feedRate.
        // We compute that time for all axes in the homing cycle, then find the longest one.
        // Then we scale the travel distances for the other axes so they would complete
        // at the same time.
        axislock           = 0;
        float maxSeekTime  = 0.0;
        float limitingRate = 0.0;
        int   debounce     = 0;

        for (int axis = 0; axis < n_axis; axis++) {
            if (bitnum_isfalse(cycle_mask, axis)) {
                continue;
            }
            auto axisConfig = config->_axes->_axis[axis];
            auto homing     = axisConfig->_homing;
            if (homing != nullptr) {
                auto axis_debounce = homing->_debounce;
                if (axis_debounce > debounce) {
                    debounce = axis_debounce;
                }
                auto axis_homing_rate = approach ? homing->_seekRate : homing->_feedRate;

                // Accumulate the squares of the homing rates for later use
                // in computing the aggregate feed rate.
                homing_rate += (axis_homing_rate * axis_homing_rate);

                // Set target location for active axes and setup computation for homing rate.
                if (bitnum_istrue(cycle_mask, axis)) {
                    sys_position[axis] = 0;

                    auto travel = approach ? axisConfig->_maxTravel : homing->_pulloff;

                    // First we compute the maximum time to completion vector; later we will
                    // convert it back to positions after we determine the limiting axis.
                    // Set target direction based on cycle mask and homing cycle approach state.
                    auto seekTime = travel / axis_homing_rate;
                    target[axis]  = (homing->_positiveDirection ^ approach) ? -seekTime : seekTime;
                    if (seekTime > maxSeekTime) {
                        maxSeekTime  = seekTime;
                        limitingRate = axis_homing_rate;
                    }
                }

                // Record the axis as active
                bitnum_true(axislock, axis);
            }
        }
        // Scale the target array, currently in units of time, back to positions
        // The SCALAR adds a small fudge factor to ensure that the limit is reached
        limitingRate *= approach ? HOMING_AXIS_SEARCH_SCALAR : HOMING_AXIS_LOCATE_SCALAR;
        for (int axis = 0; axis < n_axis; axis++) {
            if (bitnum_istrue(axislock, axis)) {
                target[axis] *= limitingRate;
            }
        }
        homing_rate = sqrt(homing_rate);  // Magnitude of homing rate vector

        sys.homing_axis_lock = axislock;
        // Perform homing cycle. Planner buffer should be empty, as required to initiate the homing cycle.
        pl_data->feed_rate =                    // Set current homing rate.
            plan_buffer_line(target, pl_data);  // Bypass mc_line(). Directly plan homing motion.
        sys.step_control                  = {};
        sys.step_control.executeSysMotion = true;  // Set to execute homing motion and clear existing flags.
        st_prep_buffer();                          // Prep and fill segment buffer from newly planned block.
        st_wake_up();                              // Initiate motion
        do {
            if (approach) {
                // Check limit state. Lock out cycle axes when they change.
                limit_state = limits_get_state();
                for (int axis = 0; axis < n_axis; axis++) {
                    if (bitnum_istrue(axislock, axis)) {
                        if (bitnum_istrue(limit_state, axis)) {
                            bitnum_false(axislock, axis);
                        }
                    }
                }
                sys.homing_axis_lock = axislock;
            }
            st_prep_buffer();  // Check and prep segment buffer. NOTE: Should take no longer than 200us.
            // Exit routines: No time to run protocol_execute_realtime() in this loop.
            if (rtSafetyDoor || rtReset || rtCycleStop) {
                // Homing failure condition: Reset issued during cycle.
                if (rtReset) {
                    sys_rt_exec_alarm = ExecAlarm::HomingFailReset;
                }
                // Homing failure condition: Safety door was opened.
                if (rtSafetyDoor) {
                    sys_rt_exec_alarm = ExecAlarm::HomingFailDoor;
                }
                // Homing failure condition: Limit switch still engaged after pull-off motion
                if (!approach && (limits_get_state() & cycle_mask)) {
                    sys_rt_exec_alarm = ExecAlarm::HomingFailPulloff;
                }
                // Homing failure condition: Limit switch not found during approach.
                if (approach && rtCycleStop) {
                    sys_rt_exec_alarm = ExecAlarm::HomingFailApproach;
                }

                if (sys_rt_exec_alarm != ExecAlarm::None) {
                    config->_axes->set_homing_mode(cycle_mask, false);  // tell motors homing is done...failed
                    grbl_msg_sendf(CLIENT_ALL, MsgLevel::Debug, "Homing fail");
                    mc_reset();  // Stop motors, if they are running.
                    protocol_execute_realtime();
                    return;
                } else {
                    // Pull-off motion complete. Disable CYCLE_STOP from executing.
                    rtCycleStop = false;
                    break;
                }
            }
        } while (STEP_MASK & axislock);
#ifdef USE_I2S_STEPS
        if (current_stepper == ST_I2S_STREAM) {
            if (!approach) {
                delay_ms(I2S_OUT_DELAY_MS);
            }
        }
#endif
        st_reset();          // Immediately force kill steppers and reset step segment buffer.
        delay_ms(debounce);  // Delay to allow transient dynamics to dissipate.
        // Reverse direction and reset homing rate for locate cycle(s).
        approach = !approach;
    } while (n_cycle-- > 0);

    // The active cycle axes should now be homed and machine limits have been located. By
    // default, Grbl defines machine space as all negative, as do most CNCs. Since limit switches
    // can be on either side of an axes, check and set axes machine zero appropriately. Also,
    // set up pull-off maneuver from axes limit switches that have been homed. This provides
    // some initial clearance off the switches and should also help prevent them from falsely
    // triggering when hard limits are enabled or when more than one axes shares a limit pin.
    int32_t set_axis_position;
    // Set machine positions for homed limit switches. Don't update non-homed axes.
    for (int axis = 0; axis < n_axis; axis++) {
        Axis* axisConf = config->_axes->_axis[axis];
        auto  homing   = axisConf->_homing;
        if (homing != nullptr && bitnum_istrue(cycle_mask, axis)) {
            auto mpos    = homing->_mpos;
            auto pulloff = homing->_pulloff;
            auto steps   = axisConf->_stepsPerMm;
            if (homing->_positiveDirection) {
                sys_position[axis] = (mpos + pulloff) * steps;
            } else {
                sys_position[axis] = (mpos - pulloff) * steps;
            }
        }
    }
    sys.step_control = {};                              // Return step control to normal operation.
    config->_axes->set_homing_mode(cycle_mask, false);  // tell motors homing is done
}

uint8_t limit_mask = 0;

void limits_init() {
    limit_mask     = 0;
    Pin::Attr mode = Pin::Attr::Input | Pin::Attr::ISR;

    auto n_axis = config->_axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        for (int gang_index = 0; gang_index < 2; gang_index++) {
            auto gangConfig = config->_axes->_axis[axis]->_gangs[gang_index];
            if (gangConfig->_endstops != nullptr && gangConfig->_endstops->_dual.defined()) {
                Pin& pin = gangConfig->_endstops->_dual;

#ifndef DISABLE_LIMIT_PIN_PULL_UP
                if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
                    mode = mode | Pin::Attr::PullUp;
                }
#endif

                pin.setAttr(mode);
                bitnum_true(limit_mask, axis);
                if (gangConfig->_endstops->_hardLimits) {
                    pin.attachInterrupt(isr_limit_switches, CHANGE, nullptr);
                } else {
                    pin.detachInterrupt();
                }

                if (limit_sw_queue == NULL) {
                    grbl_msg_sendf(
                        CLIENT_SERIAL, MsgLevel::Info, "%s limit switch on pin %s", reportAxisNameMsg(axis, gang_index), pin.name().c_str());
                }
            }
        }
    }

    // setup task used for debouncing
    if (limit_sw_queue == NULL) {
        limit_sw_queue = xQueueCreate(10, sizeof(int));
        xTaskCreate(limitCheckTask,
                    "limitCheckTask",
                    2048,
                    NULL,
                    5,  // priority
                    NULL);
    }
}

// Disables hard limits.
void limits_disable() {
    auto n_axis = config->_axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        for (int gang_index = 0; gang_index < 2; gang_index++) {
            auto gangConfig = config->_axes->_axis[axis]->_gangs[gang_index];
            if (gangConfig->_endstops != nullptr && gangConfig->_endstops->_dual.defined()) {
                Pin& pin = gangConfig->_endstops->_dual;
                pin.detachInterrupt();
            }
        }
    }
}

// Returns limit state as a bit-wise uint8 variable. Each bit indicates an axis limit, where
// triggered is 1 and not triggered is 0. Invert mask is applied. Axes are defined by their
// number in bit position, i.e. Z_AXIS is bit(2), and Y_AXIS is bit(1).
AxisMask limits_get_state() {
    AxisMask pinMask = 0;
    auto     n_axis  = config->_axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        for (int gang_index = 0; gang_index < 2; gang_index++) {
            auto gangConfig = config->_axes->_axis[axis]->_gangs[gang_index];
            if (gangConfig->_endstops != nullptr && gangConfig->_endstops->_dual.defined()) {
                Pin& pin = gangConfig->_endstops->_dual;
                bitnum_true(pinMask, (pin.read() << axis));
            }
        }
    }

#ifdef INVERT_LIMIT_PIN_MASK  // not normally used..unless you have both normal and inverted switches
    pinMask ^= INVERT_LIMIT_PIN_MASK;
#endif
    return pinMask;
}

// Performs a soft limit check. Called from mcline() only. Assumes the machine has been homed,
// the workspace volume is in all negative space, and the system is in normal operation.
// NOTE: Used by jogging to limit travel within soft-limit volume.
void limits_soft_check(float* target) {
    if (limitsCheckTravel(target)) {
        sys.soft_limit = true;
        // Force feed hold if cycle is active. All buffered blocks are guaranteed to be within
        // workspace volume so just come to a controlled stop so position is not lost. When complete
        // enter alarm mode.
        if (sys.state == State::Cycle) {
            rtFeedHold = true;
            do {
                protocol_execute_realtime();
                if (sys.abort) {
                    return;
                }
            } while (sys.state != State::Idle);
        }
        grbl_msg_sendf(CLIENT_ALL, MsgLevel::Debug, "Soft limits");
        mc_reset();                                // Issue system reset and ensure spindle and coolant are shutdown.
        sys_rt_exec_alarm = ExecAlarm::SoftLimit;  // Indicate soft limit critical event
        protocol_execute_realtime();               // Execute to enter critical event loop and system abort
        return;
    }
}

// this is the task
void limitCheckTask(void* pvParameters) {
    while (true) {
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);  // read fence for settings

        int evt;
        xQueueReceive(limit_sw_queue, &evt, portMAX_DELAY);  // block until receive queue
        vTaskDelay(DEBOUNCE_PERIOD / portTICK_PERIOD_MS);    // delay a while
        AxisMask switch_state;
        switch_state = limits_get_state();
        if (switch_state) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, "Limit Switch State %08d", switch_state);
            mc_reset();                                // Initiate system kill.
            sys_rt_exec_alarm = ExecAlarm::HardLimit;  // Indicate hard limit critical event
        }
        static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#endif
    }
}

float limitsMaxPosition(uint8_t axis) {
    auto  axisConfig = config->_axes->_axis[axis];
    auto  homing     = axisConfig->_homing;
    float mpos       = (homing != nullptr) ? homing->_mpos : 0;
    auto  maxtravel  = axisConfig->_maxTravel;

    return (homing == nullptr || homing->_positiveDirection) ? mpos + maxtravel : mpos;
}

float limitsMinPosition(uint8_t axis) {
    auto  axisConfig = config->_axes->_axis[axis];
    auto  homing     = axisConfig->_homing;
    float mpos       = (homing != nullptr) ? homing->_mpos : 0;
    auto  maxtravel  = axisConfig->_maxTravel;

    return (homing == nullptr || homing->_positiveDirection) ? mpos : mpos - maxtravel;
}

// Checks and reports if target array exceeds machine travel limits.
// Return true if exceeding limits
// Set $<axis>/MaxTravel=0 to selectively remove an axis from soft limit checks
bool __attribute__((weak)) limitsCheckTravel(float* target) {
    auto axes   = config->_axes;
    auto n_axis = axes->_numberAxis;
    for (int axis = 0; axis < n_axis; axis++) {
        float max_mpos, min_mpos;
        auto  axisSetting = axes->_axis[axis];
        if ((target[axis] < limitsMinPosition(axis) || target[axis] > limitsMaxPosition(axis)) && axisSetting->_maxTravel > 0) {
            return true;
        }
    }
    return false;
}

bool limitsSwitchDefined(uint8_t axis, uint8_t gang_index) {
    auto gangConfig = config->_axes->_axis[axis]->_gangs[gang_index];

    if (gangConfig->_endstops != nullptr) {
        return gangConfig->_endstops->_dual.defined();
    } else {
        return false;
    }
}

bool __attribute__((weak)) user_defined_homing(AxisMask cycle_mask) {
    return false;
}

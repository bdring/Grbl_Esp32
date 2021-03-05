/*
  Report.cpp - reporting and messaging methods
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modified for use on the ESP32
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

/*
  This file functions as the primary feedback interface for Grbl. Any outgoing data, such
  as the protocol status messages, feedback messages, and status reports, are stored here.
  For the most part, these functions primarily are called from Protocol.cpp methods. If a
  different style feedback is desired (i.e. JSON), then a user can change these following
  methods to accommodate their needs.


	ESP32 Notes:

	Major rewrite to fix issues with BlueTooth. As described here there is a
	when you try to send data a single byte at a time using SerialBT.write(...).
	https://github.com/espressif/arduino-esp32/issues/1537

	A solution is to send messages as a string using SerialBT.print(...). Use
	a short delay after each send. Therefore this file needed to be rewritten
	to work that way. AVR Grbl was written to be super efficient to give it
	good performance. This is far less efficient, but the ESP32 can handle it.
	Do not use this version of the file with AVR Grbl.

	ESP32 discussion here ...  https://github.com/bdring/Grbl_Esp32/issues/3


*/

#include "Grbl.h"
#include <map>

#ifdef REPORT_HEAP
EspClass esp;
#endif
const int DEFAULTBUFFERSIZE = 64;

// this is a generic send function that everything should use, so interfaces could be added (Bluetooth, etc)
void grbl_send(uint8_t client, const char* text) {
    if (client == CLIENT_INPUT) {
        return;
    }
#ifdef ENABLE_BLUETOOTH
    if (WebUI::SerialBT.hasClient() && (client == CLIENT_BT || client == CLIENT_ALL)) {
        WebUI::SerialBT.print(text);
        //delay(10); // possible fix for dropped characters
    }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_OUT)
    if (client == CLIENT_WEBUI || client == CLIENT_ALL) {
        WebUI::Serial2Socket.write((const uint8_t*)text, strlen(text));
    }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)
    if (client == CLIENT_TELNET || client == CLIENT_ALL) {
        WebUI::telnet_server.write((const uint8_t*)text, strlen(text));
    }
#endif
    if (client == CLIENT_SERIAL || client == CLIENT_ALL) {
        Serial.print(text);
    }
}

// This is a formating version of the grbl_send(CLIENT_ALL,...) function that work like printf
void grbl_sendf(uint8_t client, const char* format, ...) {
    if (client == CLIENT_INPUT) {
        return;
    }
    char    loc_buf[64];
    char*   temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf)) {
        temp = new char[len + 1];
        if (temp == NULL) {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    grbl_send(client, temp);
    va_end(arg);
    if (temp != loc_buf) {
        delete[] temp;
    }
}
// Use to send [MSG:xxxx] Type messages. The level allows messages to be easily suppressed
void grbl_msg_sendf(uint8_t client, MsgLevel level, const char* format, ...) {
    if (client == CLIENT_INPUT) {
        return;
    }

    if (message_level != NULL) {  // might be null before messages are setup
        if (level > static_cast<MsgLevel>(message_level->get())) {
            return;
        }
    }

    char    loc_buf[100];
    char*   temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf)) {
        temp = new char[len + 1];
        if (temp == NULL) {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    grbl_sendf(client, "[MSG:%s]\r\n", temp);
    va_end(arg);
    if (temp != loc_buf) {
        delete[] temp;
    }
}

//function to notify
void grbl_notify(const char* title, const char* msg) {
#ifdef ENABLE_NOTIFICATIONS
    WebUI::notificationsservice.sendMSG(title, msg);
#endif
}

void grbl_notifyf(const char* title, const char* format, ...) {
    char    loc_buf[64];
    char*   temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf)) {
        temp = new char[len + 1];
        if (temp == NULL) {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);
    grbl_notify(title, temp);
    va_end(arg);
    if (temp != loc_buf) {
        delete[] temp;
    }
}

static const int coordStringLen = 20;
static const int axesStringLen  = coordStringLen * MAX_N_AXIS;

// formats axis values into a string and returns that string in rpt
// NOTE: rpt should have at least size: axesStringLen
static void report_util_axis_values(float* axis_value, char* rpt) {
    uint8_t     idx;
    char        axisVal[coordStringLen];
    float       unit_conv = 1.0;      // unit conversion multiplier..default is mm
    const char* format    = "%4.3f";  // Default - report mm to 3 decimal places
    rpt[0]                = '\0';
    if (report_inches->get()) {
        unit_conv = 1.0 / MM_PER_INCH;
        format    = "%4.4f";  // Report inches to 4 decimal places
    }
    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++) {
        snprintf(axisVal, coordStringLen - 1, format, axis_value[idx] * unit_conv);
        strcat(rpt, axisVal);
        if (idx < (number_axis->get() - 1)) {
            strcat(rpt, ",");
        }
    }
}

// This version returns the axis values as a String
static String report_util_axis_values(const float* axis_value) {
    String  rpt = "";
    uint8_t idx;
    char    axisVal[coordStringLen];
    float   unit_conv = 1.0;  // unit conversion multiplier..default is mm
    int     decimals  = 3;    // Default - report mm to 3 decimal places
    if (report_inches->get()) {
        unit_conv = 1.0 / MM_PER_INCH;
        decimals  = 4;  // Report inches to 4 decimal places
    }
    auto n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++) {
        rpt += String(axis_value[idx] * unit_conv, decimals);
        if (idx < (number_axis->get() - 1)) {
            rpt += ",";
        }
    }
    return rpt;
}

// Handles the primary confirmation protocol response for streaming interfaces and human-feedback.
// For every incoming line, this method responds with an 'ok' for a successful command or an
// 'error:'  to indicate some error event with the line or some critical system error during
// operation. Errors events can originate from the g-code parser, settings module, or asynchronously
// from a critical error, such as a triggered hard limit. Interface should always monitor for these
// responses.
void report_status_message(Error status_code, uint8_t client) {
    switch (status_code) {
        case Error::Ok:  // Error::Ok
#ifdef ENABLE_SD_CARD
            if (get_sd_state(false) == SDState::BusyPrinting) {
                SD_ready_next = true;  // flag so system_execute_line() will send the next line
            } else {
                grbl_send(client, "ok\r\n");
            }
#else
            grbl_send(client, "ok\r\n");
#endif
            break;
        default:
#ifdef ENABLE_SD_CARD
            // do we need to stop a running SD job?
            if (get_sd_state(false) == SDState::BusyPrinting) {
                if (status_code == Error::GcodeUnsupportedCommand) {
                    grbl_sendf(client, "error:%d\r\n", status_code);  // most senders seem to tolerate this error and keep on going
                    grbl_sendf(CLIENT_ALL, "error:%d in SD file at line %d\r\n", status_code, sd_get_current_line_number());
                    // don't close file
                    SD_ready_next = true;  // flag so system_execute_line() will send the next line
                } else {
                    grbl_notifyf("SD print error", "Error:%d during SD file at line: %d", status_code, sd_get_current_line_number());
                    grbl_sendf(CLIENT_ALL, "error:%d in SD file at line %d\r\n", status_code, sd_get_current_line_number());
                    closeFile();
                }
                return;
            }
#endif
            // With verbose errors, the message text is displayed instead of the number.
            // Grbl 0.9 used to display the text, while Grbl 1.1 switched to the number.
            // Many senders support both formats.
            if (verbose_errors->get()) {
                grbl_sendf(client, "error: %s\r\n", errorString(status_code));
            } else {
                grbl_sendf(client, "error:%d\r\n", static_cast<int>(status_code));
            }
    }
}

// Prints alarm messages.
void report_alarm_message(ExecAlarm alarm_code) {
    grbl_sendf(CLIENT_ALL, "ALARM:%d\r\n", static_cast<int>(alarm_code));  // OK to send to all clients
    delay_ms(500);                                                         // Force delay to ensure message clears serial write buffer.
}

std::map<Message, const char*> MessageText = {
    { Message::CriticalEvent, "Reset to continue" },
    { Message::AlarmLock, "'$H'|'$X' to unlock" },
    { Message::AlarmUnlock, "Caution: Unlocked" },
    { Message::Enabled, "Enabled" },
    { Message::Disabled, "Disabled" },
    { Message::SafetyDoorAjar, "Check door" },
    { Message::CheckLimits, "Check limits" },
    { Message::ProgramEnd, "Program End" },
    { Message::RestoreDefaults, "Restoring defaults" },
    { Message::SpindleRestore, "Restoring spindle" },
    { Message::SleepMode, "Sleeping" },
    // Handled separately due to numeric argument
    // { Message::SdFileQuit, "Reset during SD file at line: %d" },
};

// Prints feedback messages. This serves as a centralized method to provide additional
// user feedback for things that are not of the status/alarm message protocol. These are
// messages such as setup warnings, switch toggling, and how to exit alarms.
// NOTE: For interfaces, messages are always placed within brackets. And if silent mode
// is installed, the message number codes are less than zero.
void report_feedback_message(Message message) {  // ok to send to all clients
#if defined (ENABLE_SD_CARD) 
    if (message == Message::SdFileQuit) {
        grbl_notifyf("SD print canceled", "Reset during SD file at line: %d", sd_get_current_line_number());
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Reset during SD file at line: %d", sd_get_current_line_number());

    } else 
#endif //ENABLE_SD_CARD  
    {
        auto it = MessageText.find(message);
        if (it != MessageText.end()) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, it->second);
        }
    }
}

// Welcome message
void report_init_message(uint8_t client) {
    grbl_sendf(client, "\r\nGrbl %s ['$' for help]\r\n", GRBL_VERSION);
}

// Grbl help message
void report_grbl_help(uint8_t client) {
    grbl_send(client, "[HLP:$$ $+ $# $S $L $G $I $N $x=val $Nx=line $J=line $SLP $C $X $H $F $E=err ~ ! ? ctrl-x]\r\n");
}

// Prints current probe parameters. Upon a probe command, these parameters are updated upon a
// successful probe or upon a failed probe with the G38.3 without errors command (if supported).
// These values are retained until Grbl is power-cycled, whereby they will be re-zeroed.
void report_probe_parameters(uint8_t client) {
    // Report in terms of machine position.
    float print_position[MAX_N_AXIS];
    char  probe_rpt[(axesStringLen + 13 + 6 + 1)];  // the probe report we are building here
    char  temp[axesStringLen];
    strcpy(probe_rpt, "[PRB:");  // initialize the string with the first characters
    // get the machine position and put them into a string and append to the probe report
    system_convert_array_steps_to_mpos(print_position, sys_probe_position);
    report_util_axis_values(print_position, temp);
    strcat(probe_rpt, temp);
    // add the success indicator and add closing characters
    sprintf(temp, ":%d]\r\n", sys.probe_succeeded);
    strcat(probe_rpt, temp);
    grbl_send(client, probe_rpt);  // send the report
}

// Prints Grbl NGC parameters (coordinate offsets, probing)
void report_ngc_parameters(uint8_t client) {
    String ngc_rpt = "";

    // Print persistent offsets G54 - G59, G28, and G30
    for (auto coord_select = CoordIndex::Begin; coord_select < CoordIndex::End; ++coord_select) {
        ngc_rpt += "[";
        ngc_rpt += coords[coord_select]->getName();
        ngc_rpt += ":";
        ngc_rpt += report_util_axis_values(coords[coord_select]->get());
        ngc_rpt += "]\r\n";
    }
    ngc_rpt += "[G92:";  // Print non-persistent G92,G92.1
    ngc_rpt += report_util_axis_values(gc_state.coord_offset);
    ngc_rpt += "]\r\n";
    ngc_rpt += "[TLO:";  // Print tool length offset
    float tlo = gc_state.tool_length_offset;
    if (report_inches->get()) {
        tlo *= INCH_PER_MM;
    }
    ngc_rpt += String(tlo, 3);
    ;
    ngc_rpt += "]\r\n";
    grbl_send(client, ngc_rpt.c_str());
    report_probe_parameters(client);
}

// Print current gcode parser mode state
void report_gcode_modes(uint8_t client) {
    char        temp[20];
    char        modes_rpt[75];
    const char* mode = "";
    strcpy(modes_rpt, "[GC:");

    switch (gc_state.modal.motion) {
        case Motion::None:
            mode = "G80";
            break;
        case Motion::Seek:
            mode = "G0";
            break;
        case Motion::Linear:
            mode = "G1";
            break;
        case Motion::CwArc:
            mode = "G2";
            break;
        case Motion::CcwArc:
            mode = "G3";
            break;
        case Motion::ProbeToward:
            mode = "G38.1";
            break;
        case Motion::ProbeTowardNoError:
            mode = "G38.2";
            break;
        case Motion::ProbeAway:
            mode = "G38.3";
            break;
        case Motion::ProbeAwayNoError:
            mode = "G38.4";
            break;
    }
    strcat(modes_rpt, mode);

    sprintf(temp, " G%d", gc_state.modal.coord_select + 54);
    strcat(modes_rpt, temp);

    switch (gc_state.modal.plane_select) {
        case Plane::XY:
            mode = " G17";
            break;
        case Plane::ZX:
            mode = " G18";
            break;
        case Plane::YZ:
            mode = " G19";
            break;
    }
    strcat(modes_rpt, mode);

    switch (gc_state.modal.units) {
        case Units::Inches:
            mode = " G20";
            break;
        case Units::Mm:
            mode = " G21";
            break;
    }
    strcat(modes_rpt, mode);

    switch (gc_state.modal.distance) {
        case Distance::Absolute:
            mode = " G90";
            break;
        case Distance::Incremental:
            mode = " G91";
            break;
    }
    strcat(modes_rpt, mode);

#if 0
    switch (gc_state.modal.arc_distance) {
        case ArcDistance::Absolute: mode = " G90.1"; break;
        case ArcDistance::Incremental: mode = " G91.1"; break;
    }
    strcat(modes_rpt, mode);
#endif

    switch (gc_state.modal.feed_rate) {
        case FeedRate::UnitsPerMin:
            mode = " G94";
            break;
        case FeedRate::InverseTime:
            mode = " G93";
            break;
    }
    strcat(modes_rpt, mode);

    //report_util_gcode_modes_M();
    switch (gc_state.modal.program_flow) {
        case ProgramFlow::Running:
            mode = "";
            break;
        case ProgramFlow::Paused:
            mode = " M0";
            break;
        case ProgramFlow::OptionalStop:
            mode = " M1";
            break;
        case ProgramFlow::CompletedM2:
            mode = " M2";
            break;
        case ProgramFlow::CompletedM30:
            mode = " M30";
            break;
    }
    strcat(modes_rpt, mode);

    switch (gc_state.modal.spindle) {
        case SpindleState::Cw:
            mode = " M3";
            break;
        case SpindleState::Ccw:
            mode = " M4";
            break;
        case SpindleState::Disable:
            mode = " M5";
            break;
        default:
            mode = "";
    }
    strcat(modes_rpt, mode);

    //report_util_gcode_modes_M();  // optional M7 and M8 should have been dealt with by here
    auto coolant = gc_state.modal.coolant;
    if (!coolant.Mist && !coolant.Flood) {
        strcat(modes_rpt, " M9");
    } else {
        // Note: Multiple coolant states may be active at the same time.
        if (coolant.Mist) {
            strcat(modes_rpt, " M7");
        }
        if (coolant.Flood) {
            strcat(modes_rpt, " M8");
        }
    }

#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
    if (sys.override_ctrl == Override::ParkingMotion) {
        strcat(modes_rpt, " M56");
    }
#endif

    sprintf(temp, " T%d", gc_state.tool);
    strcat(modes_rpt, temp);
    sprintf(temp, report_inches->get() ? " F%.1f" : " F%.0f", gc_state.feed_rate);
    strcat(modes_rpt, temp);
    sprintf(temp, " S%d", uint32_t(gc_state.spindle_speed));
    strcat(modes_rpt, temp);
    strcat(modes_rpt, "]\r\n");
    grbl_send(client, modes_rpt);
}

// Prints specified startup line
void report_startup_line(uint8_t n, const char* line, uint8_t client) {
    grbl_sendf(client, "$N%d=%s\r\n", n, line);  // OK to send to all
}

void report_execute_startup_message(const char* line, Error status_code, uint8_t client) {
    grbl_sendf(client, ">%s:", line);  // OK to send to all
    report_status_message(status_code, client);
}

// Prints build info line
void report_build_info(const char* line, uint8_t client) {
    grbl_sendf(client, "[VER:%s.%s:%s]\r\n[OPT:", GRBL_VERSION, GRBL_VERSION_BUILD, line);
#ifdef COOLANT_MIST_PIN
    grbl_send(client, "M");  // TODO Need to deal with M8...it could be disabled
#endif
#ifdef PARKING_ENABLE
    grbl_send(client, "P");
#endif
#ifdef HOMING_SINGLE_AXIS_COMMANDS
    grbl_send(client, "H");
#endif
#ifdef LIMITS_TWO_SWITCHES_ON_AXES
    grbl_send(client, "L");
#endif
#ifdef ALLOW_FEED_OVERRIDE_DURING_PROBE_CYCLES
    grbl_send(client, "A");
#endif
#ifdef ENABLE_BLUETOOTH
    grbl_send(client, "B");
#endif
#ifdef ENABLE_SD_CARD
    grbl_send(client, "S");
#endif
#ifdef ENABLE_PARKING_OVERRIDE_CONTROL
    grbl_send(client, "R");
#endif
#if defined(ENABLE_WIFI)
    grbl_send(client, "W");
#endif
#ifndef ENABLE_RESTORE_WIPE_ALL  // NOTE: Shown when disabled.
    grbl_send(client, "*");
#endif
#ifndef ENABLE_RESTORE_DEFAULT_SETTINGS  // NOTE: Shown when disabled.
    grbl_send(client, "$");
#endif
#ifndef ENABLE_RESTORE_CLEAR_PARAMETERS  // NOTE: Shown when disabled.
    grbl_send(client, "#");
#endif
#ifndef FORCE_BUFFER_SYNC_DURING_NVS_WRITE  // NOTE: Shown when disabled.
    grbl_send(client, "E");
#endif
#ifndef FORCE_BUFFER_SYNC_DURING_WCO_CHANGE  // NOTE: Shown when disabled.
    grbl_send(client, "W");
#endif
    // NOTE: Compiled values, like override increments/max/min values, may be added at some point later.
    // These will likely have a comma delimiter to separate them.
    grbl_send(client, "]\r\n");
    report_machine_type(client);
#if defined(ENABLE_WIFI)
    grbl_send(client, (char*)WebUI::wifi_config.info());
#endif
#if defined(ENABLE_BLUETOOTH)
    grbl_send(client, (char*)WebUI::bt_config.info());
#endif
}

// Prints the character string line Grbl has received from the user, which has been pre-parsed,
// and has been sent into protocol_execute_line() routine to be executed by Grbl.
void report_echo_line_received(char* line, uint8_t client) {
    grbl_sendf(client, "[echo: %s]\r\n", line);
}

// Prints real-time data. This function grabs a real-time snapshot of the stepper subprogram
// and the actual location of the CNC machine. Users may change the following function to their
// specific needs, but the desired real-time data report must be as short as possible. This is
// requires as it minimizes the computational overhead and allows grbl to keep running smoothly,
// especially during g-code programs with fast, short line segments and high frequency reports (5-20Hz).
void report_realtime_status(uint8_t client) {
    uint8_t idx;
    int32_t current_position[MAX_N_AXIS];  // Copy current state of the system position variable
    memcpy(current_position, sys_position, sizeof(sys_position));
    float print_position[MAX_N_AXIS];
    char  status[200];
    char  temp[MAX_N_AXIS * 20];
    system_convert_array_steps_to_mpos(print_position, current_position);
    // Report current machine state and sub-states
    strcpy(status, "<");
    switch (sys.state) {
        case State::Idle:
            strcat(status, "Idle");
            break;
        case State::Cycle:
            strcat(status, "Run");
            break;
        case State::Hold:
            if (!(sys.suspend.bit.jogCancel)) {
                strcat(status, "Hold:");
                strcat(status, sys.suspend.bit.holdComplete ? "0" : "1");  // Ready to resume
                break;
            }  // Continues to print jog state during jog cancel.
        case State::Jog:
            strcat(status, "Jog");
            break;
        case State::Homing:
            strcat(status, "Home");
            break;
        case State::Alarm:
            strcat(status, "Alarm");
            break;
        case State::CheckMode:
            strcat(status, "Check");
            break;
        case State::SafetyDoor:
            strcat(status, "Door:");
            if (sys.suspend.bit.initiateRestore) {
                strcat(status, "3");  // Restoring
            } else {
                if (sys.suspend.bit.retractComplete) {
                    strcat(status, sys.suspend.bit.safetyDoorAjar ? "1" : "0");  // Door ajar
                    // Door closed and ready to resume
                } else {
                    strcat(status, "2");  // Retracting
                }
            }
            break;
        case State::Sleep:
            strcat(status, "Sleep");
            break;
    }
    float wco[MAX_N_AXIS];
    if (bit_isfalse(status_mask->get(), RtStatus::Position) || (sys.report_wco_counter == 0)) {
        auto n_axis = number_axis->get();
        for (idx = 0; idx < n_axis; idx++) {
            // Apply work coordinate offsets and tool length offset to current position.
            wco[idx] = gc_state.coord_system[idx] + gc_state.coord_offset[idx];
            if (idx == TOOL_LENGTH_OFFSET_AXIS) {
                wco[idx] += gc_state.tool_length_offset;
            }
            if (bit_isfalse(status_mask->get(), RtStatus::Position)) {
                print_position[idx] -= wco[idx];
            }
        }
    }
    forward_kinematics(print_position);  // a weak definition does nothing. Users can provide strong version
    // Report machine position
    if (bit_istrue(status_mask->get(), RtStatus::Position)) {
        strcat(status, "|MPos:");
    } else {
        strcat(status, "|WPos:");
    }
    report_util_axis_values(print_position, temp);
    strcat(status, temp);
    // Returns planner and serial read buffer states.
#ifdef REPORT_FIELD_BUFFER_STATE
    if (bit_istrue(status_mask->get(), RtStatus::Buffer)) {
        int bufsize = DEFAULTBUFFERSIZE;
#    if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)
        if (client == CLIENT_TELNET) {
            bufsize = WebUI::telnet_server.get_rx_buffer_available();
        }
#    endif  //ENABLE_WIFI && ENABLE_TELNET
#    if defined(ENABLE_BLUETOOTH)
        if (client == CLIENT_BT) {
            //TODO FIXME
            bufsize = 512 - WebUI::SerialBT.available();
        }
#    endif  //ENABLE_BLUETOOTH
        if (client == CLIENT_SERIAL) {
            bufsize = serial_get_rx_buffer_available(CLIENT_SERIAL);
        }
        sprintf(temp, "|Bf:%d,%d", plan_get_block_buffer_available(), bufsize);
        strcat(status, temp);
    }
#endif
#ifdef USE_LINE_NUMBERS
#    ifdef REPORT_FIELD_LINE_NUMBERS
    // Report current line number
    plan_block_t* cur_block = plan_get_current_block();
    if (cur_block != NULL) {
        uint32_t ln = cur_block->line_number;
        if (ln > 0) {
            sprintf(temp, "|Ln:%d", ln);
            strcat(status, temp);
        }
    }
#    endif
#endif
    // Report realtime feed speed
#ifdef REPORT_FIELD_CURRENT_FEED_SPEED
    if (report_inches->get()) {
        sprintf(temp, "|FS:%.1f,%d", st_get_realtime_rate() / MM_PER_INCH, sys.spindle_speed);
    } else {
        sprintf(temp, "|FS:%.0f,%d", st_get_realtime_rate(), sys.spindle_speed);
    }
    strcat(status, temp);
#endif
#ifdef REPORT_FIELD_PIN_STATE
    AxisMask    lim_pin_state  = limits_get_state();
    ControlPins ctrl_pin_state = system_control_get_state();
    bool        prb_pin_state  = probe_get_state();
    if (lim_pin_state || ctrl_pin_state.value || prb_pin_state) {
        strcat(status, "|Pn:");
        if (prb_pin_state) {
            strcat(status, "P");
        }
        if (lim_pin_state) {
            auto n_axis = number_axis->get();
            if (n_axis >= 1 && bit_istrue(lim_pin_state, bit(X_AXIS))) {
                strcat(status, "X");
            }
            if (n_axis >= 2 && bit_istrue(lim_pin_state, bit(Y_AXIS))) {
                strcat(status, "Y");
            }
            if (n_axis >= 3 && bit_istrue(lim_pin_state, bit(Z_AXIS))) {
                strcat(status, "Z");
            }
            if (n_axis >= 4 && bit_istrue(lim_pin_state, bit(A_AXIS))) {
                strcat(status, "A");
            }
            if (n_axis >= 5 && bit_istrue(lim_pin_state, bit(B_AXIS))) {
                strcat(status, "B");
            }
            if (n_axis >= 6 && bit_istrue(lim_pin_state, bit(C_AXIS))) {
                strcat(status, "C");
            }
        }
        if (ctrl_pin_state.value) {
            if (ctrl_pin_state.bit.safetyDoor) {
                strcat(status, "D");
            }
            if (ctrl_pin_state.bit.reset) {
                strcat(status, "R");
            }
            if (ctrl_pin_state.bit.feedHold) {
                strcat(status, "H");
            }
            if (ctrl_pin_state.bit.cycleStart) {
                strcat(status, "S");
            }
            if (ctrl_pin_state.bit.macro0) {
                strcat(status, "0");
            }
            if (ctrl_pin_state.bit.macro1) {
                strcat(status, "1");
            }
            if (ctrl_pin_state.bit.macro2) {
                strcat(status, "2");
            }
            if (ctrl_pin_state.bit.macro3) {
                strcat(status, "3");
            }
        }
    }
#endif
#ifdef REPORT_FIELD_WORK_COORD_OFFSET
    if (sys.report_wco_counter > 0) {
        sys.report_wco_counter--;
    } else {
        switch (sys.state) {
            case State::Homing:
            case State::Cycle:
            case State::Hold:
            case State::Jog:
            case State::SafetyDoor:
                sys.report_wco_counter = (REPORT_WCO_REFRESH_BUSY_COUNT - 1);  // Reset counter for slow refresh
            default:
                sys.report_wco_counter = (REPORT_WCO_REFRESH_IDLE_COUNT - 1);
                break;
        }
        if (sys.report_ovr_counter == 0) {
            sys.report_ovr_counter = 1;  // Set override on next report.
        }
        strcat(status, "|WCO:");
        report_util_axis_values(wco, temp);
        strcat(status, temp);
    }
#endif
#ifdef REPORT_FIELD_OVERRIDES
    if (sys.report_ovr_counter > 0) {
        sys.report_ovr_counter--;
    } else {
        switch (sys.state) {
            case State::Homing:
            case State::Cycle:
            case State::Hold:
            case State::Jog:
            case State::SafetyDoor:
                sys.report_ovr_counter = (REPORT_OVR_REFRESH_BUSY_COUNT - 1);  // Reset counter for slow refresh
            default:
                sys.report_ovr_counter = (REPORT_OVR_REFRESH_IDLE_COUNT - 1);
                break;
        }

        sprintf(temp, "|Ov:%d,%d,%d", sys.f_override, sys.r_override, sys.spindle_speed_ovr);
        strcat(status, temp);
        SpindleState sp_state      = spindle->get_state();
        CoolantState coolant_state = coolant_get_state();
        if (sp_state != SpindleState::Disable || coolant_state.Mist || coolant_state.Flood) {
            strcat(status, "|A:");
            switch (sp_state) {
                case SpindleState::Disable:
                    break;
                case SpindleState::Cw:
                    strcat(status, "S");
                    break;
                case SpindleState::Ccw:
                    strcat(status, "C");
                    break;
            }

            auto coolant = coolant_state;
            if (coolant.Flood) {
                strcat(status, "F");
            }
#    ifdef COOLANT_MIST_PIN  // TODO Deal with M8 - Flood
            if (coolant.Mist) {
                strcat(status, "M");
            }
#    endif
        }
    }
#endif
#ifdef ENABLE_SD_CARD
    if (get_sd_state(false) == SDState::BusyPrinting) {
        sprintf(temp, "|SD:%4.2f,", sd_report_perc_complete());
        strcat(status, temp);
        sd_get_current_filename(temp);
        strcat(status, temp);
    }
#endif
#ifdef REPORT_HEAP
    sprintf(temp, "|Heap:%d", esp.getHeapSize());
    strcat(status, temp);
#endif
    strcat(status, ">\r\n");
    grbl_send(client, status);
}

void report_realtime_steps() {
    uint8_t idx;
    auto    n_axis = number_axis->get();
    for (idx = 0; idx < n_axis; idx++) {
        grbl_sendf(CLIENT_ALL, "%ld\n", sys_position[idx]);  // OK to send to all ... debug stuff
    }
}

void report_gcode_comment(char* comment) {
    char          msg[80];
    const uint8_t offset = 4;  // ignore "MSG_" part of comment
    uint8_t       index  = offset;
    if (strstr(comment, "MSG")) {
        while (index < strlen(comment)) {
            msg[index - offset] = comment[index];
            index++;
        }
        msg[index - offset] = 0;  // null terminate
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "GCode Comment...%s", msg);
    }
}

void report_machine_type(uint8_t client) {
    grbl_msg_sendf(client, MsgLevel::Info, "Using machine:%s", MACHINE_NAME);
}

/*
    Print a message in hex format
    Ex: report_hex_msg(msg, "Rx:", 6);
    Would would print something like ... [MSG Rx: 0x01 0x03 0x01 0x08 0x31 0xbf]
*/
void report_hex_msg(char* buf, const char* prefix, int len) {
    char report[200];
    char temp[20];
    sprintf(report, "%s", prefix);
    for (int i = 0; i < len; i++) {
        sprintf(temp, " 0x%02X", buf[i]);
        strcat(report, temp);
    }

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s", report);
}

void report_hex_msg(uint8_t* buf, const char* prefix, int len) {
    char report[200];
    char temp[20];
    sprintf(report, "%s", prefix);
    for (int i = 0; i < len; i++) {
        sprintf(temp, " 0x%02X", buf[i]);
        strcat(report, temp);
    }

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s", report);
}

char report_get_axis_letter(uint8_t axis) {
    switch (axis) {
        case X_AXIS:
            return 'X';
        case Y_AXIS:
            return 'Y';
        case Z_AXIS:
            return 'Z';
        case A_AXIS:
            return 'A';
        case B_AXIS:
            return 'B';
        case C_AXIS:
            return 'C';
        default:
            return '?';
    }
}

char* reportAxisLimitsMsg(uint8_t axis) {
    static char msg[40];
    sprintf(msg, "Limits(%0.3f,%0.3f)", limitsMinPosition(axis), limitsMaxPosition(axis));
    return msg;
}

char* reportAxisNameMsg(uint8_t axis, uint8_t dual_axis) {
    static char name[10];
    sprintf(name, "%c%c Axis", report_get_axis_letter(axis), dual_axis ? '2' : ' ');
    return name;
}

char* reportAxisNameMsg(uint8_t axis) {
    static char name[10];
    sprintf(name, "%c  Axis", report_get_axis_letter(axis));
    return name;
}

void reportTaskStackSize(UBaseType_t& saved) {
#ifdef DEBUG_REPORT_STACK_FREE
    UBaseType_t newHighWater = uxTaskGetStackHighWaterMark(NULL);
    if (newHighWater != saved) {
        saved = newHighWater;
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s Min Stack Space: %d", pcTaskGetTaskName(NULL), saved);
    }
#endif
}

void __attribute__((weak)) forward_kinematics(float* position) {}  // This version does nothing. Make your own to do something with it

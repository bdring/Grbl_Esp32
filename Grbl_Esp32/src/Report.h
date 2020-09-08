#pragma once

/*
  Report.h - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

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

// Define Grbl status codes. Valid values (0-255)
const int STATUS_OK = 0; 
const int STATUS_EXPECTED_COMMAND_LETTER = 1; 
const int STATUS_BAD_NUMBER_FORMAT = 2; 
const int STATUS_INVALID_STATEMENT = 3; 
const int STATUS_NEGATIVE_VALUE = 4; 
const int STATUS_SETTING_DISABLED = 5; 
const int STATUS_SETTING_STEP_PULSE_MIN = 6; 
const int STATUS_SETTING_READ_FAIL = 7; 
const int STATUS_IDLE_ERROR = 8; 
const int STATUS_SYSTEM_GC_LOCK = 9; 
const int STATUS_SOFT_LIMIT_ERROR = 10; 
const int STATUS_OVERFLOW = 11; 
const int STATUS_MAX_STEP_RATE_EXCEEDED = 12; 
const int STATUS_CHECK_DOOR = 13; 
const int STATUS_LINE_LENGTH_EXCEEDED = 14; 
const int STATUS_TRAVEL_EXCEEDED = 15; 
const int STATUS_INVALID_JOG_COMMAND = 16; 
const int STATUS_SETTING_DISABLED_LASER = 17; 

const int STATUS_GCODE_UNSUPPORTED_COMMAND = 20; 
const int STATUS_GCODE_MODAL_GROUP_VIOLATION = 21; 
const int STATUS_GCODE_UNDEFINED_FEED_RATE = 22; 
const int STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER = 23; 
const int STATUS_GCODE_AXIS_COMMAND_CONFLICT = 24; 
const int STATUS_GCODE_WORD_REPEATED = 25; 
const int STATUS_GCODE_NO_AXIS_WORDS = 26; 
const int STATUS_GCODE_INVALID_LINE_NUMBER = 27; 
const int STATUS_GCODE_VALUE_WORD_MISSING = 28; 
const int STATUS_GCODE_UNSUPPORTED_COORD_SYS = 29; 
const int STATUS_GCODE_G53_INVALID_MOTION_MODE = 30; 
const int STATUS_GCODE_AXIS_WORDS_EXIST = 31; 
const int STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE = 32; 
const int STATUS_GCODE_INVALID_TARGET = 33; 
const int STATUS_GCODE_ARC_RADIUS_ERROR = 34; 
const int STATUS_GCODE_NO_OFFSETS_IN_PLANE = 35; 
const int STATUS_GCODE_UNUSED_WORDS = 36; 
const int STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR = 37; 
const int STATUS_GCODE_MAX_VALUE_EXCEEDED = 38; 
const int STATUS_P_PARAM_MAX_EXCEEDED = 39; 

const int STATUS_SD_FAILED_MOUNT = 60; // SD Failed to mount
const int STATUS_SD_FAILED_READ = 61; // SD Failed to read file
const int STATUS_SD_FAILED_OPEN_DIR = 62; // SD card failed to open directory
const int STATUS_SD_DIR_NOT_FOUND = 63; // SD Card directory not found
const int STATUS_SD_FILE_EMPTY = 64; // SD Card directory not found
const int STATUS_SD_FILE_NOT_FOUND = 65; // SD Card file not found
const int STATUS_SD_FAILED_OPEN_FILE = 66; // SD card failed to open file
const int STATUS_SD_FAILED_BUSY = 67; // SD card is busy
const int STATUS_SD_FAILED_DEL_DIR = 68; 
const int STATUS_SD_FAILED_DEL_FILE = 69; 

const int STATUS_BT_FAIL_BEGIN = 70; // Bluetooth failed to start
const int STATUS_WIFI_FAIL_BEGIN = 71; // WiFi failed to start

const int STATUS_NUMBER_RANGE = 80; // Setting number range problem
const int STATUS_INVALID_VALUE = 81; // Setting string problem

const int STATUS_MESSAGE_FAILED = 90; 

const int STATUS_NVS_SET_FAILED = 100; 

const int STATUS_AUTHENTICATION_FAILED = 110; 
const int STATUS_EOL = 111; 

typedef uint8_t err_t;  // For status codes
const char*     errorString(err_t errorNumber);

// Define Grbl alarm codes. Valid values (1-255). 0 is reserved.
const int ALARM_ABORT_CYCLE = EXEC_ALARM_ABORT_CYCLE; 
const int ALARM_PROBE_FAIL_INITIAL = EXEC_ALARM_PROBE_FAIL_INITIAL; 
const int ALARM_PROBE_FAIL_CONTACT = EXEC_ALARM_PROBE_FAIL_CONTACT; 
const int ALARM_HOMING_FAIL_RESET = EXEC_ALARM_HOMING_FAIL_RESET; 
const int ALARM_HOMING_FAIL_DOOR = EXEC_ALARM_HOMING_FAIL_DOOR; 
const int ALARM_HOMING_FAIL_PULLOFF = EXEC_ALARM_HOMING_FAIL_PULLOFF; 
const int ALARM_HOMING_FAIL_APPROACH = EXEC_ALARM_HOMING_FAIL_APPROACH; 

// Define Grbl feedback message codes. Valid values (0-255).
const int MESSAGE_CRITICAL_EVENT = 1; 
const int MESSAGE_ALARM_LOCK = 2; 
const int MESSAGE_ALARM_UNLOCK = 3; 
const int MESSAGE_ENABLED = 4; 
const int MESSAGE_DISABLED = 5; 
const int MESSAGE_SAFETY_DOOR_AJAR = 6; 
const int MESSAGE_CHECK_LIMITS = 7; 
const int MESSAGE_PROGRAM_END = 8; 
const int MESSAGE_RESTORE_DEFAULTS = 9; 
const int MESSAGE_SPINDLE_RESTORE = 10; 
const int MESSAGE_SLEEP_MODE = 11; 
const int MESSAGE_SD_FILE_QUIT = 60; // mc_reset was called during an SD job

const int CLIENT_SERIAL = 0; 
const int CLIENT_BT = 1; 
const int CLIENT_WEBUI = 2; 
const int CLIENT_TELNET = 3; 
const int CLIENT_INPUT = 4; 
const int CLIENT_ALL = 0xFF; 
const int CLIENT_COUNT = 5; // total number of client types regardless if they are used

const int MSG_LEVEL_ERROR = 1; 
const int MSG_LEVEL_WARNING = 2; 
const int MSG_LEVEL_INFO = 3; 
const int MSG_LEVEL_DEBUG = 4; 

// functions to send data to the user.
void grbl_send(uint8_t client, const char* text);
void grbl_sendf(uint8_t client, const char* format, ...);
void grbl_msg_sendf(uint8_t client, uint8_t level, const char* format, ...);

//function to notify
void grbl_notify(const char* title, const char* msg);
void grbl_notifyf(const char* title, const char* format, ...);

// Prints system status messages.
void report_status_message(uint8_t status_code, uint8_t client);
void report_realtime_steps();

// Prints system alarm messages.
void report_alarm_message(uint8_t alarm_code);

// Prints miscellaneous feedback messages.
void report_feedback_message(uint8_t message_code);

// Prints welcome message
void report_init_message(uint8_t client);

// Prints Grbl help and current global settings
void report_grbl_help(uint8_t client);

// Prints Grbl global settings
void report_grbl_settings(uint8_t client, uint8_t show_extended);

// Prints an echo of the pre-parsed line received right before execution.
void report_echo_line_received(char* line, uint8_t client);

// Prints realtime status report
void report_realtime_status(uint8_t client);

// Prints recorded probe position
void report_probe_parameters(uint8_t client);

// Prints Grbl NGC parameters (coordinate offsets, probe)
void report_ngc_parameters(uint8_t client);

// Prints current g-code parser mode state
void report_gcode_modes(uint8_t client);

// Prints startup line when requested and executed.
void report_startup_line(uint8_t n, const char* line, uint8_t client);
void report_execute_startup_message(const char* line, uint8_t status_code, uint8_t client);

// Prints build info and user info
void report_build_info(char* line, uint8_t client);

void report_gcode_comment(char* comment);

#ifdef DEBUG
void report_realtime_debug();
#endif

void report_machine_type(uint8_t client);

void report_hex_msg(char* buf, const char* prefix, int len);
void report_hex_msg(uint8_t* buf, const char* prefix, int len);

char report_get_axis_letter(uint8_t axis);

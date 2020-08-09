#pragma once

/*
  grbl.h - Header for system level commands and real-time processes
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

// Grbl versioning system

#define GRBL_VERSION "1.3a"
#define GRBL_VERSION_BUILD "20200808"

//#include <sdkconfig.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <driver/rmt.h>
#include <esp_task_wdt.h>
#include <freertos/task.h>
#include <Preferences.h>

#include "driver/timer.h"

// Define the Grbl system include files. NOTE: Do not alter organization.
#include "config.h"
#include "nuts_bolts.h"
#include "tdef.h"

#include "defaults.h"
#include "settings.h"
#include "authentication.h"
#include "system.h"

#include "planner.h"
#include "coolant_control.h"
#include "grbl_eeprom.h"
#include "gcode.h"
#include "grbl_limits.h"
#include "motion_control.h"
#include "probe.h"
#include "protocol.h"
#include "report.h"
#include "serial.h"
#include "Pins.h"
#include "Spindles/Spindle.h"
#include "Motors/Motors.h"
#include "stepper.h"
#include "jog.h"
#include "inputbuffer.h"
#include "commands.h"
#include "SettingsClass.h"
#include "SettingsDefinitions.h"
#include "WebSettings.h"

// Do not guard this because it is needed for local files too
#include "grbl_sd.h"

#ifdef ENABLE_BLUETOOTH
#    include "BTconfig.h"
#endif

#ifdef ENABLE_WIFI
#    include "wificonfig.h"
#    ifdef ENABLE_HTTP
#        include "serial2socket.h"
#    endif
#    ifdef ENABLE_TELNET
#        include "telnet_server.h"
#    endif
#    ifdef ENABLE_NOTIFICATIONS
#        include "notifications_service.h"
#    endif
#endif

#include "solenoid_pen.h"

#ifdef USE_SERVO_AXES
#    include "servo_axis.h"
#endif

#ifdef USE_UNIPOLAR
#    include "grbl_unipolar.h"
#endif

#ifdef USE_I2S_OUT
#    include "i2s_out.h"
#endif

// Called if USE_MACHINE_INIT is defined
void machine_init();

// Called if USE_CUSTOM_HOMING is defined
bool user_defined_homing();

// Called if USE_KINEMATICS is defined
void inverse_kinematics(float* target, plan_line_data_t* pl_data, float* position);
bool kinematics_pre_homing(uint8_t cycle_mask);
void kinematics_post_homing();

// Called if USE_FWD_KINEMATIC is defined
void forward_kinematics(float* position);

// Called if MACRO_BUTTON_0_PIN or MACRO_BUTTON_1_PIN or MACRO_BUTTON_2_PIN is defined
void user_defined_macro(uint8_t index);

// Called if USE_M30 is defined
void user_m30();

// Called if USE_TOOL_CHANGE is defined
void user_tool_change(uint8_t new_tool);

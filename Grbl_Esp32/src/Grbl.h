#pragma once

/*
  Grbl.h - Header for system level commands and real-time processes
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

const char* const GRBL_VERSION       = "2.1a";
const char* const GRBL_VERSION_BUILD = "20210326";

//#include <sdkconfig.h>
#include <Arduino.h>
#include <EEPROM.h>
#include <driver/rmt.h>
#include <esp_task_wdt.h>
#include <freertos/task.h>
#include <Preferences.h>

#include <driver/timer.h>

// Define the Grbl system include files. NOTE: Do not alter organization.
#include "Config.h"
#include "NutsBolts.h"

#include "Defaults.h"
#include "Error.h"
#include "WebUI/Authentication.h"
#include "WebUI/Commands.h"
#include "Probe.h"
#include "System.h"

#include "GCode.h"
#include "Planner.h"
#include "CoolantControl.h"
#include "Limits.h"
#include "MotionControl.h"
#include "Protocol.h"
#include "Uart.h"
#include "Serial.h"
#include "Report.h"
#include "Pin.h"
#include "Spindles/Spindle.h"
#include "Stepper.h"
#include "Jog.h"
#include "WebUI/InputBuffer.h"
#include "Settings.h"
#include "SettingsDefinitions.h"
#include "WebUI/WebSettings.h"
#include "ControlPin.h"

#include "UserOutput.h"

// Do not guard this because it is needed for local files too
#include "SDCard.h"

// These WebUI components create stub classes if not enabled
#include "WebUI/WifiConfig.h"
#include "WebUI/TelnetServer.h"
#include "WebUI/Serial2Socket.h"
#include "WebUI/NotificationsService.h"

#include "I2SOut.h"

void grbl_init();
void run_once();

// Weak definitions that can be overridden
void machine_init();
void display_init();
bool user_defined_homing(AxisMask cycle_mask);

// weak definitions in MotionControl.cpp
bool inverse_kinematics(float* target, plan_line_data_t* pl_data, float* position);
bool kinematics_pre_homing(AxisMask cycle_mask);
void kinematics_post_homing();

bool limitsCheckTravel(float* target);  // weak in Limits.cpp; true if out of range

void inverse_kinematics(float* position);  // used to return a converted value
void forward_kinematics(float* position);

void user_defined_macro(uint8_t index);

void user_m30();

void user_tool_change(uint8_t new_tool);

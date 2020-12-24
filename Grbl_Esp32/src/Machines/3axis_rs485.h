#pragma once
// clang-format off

/*
	3axis_xyx.h
	Part of Grbl_ESP32

	This is a general XYZ-axis RS-485 CNC machine. The schematic is quite 
	easy, you basically need a MAX485 wired through a logic level converter
	for the VFD, and a few pins wired through an ULN2803A to the external 
	stepper drivers. It's common to have a dual gantry for the Y axis.

	Optional limit pins are slightly more difficult, as these require a 
	Schmitt trigger and optocouplers.

	2020    - Stefan de Bruijn

	Grbl_ESP32 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	Grbl is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#define MACHINE_NAME            "ESP32_XYZ_RS485"

#define X_MOTOR_TYPE            MotorType::StepStick
#define X_STEP_PIN              "gpio.4"   // labeled X
#define X_DIRECTION_PIN         "gpio.16"  // labeled X

#define Y_MOTOR_TYPE            MotorType::StepStick
#define Y_STEP_PIN              "gpio.17"  // labeled Y
#define Y_DIRECTION_PIN         "gpio.18"  // labeled Y

#define Y2_MOTOR_TYPE            MotorType::StepStick
#define Y2_STEP_PIN             "gpio.19"  // labeled Y2
#define Y2_DIRECTION_PIN        "gpio.21"  // labeled Y2

#define Z_MOTOR_TYPE            MotorType::StepStick
#define Z_STEP_PIN              "gpio.22"  // labeled Z
#define Z_DIRECTION_PIN         "gpio.23"  // labeled Z

#define SPINDLE_TYPE            SpindleType::H2A
#define VFD_RS485_TXD_PIN       "gpio.13" // RS485 TX
#define VFD_RS485_RTS_PIN       "gpio.15" // RS485 RTS
#define VFD_RS485_RXD_PIN       "gpio.2"  // RS485 RX

#define X_LIMIT_PIN             "gpio.33"
#define Y_LIMIT_PIN             "gpio.32"
#define Y2_LIMIT_PIN            "gpio.35"
#define Z_LIMIT_PIN             "gpio.34"

// Set $Homing/Cycle0=X and $Homing/Cycle=XY

#define PROBE_PIN               "gpio.14"  // labeled Probe
#define CONTROL_RESET_PIN       "gpio.27"  // labeled Reset
#define CONTROL_FEED_HOLD_PIN   "gpio.26"  // labeled Hold
#define CONTROL_CYCLE_START_PIN "gpio.25"  // labeled Start

// #define VFD_DEBUG_MODE

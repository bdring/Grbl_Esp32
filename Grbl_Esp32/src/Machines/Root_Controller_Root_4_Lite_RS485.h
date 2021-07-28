#pragma once
// clang-format off

/*
    Root_Controller_Root_4_Lite_RS485.h

    Covers initial release version

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S Root Controller 6-axis board
    2018    - Bart Dring
    2020    - Mitch Bradley
    2020    - Michiyasu Odaki
	2020 	- Pete Newbery 
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
#define MACHINE_NAME            "Root Controller 3 Axis XYYZ"

#define N_AXIS 3

// === Special Features

//**I2S (steppers & other output-only pins)
//#define DEFAULT_STEPPER ST_I2S_STATIC
#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

//Setup I2S Clocking
#define I2S_OUT_BCK      "gpio.22"
#define I2S_OUT_WS       "gpio.21"
#define I2S_OUT_DATA     "gpio.12"


//**Motor Socket #1
#define X_DISABLE_PIN           "i2so.7"
#define X_DIRECTION_PIN         "i2so.6"
#define X_STEP_PIN              "i2so.5"
//**Motor Socket #2
#define Y_DISABLE_PIN           "i2so.4"
#define Y_DIRECTION_PIN         "i2so.3"
#define Y_STEP_PIN              "i2so.2"
//**Motor Socket #3
#define Y2_DISABLE_PIN          "i2so.1"
#define Y2_DIRECTION_PIN        "i2so.0"
#define Y2_STEP_PIN             "i2so.15"
//**Motor Socket #4
#define Z_DISABLE_PIN           "i2so.14"
#define Z_DIRECTION_PIN         "i2so.13"
#define Z_STEP_PIN              "i2so.12"

//**Motion Control 
//200pulses/rev stepper motor with SFU1204 ballscrew with a pitch of 4mm
//equates to 50 Steps/mm * micro stepping
//Steps per MM
#define DEFAULT_X_STEPS_PER_MM 800 
#define DEFAULT_Y_STEPS_PER_MM 800
#define DEFAULT_Z_STEPS_PER_MM 1000 // 50 Steps/mm * micro stepping * belt ratio 
//**Max Feedrate
#define DEFAULT_X_MAX_RATE		1000
#define DEFAULT_Y_MAX_RATE		1000
#define DEFAULT_Z_MAX_RATE		1000
//**Acceleration
#define DEFAULT_X_ACCELERATION 	50
#define DEFAULT_Y_ACCELERATION 	50
#define DEFAULT_Z_ACCELERATION 	50
//**Max travel
#define DEFAULT_X_MAX_TRAVEL 	220
#define DEFAULT_Y_MAX_TRAVEL 	278
#define DEFAULT_Z_MAX_TRAVEL 	60


//**Storage
#define ENABLE_SD_CARD

//**Endstop pins
#define X_LIMIT_PIN             "gpio.2"
#define Y_LIMIT_PIN             "gpio.26"
#define Y2_LIMIT_PIN            "gpio.27"
#define Z_LIMIT_PIN             "gpio.14"
#define PROBE_PIN               "gpio.33" 
#define INVERT_CONTROL_PIN_MASK 1
#define DEFAULT_INVERT_LIMIT_PINS 7 // Enable for NC switch types
//**Homing Routine
#define DEFAULT_HOMING_ENABLE       1
#define DEFAULT_HOMING_DIR_MASK 	0
#define DEFAULT_SOFT_LIMIT_ENABLE	1
#define DEFAULT_HARD_LIMIT_ENABLE	1
#define DEFAULT_HOMING_SQUARED_AXES (bitnum_to_mask(Y_AXIS))
#define DEFAULT_HOMING_FEED_RATE 	100
#define DEFAULT_HOMING_SEEK_RATE 	800
#define DEFAULT_HOMING_PULLOFF 		2
#define HOMING_INIT_LOCK
#define LIMITS_TWO_SWITCHES_ON_AXES 1
#define DEFAULT_HOMING_CYCLE_0		(1<<Z_AXIS) 
#define DEFAULT_HOMING_CYCLE_1  	(1<<X_AXIS) 
#define DEFAULT_HOMING_CYCLE_2		(1<<Y_AXIS) 
//#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255


#define USER_DIGITAL_PIN_0     		"i2so.17"
#define USER_DIGITAL_PIN_1     		"i2so.18"
#define USER_DIGITAL_PIN_2     		"i2so.19"


//**Spindle SETUP
//#define DEBUG_VFD
#define SPINDLE_TYPE            SpindleType::HUANYANG // only one spindle at a time
#define VFD_RS485_TXD_PIN		"gpio.17"
#define VFD_RS485_RXD_PIN		"gpio.16"
#define VFD_RS485_RTS_PIN		"gpio.4"
#define DEFAULT_SPINDLE_RPM_MAX 24000
#define DEFAULT_SPINDLE_RPM_MIN 1000
#define DEFAULT_SPINDLE_DELAY_SPINDOWN 10
#define DEFAULT_SPINDLE_DELAY_SPINUP 10
//Setup AUX CNC outputs
#define COOLANT_MIST_PIN        "i2so.20"  // labeled Mist
#define COOLANT_FLOOD_PIN       "i2so.21"  // labeled Flood

//On PCB control buttons
#define CONTROL_RESET_PIN       "gpio.13"  // labeled "GRBL RST"
#define CONTROL_FEED_HOLD_PIN   "gpio.36"  // labeled "HOLD"
#define CONTROL_CYCLE_START_PIN "gpio.39"  // labeled "START"
#define MACRO_BUTTON_0_PIN		"gpio.34"  // labeled "START"

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

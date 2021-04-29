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
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC
#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

//Setup I2S Clocking
#define I2S_OUT_BCK      GPIO_NUM_22
#define I2S_OUT_WS       GPIO_NUM_21
#define I2S_OUT_DATA     GPIO_NUM_12


//**Motor Socket #1
#define X_DISABLE_PIN           I2SO(7)
#define X_DIRECTION_PIN         I2SO(6)
#define X_STEP_PIN              I2SO(5)
//**Motor Socket #2
#define Y_DISABLE_PIN           I2SO(4)
#define Y_DIRECTION_PIN         I2SO(3)
#define Y_STEP_PIN              I2SO(2)
//**Motor Socket #3
#define Y2_DISABLE_PIN          I2SO(1)
#define Y2_DIRECTION_PIN        I2SO(0)
#define Y2_STEP_PIN             I2SO(15)
//**Motor Socket #4
#define Z_DISABLE_PIN           I2SO(14)
#define Z_DIRECTION_PIN         I2SO(13)
#define Z_STEP_PIN              I2SO(12)

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
#define X_LIMIT_PIN             GPIO_NUM_2
#define Y_LIMIT_PIN             GPIO_NUM_26
#define Y2_LIMIT_PIN            GPIO_NUM_27
#define Z_LIMIT_PIN             GPIO_NUM_14
#define PROBE_PIN               GPIO_NUM_33 
#define INVERT_CONTROL_PIN_MASK 1
#define DEFAULT_INVERT_LIMIT_PINS 7 // Enable for NC switch types
//**Homing Routine
#define DEFAULT_HOMING_ENABLE       1
#define DEFAULT_HOMING_DIR_MASK 	0
#define DEFAULT_SOFT_LIMIT_ENABLE	1
#define DEFAULT_HARD_LIMIT_ENABLE	1
#define DEFAULT_HOMING_SQUARED_AXES (bit(Y_AXIS))
#define DEFAULT_HOMING_FEED_RATE 	100
#define DEFAULT_HOMING_SEEK_RATE 	800
#define DEFAULT_HOMING_PULLOFF 		2
#define HOMING_INIT_LOCK
#define LIMITS_TWO_SWITCHES_ON_AXES 1
#define DEFAULT_HOMING_CYCLE_0		(1<<Z_AXIS) 
#define DEFAULT_HOMING_CYCLE_1  	(1<<X_AXIS) 
#define DEFAULT_HOMING_CYCLE_2		(1<<Y_AXIS) 
//#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255


#define USER_DIGITAL_PIN_0     		I2SO(17)
#define USER_DIGITAL_PIN_1     		I2SO(18)
#define USER_DIGITAL_PIN_2     		I2SO(19)


//**Spindle SETUP
//#define VFD_DEBUG_MODE
#define SPINDLE_TYPE            SpindleType::HUANYANG // only one spindle at a time
#define VFD_RS485_TXD_PIN		GPIO_NUM_17
#define VFD_RS485_RXD_PIN		GPIO_NUM_16
#define VFD_RS485_RTS_PIN		GPIO_NUM_4
#define DEFAULT_SPINDLE_RPM_MAX 24000
#define DEFAULT_SPINDLE_RPM_MIN 1000
#define DEFAULT_SPINDLE_DELAY_SPINDOWN 10
#define DEFAULT_SPINDLE_DELAY_SPINUP 10
//Setup AUX CNC outputs
#define COOLANT_MIST_PIN        I2SO(20)  // labeled Mist
#define COOLANT_FLOOD_PIN       I2SO(21)  // labeled Flood

//On PCB control buttons
#define CONTROL_RESET_PIN       GPIO_NUM_13  // labeled "GRBL RST"
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_36  // labeled "HOLD"
#define CONTROL_CYCLE_START_PIN GPIO_NUM_39  // labeled "START"
#define MACRO_BUTTON_0_PIN		GPIO_NUM_34  // labeled "START"

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

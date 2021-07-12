#pragma once
// clang-format off

/*
    CNC_xPRO_V5_XYYZ_PWM_NO.h
    Part of Grbl_ESP32

    Pin assignments for a 3-axis with Y ganged using Triaminic drivers
    in daisy-chained SPI mode.
    https://github.com/bdring/4_Axis_SPI_CNC

    2019    - Bart Dring
    2020    - Mitch Bradley
    2020    - Spark Concepts 

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

#define MACHINE_NAME "CNC_xPRO_V5_XYYZ_PWM_NO"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 3 // can be 3 or 4. (if 3 install bypass jumper next to the A driver)

#define TRINAMIC_DAISY_CHAIN

#define TRINAMIC_RUN_MODE           TrinamicMode :: CoolStep
#define TRINAMIC_HOMING_MODE        TrinamicMode :: CoolStep

// Use SPI enable instead of the enable pin
// The hardware enable pin is tied to ground
#define USE_TRINAMIC_ENABLE

//#define DEFAULT_HOMING_SQUARED_AXES bit(Y_AXIS)

// Y motor connects to the 1st driver
#define X_TRINAMIC_DRIVER       5160        // Which Driver Type?
#define X_RSENSE                0.05f
#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_14
#define X_CS_PIN                GPIO_NUM_17  // Daisy Chain, all share same CS pin

// Y motor connects to the 2nd driver
#define Y_TRINAMIC_DRIVER       5160        // Which Driver Type?
#define Y_RSENSE                0.05f
#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_26
#define Y_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin

// Y2 motor connects to the 2nd driver
#define Y2_TRINAMIC_DRIVER       5160        // Which Driver Type?
#define Y2_RSENSE                0.05f
#define Y2_STEP_PIN              GPIO_NUM_33  // Z on schem
#define Y2_DIRECTION_PIN         GPIO_NUM_32   // Z on schem
#define Y2_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin


// Z Axis motor connects to the 4th driver
#define Z_TRINAMIC_DRIVER       5160        // Which Driver Type?
#define Z_RSENSE                0.05f
#define Z_STEP_PIN              GPIO_NUM_15 // A on schem
#define Z_DIRECTION_PIN         GPIO_NUM_2 // A on schem
#define Z_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin

// Mist is a 3.3V output
// Turn on with M7 and off with M9
#define COOLANT_MIST_PIN        GPIO_NUM_21
#define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_25
#define SPINDLE_ENABLE_PIN      GPIO_NUM_4
#define VFD_RS485_TXD_PIN		GPIO_NUM_4
#define VFD_RS485_RXD_PIN		GPIO_NUM_25

// Relay operation
// Install Jumper near relay
// For spindle Use max RPM of 1
// For PWM remove jumper and set MAX RPM to something higher ($30 setting)
// Interlock jumper along top edge needs to be installed for both versions
#define DEFAULT_SPINDLE_RPM_MAX     12000 // Should be 1 for relay operation

#define PROBE_PIN               GPIO_NUM_22
//#define SHOW_EXTENDED_SETTINGS
#define X_LIMIT_PIN             GPIO_NUM_35
//#define A_LIMIT_PIN           GPIO_NUM_36
#define Y_LIMIT_PIN             GPIO_NUM_34
#define Z_LIMIT_PIN             GPIO_NUM_39
#define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_16


// Define default configuration
 //steps per mm
 #define DEFAULT_X_STEPS_PER_MM 200.0
 #define DEFAULT_Y_STEPS_PER_MM 200.0
 #define DEFAULT_Z_STEPS_PER_MM 200.0
 #define DEFAULT_A_STEPS_PER_MM 200.0

 //max speed
 #    define DEFAULT_X_MAX_RATE 2500.0 
 #    define DEFAULT_Y_MAX_RATE 2500.0 
 #    define DEFAULT_Z_MAX_RATE 2500.0 
 
 #    define DEFAULT_X_ACCELERATION 100.0
 #    define DEFAULT_Y_ACCELERATION 100.0
 #    define DEFAULT_Z_ACCELERATION 100.0
 
 //default motor run current
 #    define DEFAULT_X_CURRENT 1.8
 #    define DEFAULT_Y_CURRENT 1.8
 #    define DEFAULT_Z_CURRENT 1.8
 #    define DEFAULT_A_CURRENT 1.8
 
 //default motor hold current
 #    define DEFAULT_X_HOLD_CURRENT 1.25 
 #    define DEFAULT_Y_HOLD_CURRENT 1.25 
 #    define DEFAULT_Z_HOLD_CURRENT 1.25 
 #    define DEFAULT_A_HOLD_CURRENT 1.25 
 
 //micro steps
 #    define DEFAULT_X_MICROSTEPS 8
 #    define DEFAULT_Y_MICROSTEPS 8
 #    define DEFAULT_Z_MICROSTEPS 8
 
 //homing cycles (z then xy combined)
 #    define DEFAULT_HOMING_CYCLE_0 bit(Z_AXIS)
 #    define DEFAULT_HOMING_CYCLE_1 (bit(X_AXIS) | bit(Y_AXIS))
 #    define DEFAULT_HOMING_PULLOFF 2.5
 
 //Stepper settings
 #    define DEFAULT_STEP_PULSE_MICROSECONDS 4
 #    define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 
 
 //switchs and probes
 #    define DEFAULT_INVERT_PROBE_PIN 1
 #    define DEFAULT_INVERT_LIMIT_PINS 1
 
 // Macro3 | Macro2 | Macro 1| Macr0 |Cycle Start | Feed Hold | Reset | Safety Door
 // For example B1101 will invert the function of the Reset pin.
 #ifdef INVERT_CONTROL_PIN_MASK
	#undef INVERT_CONTROL_PIN_MASK
 #endif
 // NC Door = B00001110, NO Door = B00001111
 #	  define INVERT_CONTROL_PIN_MASK B00001111
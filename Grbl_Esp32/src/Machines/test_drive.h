#pragma once
// clang-format off

/*
    test_drive.h
    Part of Grbl_ESP32

    Pin assignments (or lack thereof) for testing Grbl_ESP32.

    It creates a basic 3 axis machine without actually driving
    I/O pins.  Grbl will report that axes are moving, but no physical
    motor motion will occur.

    This can be uploaded to an unattached ESP32 or attached to
    unknown hardware with no risk of pins trying to output signals
    into a short, etc that could dmamge the ESP32

    It can also be used to get the basic program running so OTA
    (over the air) firmware loading can be done.

    2018    - Bart Dring
    2020    - Mitch Bradley

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

#define MACHINE_NAME "Test Drive - Demo Only No I/O!"


#define N_AXIS 3
// This cannot use homing because there are no switches
#ifdef DEFAULT_HOMING_CYCLE_0
    #undef DEFAULT_HOMING_CYCLE_0
#endif

#ifdef DEFAULT_HOMING_CYCLE_1
    #undef DEFAULT_HOMING_CYCLE_1
#endif


#ifdef USE_RMT_STEPS
    #undef USE_RMT_STEPS  // Suppress unused variable warning
#endif


// Output devices:
// - I2S

// #define USE_I2S_OUT
// #define USE_I2S_STEPS
// #define I2S_OUT_BCK             GPIO_NUM_22
// #define I2S_OUT_WS              GPIO_NUM_17
// #define I2S_OUT_DATA            GPIO_NUM_21
// 
// #define COOLANT_MIST_PIN        "i2s.24"

// x CoolantControl

// #define COOLANT_MIST_PIN        "gpio.2"
// #define COOLANT_FLOOD_PIN       "gpio.4"

// x StandardStepper

// #define X_STEP_PIN              "gpio.2"
// #define X_DIRECTION_PIN         "gpio.4"
// #define X_DISABLE_PIN           "gpio.16"

// x Unipolar

// #define X_UNIPOLAR
// #define X_PIN_PHASE_0           "gpio.2"
// #define X_PIN_PHASE_1           "gpio.4"
// #define X_PIN_PHASE_2           "gpio.16"
// #define X_PIN_PHASE_3           "gpio.15"

// Spindles:

#define SPINDLE_TYPE            SpindleType::NONE

// #define SPINDLE_OUTPUT_PIN	    "gpio.15"
// #define SPINDLE_ENABLE_PIN	    "gpio.2"
// #define SPINDLE_DIRECTION_PIN	"gpio.4"
// #define SPINDLE_FORWARD_PIN	    "gpio.16"
// #define SPINDLE_REVERSE_PIN	    "gpio.17"

// x 10V
// #define SPINDLE_TYPE            SpindleType::_10V

// x PWM
// #define SPINDLE_TYPE            SpindleType::PWM

// x Relay
// #define SPINDLE_TYPE            SpindleType::RELAY

// - BESC
// - DAC
// 
// x User Digital pins
// #define USER_DIGITAL_PIN_0	    "gpio.15"
// #define USER_DIGITAL_PIN_1	    "gpio.2"
// #define USER_DIGITAL_PIN_2  	"gpio.4"
// #define USER_DIGITAL_PIN_3      "gpio.16"

// x User Analog pins
// #define USER_ANALOG_PIN_0	    "gpio.15"
// #define USER_ANALOG_PIN_1	    "gpio.2"
// #define USER_ANALOG_PIN_2  	    "gpio.4"
// #define USER_ANALOG_PIN_3       "gpio.16"

// 
// Input devices :
// x Probe
// #define PROBE_PIN               "gpio.18:pu"

// x Limits
// #define X_LIMIT_PIN               "gpio.18:pu"
// x System : ControlSafetyDoor
// #define CONTROL_SAFETY_DOOR_PIN   "gpio.22:pu"
// x System : ControlReset
// #define CONTROL_RESET_PIN         "gpio.23:pu"
// x System : ControlFeedHold
// #define CONTROL_FEED_HOLD_PIN        "gpio.22:pu"
// x System : ControlCycleStart
// #define CONTROL_CYCLE_START_PIN      "gpio.18:pu"
// - System : Macro0
// - System : Macro1
// - System : Macro2
// - System : Macro3
// 
// Uart devices :
// - Dynamixel
// - H2ASpindle
// - HuanyangSpindle
// 
// Other :
// - RcServo
// - SDCard
// - Trinamic


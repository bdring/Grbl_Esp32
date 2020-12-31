#pragma once
// clang-format off

/*
    6_pack_stepstick_XYZ_v1.h

    Covers all V1 versions V1p0, V1p1, etc

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board
    2018    - Bart Dring
    2020    - Mitch Bradley
    2020    - Michiyasu Odaki
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
#define MACHINE_NAME            "6 Pack Controller StepStick XYZ"

#define N_AXIS 3

// Note: MS3 can be used to roughly switch microstepping
// DRV8825 low=1/8 high=1/32
// A4988 low=1/8 high=1/16

// Motor Socket #1
#define X_MOTOR_TYPE            MotorType::StepStick
#define X_DISABLE_PIN           "i2so.0"
#define X_DIRECTION_PIN         "i2so.1"
#define X_STEP_PIN              "i2so.2"
#define X_STEPPER_MS3           "i2so.3:high"

// Motor Socket #2
#define Y_MOTOR_TYPE            MotorType::StepStick
#define Y_DIRECTION_PIN         "i2so.4"
#define Y_STEP_PIN              "i2so.5"
#define Y_DISABLE_PIN           "i2so.7"
#define Y_STEPPER_MS3           "i2so.6:high"

// Motor Socket #3
#define Z_MOTOR_TYPE            MotorType::StepStick
#define Z_DISABLE_PIN           "i2so.8"
#define Z_DIRECTION_PIN         "i2so.9"
#define Z_STEP_PIN              "i2so.10"
#define Z_STEPPER_MS3           "i2so.11:high"

/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    Click on each module to get example for using the modules in the sockets


Socket #1
#1 "gpio.33" 
#2 "gpio.32"
#3 "gpio.35" (input only)
#4 "gpio.34" (input only)

Socket #2
#1 "gpio.2"
#2 "gpio.25"
#3 "gpio.39" (input only)
#4 "gpio.36" (input only)

Socket #3
#1 "gpio.26"
#2 "gpio.4"
#3 "gpio.16"
#4 "gpio.27"

Socket #4
#1 "gpio.14"
#2 "gpio.13"
#3 "gpio.15"
#4 "gpio.12"

Socket #5
#1 "i2so.24"  (output only)
#2 "i2so.25"  (output only)
#3 "i2so.26"  (output only)

*/


// 4x Input Module in Socket #1
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define X_LIMIT_PIN                 "gpio.33"
#define Y_LIMIT_PIN                 "gpio.32"
#define Z_LIMIT_PIN                 "gpio.35"




// // 4x Input Module in Socket #2
// // https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
// #define X_LIMIT_PIN                 "gpio.2"
// #define Y_LIMIT_PIN                 "gpio.25"
// #define Z_LIMIT_PIN                 "gpio.39"
// #define MACRO_BUTTON_0_PIN            "gpio.2"
// #define MACRO_BUTTON_1_PIN            "gpio.25"
// #define MACRO_BUTTON_2_PIN            "gpio.39"
// #define MACRO_BUTTON_3_PIN            "gpio.36"

// 5V output CNC module in socket #4
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-5V-Buffered-Output-Module
// #define SPINDLE_TYPE                SpindleType::PWM
// #define SPINDLE_OUTPUT_PIN          "gpio.14"
// #define SPINDLE_ENABLE_PIN          "gpio.13" // optional
// #define LASER_OUTPUT_PIN            "gpio.15" // optional
// #define LASER_ENABLE_PIN            "gpio.12"




// // RS485 Modbus In socket #3
// // https://github.com/bdring/6-Pack_CNC_Controller/wiki/RS485-Modbus-Module
// #define VFD_RS485_TXD_PIN        GPIO_NUM_26
// #define VFD_RS485_RTS_PIN        GPIO_NUM_4
// #define VFD_RS485_RXD_PIN        GPIO_NUM_16

// // 4x Input Module in Socket #3
// // https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
// #define CONTROL_CYCLE_START_PIN      "gpio.26"
// #define CONTROL_FEED_HOLD_PIN        "gpio.4"
// #define CONTROL_RESET_PIN            "gpio.16"
// #define CONTROL_SAFETY_DOOR_PIN      "gpio.27"

// ================= Setting Defaults ==========================
// https://github.com/bdring/Grbl_Esp32/wiki/Setting-Defaults
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define DEFAULT_X_STEPS_PER_MM      800
#define DEFAULT_Y_STEPS_PER_MM      800
#define DEFAULT_Z_STEPS_PER_MM      800

#define DEFAULT_HOMING_DIR_MASK     bit(X_AXIS)


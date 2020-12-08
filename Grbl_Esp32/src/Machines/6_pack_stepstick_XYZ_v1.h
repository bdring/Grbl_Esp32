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

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC
// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

#define I2S_OUT_BCK             GPIO_NUM_22
#define I2S_OUT_WS              GPIO_NUM_17
#define I2S_OUT_DATA            GPIO_NUM_21


// Motor Socket #1
#define X_DISABLE_PIN           I2SO(0)
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)
#define X_STEPPER_MS3           I2SO(3)

// Motor Socket #2
#define Y_DIRECTION_PIN         I2SO(4)
#define Y_STEP_PIN              I2SO(5)
#define Y_STEPPER_MS3           I2SO(6)
#define Y_DISABLE_PIN           I2SO(7)

// Motor Socket #3
#define Z_DISABLE_PIN           I2SO(8)
#define Z_DIRECTION_PIN         I2SO(9)
#define Z_STEP_PIN              I2SO(10)
#define Z_STEPPER_MS3           I2SO(11) 

/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    Click on each module to get example for using the modules in the sockets


Socket #1
#1 GPIO_NUM_33 
#2 GPIO_NUM_32
#3 GPIO_NUM_35 (input only)
#4 GPIO_NUM_34 (input only)

Socket #2
#1 GPIO_NUM_2
#2 GPIO_NUM_25
#3 GPIO_NUM_39 (input only)
#4 GPIO_NUM_36 (input only)

Socket #3
#1 GPIO_NUM_26
#2 GPIO_NUM_4
#3 GPIO_NUM_16
#4 GPIO_NUM_27

Socket #4
#1 GPIO_NUM_14
#2 GPIO_NUM_13
#3 GPIO_NUM_15
#4 GPIO_NUM_12

Socket #5
#1 I2SO(24)  (output only)
#2 I2SO(25)  (output only)
#3 I2SO26)  (output only)

*/


// 4x Input Module in Socket #1
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define X_LIMIT_PIN                 GPIO_NUM_33
#define Y_LIMIT_PIN                 GPIO_NUM_32
#define Z_LIMIT_PIN                 GPIO_NUM_35




// // 4x Input Module in Socket #2
// // https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define MACRO_BUTTON_0_PIN            GPIO_NUM_2
#define MACRO_BUTTON_1_PIN            GPIO_NUM_25
#define MACRO_BUTTON_2_PIN            GPIO_NUM_39
#define MACRO_BUTTON_3_PIN            GPIO_NUM_36

// 5V output CNC module in socket #4
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-5V-Buffered-Output-Module
#define SPINDLE_TYPE                SpindleType::PWM
#define SPINDLE_OUTPUT_PIN          GPIO_NUM_14
#define SPINDLE_ENABLE_PIN          GPIO_NUM_13 // optional
#define LASER_OUTPUT_PIN            GPIO_NUM_15 // optional
#define LASER_ENABLE_PIN            GPIO_NUM_12




// // RS485 Modbus In socket #3
// // https://github.com/bdring/6-Pack_CNC_Controller/wiki/RS485-Modbus-Module
// #define VFD_RS485_TXD_PIN        GPIO_NUM_26
// #define VFD_RS485_RTS_PIN        GPIO_NUM_4
// #define VFD_RS485_RXD_PIN        GPIO_NUM_16

// Example (4x) 5V Buffer Output on socket #5
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-5V-Buffered-Output-Module
#define USER_DIGITAL_PIN_0      I2SO(24) // No PWM
#define USER_DIGITAL_PIN_1      I2SO(25)
#define USER_DIGITAL_PIN_2      I2SO(26) //  M7 on M9 Off
#define USER_DIGITAL_PIN_3      I2SO(27) //  M8 on M9 Off

// ================= Setting Defaults ==========================
#define DEFAULT_X_STEPS_PER_MM      800
#define DEFAULT_Y_STEPS_PER_MM      800
#define DEFAULT_Z_STEPS_PER_MM      800

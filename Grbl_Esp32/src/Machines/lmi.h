#pragma once
// clang-format off
/*
    6_pack_SE2050_laser.h
    Covers all V1 versions V1p0, V1p1, etc
    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board
    2020-11-21 B. Dring for Taylor F.
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
#define MACHINE_NAME            "X820xY316xZ246 50w RF Laser"
#define N_AXIS 3  // change to 4 to include A axis
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
#if (N_AXIS == 4)
    // Motor Socket #4
    #define A_DIRECTION_PIN         I2SO(12)
    #define A_STEP_PIN              I2SO(13)
    #define A_STEPPER_MS3           I2SO(14) 
    #define A_DISABLE_PIN           I2SO(15)
#endif
/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    Click on each module to get example for using the modules in the sockets
*/
// 4x Input Module in Socket #1
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define X_LIMIT_PIN                 GPIO_NUM_33
#define Y_LIMIT_PIN                 GPIO_NUM_32
#define Z_LIMIT_PIN                 GPIO_NUM_35
// #define CONTROL_SAFETY_DOOR_PIN     GPIO_NUM_34

#define DEFAULT_INVERT_LIMIT_PINS       1  // Sets the default for N.O. switches
// 4x Input in socket #2 for future custom buttons.
// 4x Switch Input module  in socket #2
/*
#define  MACRO_BUTTON_0_PIN     GPIO_NUM_2
#define  MACRO_BUTTON_1_PIN     GPIO_NUM_25
#define  MACRO_BUTTON_2_PIN     GPIO_NUM_39
#define  MACRO_BUTTON_3_PIN     GPIO_NUM_36
*/
// Example 5V output CNC module in socket #3
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-5V-Buffered-Output-Module
#define SPINDLE_TYPE            SpindleType::LASER
// #define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_PWM_BASE_FREQ 20000
#define SPINDLE_PWM_BIT_PRECISION 12
#define SPINDLE_PWM_MAX_VALUE 4096

#define SPINDLE_OUTPUT_PIN      GPIO_NUM_16  // 1st channel
#define SPINDLE_ENABLE_PIN      GPIO_NUM_4   // 2nd channel Enable Pin
#define COOLANT_MIST_PIN        GPIO_NUM_27  // 3nd channel Air Assist using M7 on M9 off
// Socket #4
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/Quad-MOSFET-Module
#define USER_DIGITAL_PIN_0       GPIO_NUM_14 // Laser shutter using M62P0 on M63P0 Off
#define USER_DIGITAL_PIN_1       GPIO_NUM_13 // Aim Guide control using M62P1 on M63P1 Off

#define DEFAULT_HOMING_ENABLE 1
#define DEFAULT_SOFT_LIMIT_ENABLE 1
#define DEFAULT_HOMING_DIR_MASK 7
#define DEFAULT_HOMING_FEED_RATE 100.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE 200.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF 0.75 // mm

#define DEFAULT_X_STEPS_PER_MM 80
#define DEFAULT_Y_STEPS_PER_MM 314.96
#define DEFAULT_Z_STEPS_PER_MM 314.96

#define DEFAULT_X_MAX_RATE 1000
#define DEFAULT_Y_MAX_RATE 600 // mm/min
#define DEFAULT_Z_MAX_RATE 200 // mm/min

#define DEFAULT_X_ACCELERATION 200.0 // mm/sec^2
#define DEFAULT_Y_ACCELERATION 175.0 // mm/sec^2
#define DEFAULT_Z_ACCELERATION 100.0 // mm/sec^2

#define DEFAULT_X_MAX_TRAVEL 820 // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 316 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 246 // mm NOTE: Must be a positive value.


//x steps 80
//y steps 314.96
//z steps 314.96
// laser mode
//accel 200
//100
//100
//distance820
//316
//246*

//homing cycle
// soft limits
// homing seek
// homing feed
//homing mask 7
//max spd 1500
//600
//100

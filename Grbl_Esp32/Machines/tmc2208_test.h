/*
    tmc2208_test.h
    Part of Grbl_ESP32

    Pin assignments for the TMC2208 

    2020    - The Ant Team

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


// Select a version to match your PCB

#define MACHINE_NAME    "ESP32_TMC2208_TEST SINGLE MOTOR"

#define USE_TRINAMIC // Using at least 1 trinamic driver

#define X_STEP_PIN              GPIO_NUM_18
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC              // using SPI control
#define X_TRINAMIC_DRIVER       2208        // Which Driver Type?
#define X_RSENSE                TMC2130_RSENSE_DEFAULT //TODO: change this!

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN GPIO_NUM_32

// Define one of these 2 options for spindle or servo
//#define USE_SERVO_AXES
#define USE_SPINDLE

#ifdef USE_SERVO_AXES
    #define SPINDLE_TYPE            SPINDLE_TYPE_NONE

    #define SERVO_Z_PIN                     GPIO_NUM_27 // comment this out if PWM spindle/laser control.
    #define SERVO_Z_RANGE_MIN               0.0
    #define SERVO_Z_RANGE_MAX               5.0
    #define SERVO_Z_HOMING_TYPE             SERVO_HOMING_TARGET // during homing it will instantly move to a target value
    #define SERVO_Z_HOME_POS                SERVO_Z_RANGE_MAX // move to max during homing
    #define SERVO_Z_MPOS                    false           // will not use mpos, uses work coordinates
#else

    #define SPINDLE_TYPE        SPINDLE_TYPE_PWM
    #define SPINDLE_OUTPUT_PIN     GPIO_NUM_27
#endif

// #define X_LIMIT_PIN 
#define X_LIMIT_PIN             GPIO_NUM_39
#define LIMIT_MASK              B1 //TODO: to be checked

// defaults
#define DEFAULT_Z_STEPS_PER_MM 100.0    // This is used as the servo calibration
#define DEFAULT_Z_MAX_TRAVEL 300.0      // This is used as the servo calibration

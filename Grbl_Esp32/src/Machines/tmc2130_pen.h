#pragma once
// clang-format off

/*
    tmc2130_pen.h
    Part of Grbl_ESP32

    Pin assignments for the TMC2130 Pen/Laser controller
    https://github.com/bdring/Grbl_ESP32_TMC2130_Plotter_Controller

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


// Select a version to match your PCB
//#define MACHINE_V1 // version 1 PCB
#define MACHINE_V2 // version 2 PCB

#ifdef MACHINE_V1
    #define MACHINE_NAME    "ESP32_TMC2130_PEN V1"
    #define X_LIMIT_PIN     GPIO_NUM_2
#else
    #define MACHINE_NAME    "ESP32_TMC2130_PEN V2"
    #define X_LIMIT_PIN     GPIO_NUM_32
#endif

#define TRINAMIC_RUN_MODE           TrinamicMode :: CoolStep
#define TRINAMIC_HOMING_MODE        TrinamicMode :: CoolStep

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC_DRIVER       2130        // Which Driver Type?
#define X_CS_PIN                GPIO_NUM_17  //chip select
#define X_RSENSE                TMC2130_RSENSE_DEFAULT

#define Y_STEP_PIN              GPIO_NUM_14
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Y_TRINAMIC_DRIVER       2130        // Which Driver Type?
#define Y_CS_PIN                GPIO_NUM_16  //chip select
#define Y_RSENSE                TMC2130_RSENSE_DEFAULT

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN GPIO_NUM_13


// Define one of these 2 options for spindle or servo
#define Z_SERVO_PIN                     GPIO_NUM_27 // comment this out if PWM spindle/laser control.
#define DEFAULT_Z_MAX_TRAVEL          5.0   // Range of travel is 5mm
#define DEFAULT_Z_HOMING_MPOS         5.0   // MPos will be set to 5mm after homing
#define Z_SERVO_CAL_MIN               1.0   // calibration factor for the minimum PWM duty
#define Z_SERVO_CAL_MAX               1.0   // calibration factor for the maximum PWM duty

// #define X_LIMIT_PIN          See version section at beginning of file
#define Y_LIMIT_PIN             GPIO_NUM_4

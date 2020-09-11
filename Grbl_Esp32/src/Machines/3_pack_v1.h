#pragma once
// clang-format off

/*
    3_pack_v1.h

    Covers all V1 versions V1p0, V1p1, etc

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board

    2020    - Bart Dring

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
#define MACHINE_NAME            "3 Pack Controller V1"


#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

#define X_STEPPER_MS3           GPIO_NUM_4  // X_CS (works for all drivers on this board)

#define X_STEP_PIN              GPIO_NUM_32
#define X_DIRECTION_PIN         GPIO_NUM_27

#define Y_STEP_PIN              GPIO_NUM_33
#define Y_DIRECTION_PIN         GPIO_NUM_26

#define Z_STEP_PIN              GPIO_NUM_25
#define Z_DIRECTION_PIN         GPIO_NUM_17

#define STEPPERS_DISABLE_PIN    GPIO_NUM_16

/*  Socket #1 ref
#1  GPIO_NUM_36     // Input Only (X_LIMIT_PIN)
#2  GPIO_NUM_39     // Input Only (Y_LIMIT_PIN)
#4  GPIO_NUM_34     // Input Only (Z_LIMIT_PIN)
#4  GPIO_NUM_35     // Input Only
*/

/*  Socket #2 ref
#1  GPIO_NUM_2
#2  GPIO_NUM_21
#4  GPIO_NUM_22
#4  N/C
*/

/*  Socket #3 ref
#1  GPIO_NUM_14
#2  GPIO_NUM_13
#4  GPIO_NUM_15
#4  GPIO_NUM_12
*/

// Socket #1
#define X_LIMIT_PIN     GPIO_NUM_36
#define Y_LIMIT_PIN     GPIO_NUM_39 
#define Z_LIMIT_PIN     GPIO_NUM_34
#define PROBE_PIN       GPIO_NUM_35

/*
// Socket #3 5V Output
#define SPINDLE_TYPE            SpindleType::LASER
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_14   // labeled SpinPWM
#define SPINDLE_ENABLE_PIN      GPIO_NUM_13  // labeled SpinEnbl
#define COOLANT_MIST_PIN        GPIO_NUM_15
#define COOLANT_FLOOD_PIN       GPIO_NUM_12
*/

// Socket #3 5V Output
#define USER_DIGITAL_PIN_0        GPIO_NUM_14   
#define USER_DIGITAL_PIN_1        GPIO_NUM_13  
#define USER_DIGITAL_PIN_2        GPIO_NUM_15
#define USER_DIGITAL_PIN_3        GPIO_NUM_12

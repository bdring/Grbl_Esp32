#pragma once
// clang-format off

/*
    Yboard.h
    Part of Grbl_ESP32

    Pin assignments for the ESP32 CNC shield Controller, v4.5 and later.
    https://github.com/CN-CNC/Yboard
    https://diystudio.taobao.com/

    2018    - Bart Dring
    2022    - Miller Ren

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

#define MACHINE_NAME            "YFSTUDIO Yboard v4"

#define N_AXIS 4

#define STEPPERS_DISABLE_PIN    GPIO_NUM_15

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_14
#define Y_STEP_PIN              GPIO_NUM_27
#define Y_DIRECTION_PIN         GPIO_NUM_26
#define Z_STEP_PIN              GPIO_NUM_25
#define Z_DIRECTION_PIN         GPIO_NUM_33
#define A_STEP_PIN              GPIO_NUM_16
#define A_DIRECTION_PIN         GPIO_NUM_17


#define X_LIMIT_PIN             GPIO_NUM_34
#define Y_LIMIT_PIN             GPIO_NUM_35
#define Z_LIMIT_PIN             GPIO_NUM_32
#define PROBE_PIN               GPIO_NUM_13

#define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_4   

// #define SPINDLE_TYPE            SpindleType::LASER
#define LASER_OUTPUT_PIN        GPIO_NUM_4   

#define COOLANT_MIST_PIN        GPIO_NUM_2
#define COOLANT_FLOOD_PIN        GPIO_NUM_2

#define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_36

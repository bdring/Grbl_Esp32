#pragma once
// clang-format off

/*
    makerfr_v2_XYZA.h
    Part of Grbl_ESP32

    Pin assignments for the RS-CNC32 board (4 axis with external drivers)
    https://github.com/bdring/4_Axis_SPI_CNC

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

#define MACHINE_NAME            "MakerFr GRBL 32 bits Board V2 XYZA"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 4

#define X_STEP_PIN              GPIO_NUM_33
#define X_DIRECTION_PIN         GPIO_NUM_25
#define Y_STEP_PIN              GPIO_NUM_26
#define Y_DIRECTION_PIN         GPIO_NUM_27
#define Z_STEP_PIN              GPIO_NUM_14
#define Z_DIRECTION_PIN         GPIO_NUM_12
#define A_STEP_PIN              GPIO_NUM_13
#define A_DIRECTION_PIN         GPIO_NUM_32
#define STEPPERS_DISABLE_PIN    GPIO_NUM_15

#define SPINDLE_OUTPUT_PIN      GPIO_NUM_2   // labeled SpinPWM
#define SPINDLE_ENABLE_PIN      GPIO_NUM_17  // labeled SpinEnbl
#define LASER_OUTPUT_PIN        GPIO_NUM_2

#define X_LIMIT_PIN             GPIO_NUM_36
#define Y_LIMIT_PIN             GPIO_NUM_39
#define Z_LIMIT_PIN             GPIO_NUM_34
#define A_LIMIT_PIN             GPIO_NUM_35

#define PROBE_PIN               GPIO_NUM_16
#define COOLANT_MIST_PIN        GPIO_NUM_22

#ifdef INVERT_CONTROL_PIN_MASK
    #undef INVERT_CONTROL_PIN_MASK
#endif

#define INVERT_CONTROL_PIN_MASK B1110

#define CONTROL_RESET_PIN       GPIO_NUM_4  //4 labeled Reset
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_21  //21 labeled Hold
#define CONTROL_CYCLE_START_PIN  GPIO_NUM_0  //0 labeled Start


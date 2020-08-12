#pragma once
// clang-format off

/*
    6_pack_MPCNC_stepstick_v1.h

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
#define MACHINE_NAME            "6 Pack MPCNC XYZXY V1 (StepStick)"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 3

#ifdef ENABLE_SD_CARD
    #undef ENABLE_SD_CARD
#endif

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
// Define USE_I2S_OUT_STREAM if buffering is used.
// (there will be a delay between the specified I/O operation and the actual I/O execution)
#define USE_I2S_OUT_STREAM
#undef USE_RMT_STEPS

#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

#define I2S_OUT_BCK      GPIO_NUM_22
#define I2S_OUT_WS       GPIO_NUM_17
#define I2S_OUT_DATA     GPIO_NUM_21


#define X_STEPPER_MS3           I2SO(3)   // Labeled X_CS
#define Y_STEPPER_MS3           I2SO(6)   // Y_CS
#define Z_STEPPER_MS3           I2SO(11)  // Z_CS
#define X2_STEPPER_MS3          I2SO(14)  // A_CS
#define Y2_STEPPER_MS3          I2SO(19)  // B_CS

#define STEPPER_RESET           GPIO_NUM_19

#define X_DISABLE_PIN           I2SO(0)
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)
#define X_AXIS_SQUARING

#define Y_DIRECTION_PIN         I2SO(4)
#define Y_STEP_PIN              I2SO(5)
#define Y_DISABLE_PIN           I2SO(7)
#define Y_AXIS_SQUARING

#define Z_DISABLE_PIN           I2SO(8)
#define Z_DIRECTION_PIN         I2SO(9)
#define Z_STEP_PIN              I2SO(10)

// labeled A on controller
#define X2_DIRECTION_PIN         I2SO(12)
#define X2_STEP_PIN              I2SO(13)
#define X2_DISABLE_PIN           I2SO(15)

// labeled B on controller
#define Y2_DISABLE_PIN           I2SO(16)
#define Y2_DIRECTION_PIN         I2SO(17)
#define Y2_STEP_PIN              I2SO(18)

// stepper C unused

#define X_LIMIT_PIN             GPIO_NUM_33
#define Y_LIMIT_PIN             GPIO_NUM_32
#define Z_LIMIT_PIN             GPIO_NUM_35
#define X2_LIMIT_PIN            GPIO_NUM_34 // labeled A
#define Y2_LIMIT_PIN            GPIO_NUM_39 // labeled B



// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

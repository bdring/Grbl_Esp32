#pragma once
// clang-format off

/*
    i2s_out_xxyyzz.h
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

#define MACHINE_NAME            "ESP32 I2S XXYYZZ Axis Driver Board (StepStick)"

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
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC

#define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

#define I2S_OUT_BCK             GPIO_NUM_22
#define I2S_OUT_WS              GPIO_NUM_17
#define I2S_OUT_DATA            GPIO_NUM_21


#define STEPPER_MS1             GPIO_NUM_23 // MOSI
#define STEPPER_MS2             GPIO_NUM_18 // SCK

#define STEPPER_X_MS3           I2SO(3)   // X_CS
#define STEPPER_Y_MS3           I2SO(6)   // Y_CS
#define STEPPER_Z_MS3           I2SO(11)  // Z_CS
#define STEPPER_A_MS3           I2SO(14)  // A_CS
#define STEPPER_B_MS3           I2SO(19)  // B_CS
#define STEPPER_C_MS3           I2SO(22)  // C_CS

#define STEPPER_RESET           GPIO_NUM_19

#define X_DISABLE_PIN           I2SO(0)
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)

#define X2_DIRECTION_PIN        I2SO(4)
#define X2_STEP_PIN             I2SO(5)
#define X2_DISABLE_PIN          I2SO(7)

#define Y_DISABLE_PIN           I2SO(8)
#define Y_DIRECTION_PIN         I2SO(9)
#define Y_STEP_PIN              I2SO(10)

#define Y2_DIRECTION_PIN        I2SO(12)
#define Y2_STEP_PIN             I2SO(13)
#define Y2_DISABLE_PIN          I2SO(15)

#define Z_DISABLE_PIN           I2SO(16)
#define Z_DIRECTION_PIN         I2SO(17)
#define Z_STEP_PIN              I2SO(18)

#define Z2_DIRECTION_PIN        I2SO(20)
#define Z2_STEP_PIN             I2SO(21)
#define Z2_DISABLE_PIN          I2SO(23)


#define SPINDLE_TYPE            SpindleType::PWM // only one spindle at a time
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_26
#define SPINDLE_ENABLE_PIN      GPIO_NUM_4
#define SPINDLE_DIR_PIN         GPIO_NUM_16

#define X_LIMIT_PIN             GPIO_NUM_36
#define Y_LIMIT_PIN             GPIO_NUM_39
#define Z_LIMIT_PIN             GPIO_NUM_34
//#define A_LIMIT_PIN             GPIO_NUM_35
//#define B_LIMIT_PIN             GPIO_NUM_32
//#define C_LIMIT_PIN             GPIO_NUM_33

#define PROBE_PIN               GPIO_NUM_25

#define COOLANT_MIST_PIN        GPIO_NUM_2

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

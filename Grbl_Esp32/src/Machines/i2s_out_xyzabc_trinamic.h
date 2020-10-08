#pragma once
// clang-format off

/*
    i2s_out_xyzabc_trinamic.h
    Part of Grbl_ESP32
    Pin assignments for the ESP32 SPI 6-axis board
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
#define MACHINE_NAME            "ESP32 SPI 6 Axis Driver Board Trinamic"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 6

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC

#define I2S_OUT_BCK             GPIO_NUM_22
#define I2S_OUT_WS              GPIO_NUM_17
#define I2S_OUT_DATA            GPIO_NUM_21

#define TRINAMIC_RUN_MODE           TrinamicMode :: CoolStep
#define TRINAMIC_HOMING_MODE        TrinamicMode :: CoolStep

#define X_TRINAMIC_DRIVER       2130
#define X_DISABLE_PIN           I2SO(0)
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)
#define X_CS_PIN                I2SO(3)
#define X_RSENSE                TMC2130_RSENSE_DEFAULT

#define Y_TRINAMIC_DRIVER       2130
#define Y_DIRECTION_PIN         I2SO(4)
#define Y_STEP_PIN              I2SO(5)
#define Y_DISABLE_PIN           I2SO(7)
#define Y_CS_PIN                I2SO(6)
#define Y_RSENSE                X_RSENSE

#define Z_TRINAMIC_DRIVER       2130
#define Z_DISABLE_PIN           I2SO(8)
#define Z_DIRECTION_PIN         I2SO(9)
#define Z_STEP_PIN              I2SO(10)
#define Z_CS_PIN                I2SO(11)
#define Z_RSENSE                X_RSENSE

#define A_TRINAMIC_DRIVER       2130
#define A_DIRECTION_PIN         I2SO(12)
#define A_STEP_PIN              I2SO(13)
#define A_DISABLE_PIN           I2SO(15)
#define A_CS_PIN                I2SO(14)
#define A_RSENSE                X_RSENSE

#define B_TRINAMIC_DRIVER       2130
#define B_DISABLE_PIN           I2SO(16)
#define B_DIRECTION_PIN         I2SO(17)
#define B_STEP_PIN              I2SO(18)
#define B_CS_PIN                I2SO(19)
#define B_RSENSE                X_RSENSE

#define C_TRINAMIC_DRIVER       2130
#define C_DIRECTION_PIN         I2SO(20)
#define C_STEP_PIN              I2SO(21)
#define C_DISABLE_PIN           I2SO(23)
#define C_CS_PIN                I2SO(22)
#define C_RSENSE                X_RSENSE

/*
#define SPINDLE_TYPE            SpindleType::PWM // only one spindle at a time
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_26
#define SPINDLE_ENABLE_PIN      GPIO_NUM_4
#define SPINDLE_DIR_PIN         GPIO_NUM_16
*/
#define X_LIMIT_PIN             GPIO_NUM_33
#define Y_LIMIT_PIN             GPIO_NUM_32
#define Z_LIMIT_PIN             GPIO_NUM_35
#define A_LIMIT_PIN             GPIO_NUM_34
#define B_LIMIT_PIN             GPIO_NUM_39
#define C_LIMIT_PIN             GPIO_NUM_36

#define SPINDLE_TYPE SpindleType::RELAY
#define SPINDLE_OUTPUT_PIN GPIO_NUM_26

#define PROBE_PIN               GPIO_NUM_25

#define COOLANT_MIST_PIN        GPIO_NUM_2

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

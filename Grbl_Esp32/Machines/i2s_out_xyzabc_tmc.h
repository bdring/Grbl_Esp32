/*
    i2s_out_xyzabc_tmc.h
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
#define MACHINE_NAME            "ESP32 SPI 6 Axis Driver Board (Trinamic)"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 6

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_IOEXPANDER
// Define I2S_STEPPER_STREAM if buffering is used.
// (there will be a delay between the specified I/O operation and the actual I/O execution)
#define I2S_STEPPER_STREAM
#undef USE_RMT_STEPS

#define I2S_IOEXPANDER_BCK      GPIO_NUM_22
#define I2S_IOEXPANDER_WS       GPIO_NUM_17
#define I2S_IOEXPANDER_DATA     GPIO_NUM_21

#define USE_TRINAMIC // Using at least 1 trinamic driver

#define X_DISABLE_PIN            GPIO_NUM_I2S_OUT_0
#define X_DIRECTION_PIN         GPIO_NUM_I2S_OUT_1
#define X_STEP_PIN              GPIO_NUM_I2S_OUT_2
#define X_TRINAMIC              // using SPI control
#define X_DRIVER_TMC2130        // Which Driver Type?
#define X_CS_PIN                GPIO_NUM_I2S_OUT_3
#define X_RSENSE                TMC2130_RSENSE_DEFAULT

#define Y_DIRECTION_PIN         GPIO_NUM_I2S_OUT_4
#define Y_STEP_PIN              GPIO_NUM_I2S_OUT_5
#define Y_DISABLE_PIN            GPIO_NUM_I2S_OUT_7
#define Y_TRINAMIC              // using SPI control
#define Y_DRIVER_TMC2130        // Which Driver Type?
#define Y_CS_PIN                GPIO_NUM_I2S_OUT_6
#define Y_RSENSE                X_RSENSE

#define Z_DISABLE_PIN            GPIO_NUM_I2S_OUT_8
#define Z_DIRECTION_PIN         GPIO_NUM_I2S_OUT_9
#define Z_STEP_PIN              GPIO_NUM_I2S_OUT_10
#define Z_TRINAMIC              // using SPI control
#define Z_DRIVER_TMC2130        // Which Driver Type?
#define Z_CS_PIN                GPIO_NUM_I2S_OUT_11
#define Z_RSENSE                X_RSENSE

#define A_DIRECTION_PIN         GPIO_NUM_I2S_OUT_12
#define A_STEP_PIN              GPIO_NUM_I2S_OUT_13
#define A_DISABLE_PIN            GPIO_NUM_I2S_OUT_15
#define A_TRINAMIC              // using SPI control
#define A_DRIVER_TMC2130        // Which Driver Type?
#define A_CS_PIN                GPIO_NUM_I2S_OUT_14
#define A_RSENSE                X_RSENSE

#define B_DISABLE_PIN            GPIO_NUM_I2S_OUT_16
#define B_DIRECTION_PIN         GPIO_NUM_I2S_OUT_17
#define B_STEP_PIN              GPIO_NUM_I2S_OUT_18
#define B_TRINAMIC              // using SPI control
#define B_DRIVER_TMC2130        // Which Driver Type?
#define B_CS_PIN                GPIO_NUM_I2S_OUT_19
#define B_RSENSE                X_RSENSE

#define C_DIRECTION_PIN         GPIO_NUM_I2S_OUT_20
#define C_STEP_PIN              GPIO_NUM_I2S_OUT_21
#define C_DISABLE_PIN            GPIO_NUM_I2S_OUT_23
#define C_TRINAMIC              // using SPI control
#define C_DRIVER_TMC2130        // Which Driver Type?
#define C_CS_PIN                GPIO_NUM_I2S_OUT_22
#define C_RSENSE                X_RSENSE

#define SPINDLE_TYPE            SPINDLE_TYPE_PWM // only one spindle at a time
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_26
#define SPINDLE_ENABLE_PIN      GPIO_NUM_4
#define SPINDLE_DIR_PIN         GPIO_NUM_16

#define X_LIMIT_PIN             GPIO_NUM_36
#define Y_LIMIT_PIN             GPIO_NUM_39
#define Z_LIMIT_PIN             GPIO_NUM_34
#define A_LIMIT_PIN             GPIO_NUM_35
#define B_LIMIT_PIN             GPIO_NUM_32
#define C_LIMIT_PIN             GPIO_NUM_33
#define LIMIT_MASK              B111111

#define PROBE_PIN               GPIO_NUM_25

#define COOLANT_MIST_PIN        GPIO_NUM_2

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

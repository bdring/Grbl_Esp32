/*
    i2s_out_xyzabc.h
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
#define MACHINE_NAME            "ESP32 I2S 6 Axis Driver Board (StepStick)"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 6

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


#define STEPPER_MS1             GPIO_NUM_23 // MOSI
#define STEPPER_MS2             GPIO_NUM_18 // SCK

#define STEPPER_X_MS3           I2S_OUT_3   // X_CS
#define STEPPER_Y_MS3           I2S_OUT_6   // Y_CS
#define STEPPER_Z_MS3           I2S_OUT_11  // Z_CS
#define STEPPER_A_MS3           I2S_OUT_14  // A_CS
#define STEPPER_B_MS3           I2S_OUT_19  // B_CS
#define STEPPER_C_MS3           I2S_OUT_22  // C_CS

#define STEPPER_RESET           GPIO_NUM_19

#define X_DISABLE_PIN           I2S_OUT_0
#define X_DIRECTION_PIN         I2S_OUT_1
#define X_STEP_PIN              I2S_OUT_2

#define Y_DIRECTION_PIN         I2S_OUT_4
#define Y_STEP_PIN              I2S_OUT_5
#define Y_DISABLE_PIN           I2S_OUT_7

#define Z_DISABLE_PIN           I2S_OUT_8
#define Z_DIRECTION_PIN         I2S_OUT_9
#define Z_STEP_PIN              I2S_OUT_10

#define A_DIRECTION_PIN         I2S_OUT_12
#define A_STEP_PIN              I2S_OUT_13
#define A_DISABLE_PIN           I2S_OUT_15

#define B_DISABLE_PIN           I2S_OUT_16
#define B_DIRECTION_PIN         I2S_OUT_17
#define B_STEP_PIN              I2S_OUT_18
//#define B_CS_PIN                I2S_OUT_19

#define C_DIRECTION_PIN         I2S_OUT_20
#define C_STEP_PIN              I2S_OUT_21
//#define C_CS_PIN                I2S_OUT_22
#define C_DISABLE_PIN           I2S_OUT_23


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

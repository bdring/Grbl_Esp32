#pragma once
// clang-format off

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

#define I2S_OUT_BCK      "gpio.22"
#define I2S_OUT_WS       "gpio.17"
#define I2S_OUT_DATA     "gpio.21"


#define STEPPER_MS1             "gpio.23" // MOSI
#define STEPPER_MS2             "gpio.18" // SCK

#define STEPPER_X_MS3           "i2so.3"   // X_CS
#define STEPPER_Y_MS3           "i2so.6"   // Y_CS
#define STEPPER_Z_MS3           "i2so.11"  // Z_CS
#define STEPPER_A_MS3           "i2so.14"  // A_CS
#define STEPPER_B_MS3           "i2so.19"  // B_CS
#define STEPPER_C_MS3           "i2so.22"  // C_CS

#define STEPPER_RESET           "gpio.19"

#define X_DISABLE_PIN           "i2so.0"
#define X_DIRECTION_PIN         "i2so.1"
#define X_STEP_PIN              "i2so.2"

#define Y_DIRECTION_PIN         "i2so.4"
#define Y_STEP_PIN              "i2so.5"
#define Y_DISABLE_PIN           "i2so.7"

#define Z_DISABLE_PIN           "i2so.8"
#define Z_DIRECTION_PIN         "i2so.9"
#define Z_STEP_PIN              "i2so.10"

#define A_DIRECTION_PIN         "i2so.12"
#define A_STEP_PIN              "i2so.13"
#define A_DISABLE_PIN           "i2so.15"

#define B_DISABLE_PIN           "i2so.16"
#define B_DIRECTION_PIN         "i2so.17"
#define B_STEP_PIN              "i2so.18"
//#define B_CS_PIN                "i2so.19"

#define C_DIRECTION_PIN         "i2so.20"
#define C_STEP_PIN              "i2so.21"
//#define C_CS_PIN                "i2so.22"
#define C_DISABLE_PIN           "i2so.23"


#define SPINDLE_TYPE            SpindleType::PWM // only one spindle at a time
#define SPINDLE_OUTPUT_PIN      "gpio.26"
#define SPINDLE_ENABLE_PIN      "gpio.4"
#define SPINDLE_DIR_PIN         "gpio.16"

#define X_LIMIT_PIN             "gpio.36"
#define Y_LIMIT_PIN             "gpio.39"
#define Z_LIMIT_PIN             "gpio.34"
#define A_LIMIT_PIN             "gpio.35"
#define B_LIMIT_PIN             "gpio.32"
#define C_LIMIT_PIN             "gpio.33"

#define PROBE_PIN               "gpio.25"

#define COOLANT_MIST_PIN        "gpio.2"



// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

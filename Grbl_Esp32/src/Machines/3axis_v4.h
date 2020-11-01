#pragma once
// clang-format off

/*
    3axis_v4.h
    Part of Grbl_ESP32

    Pin assignments for the ESP32 Development Controller, v4.1 and later.
    https://github.com/bdring/Grbl_ESP32_Development_Controller
    https://www.tindie.com/products/33366583/grbl_esp32-cnc-development-board-v35/

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

#define MACHINE_NAME            "ESP32_V4"

#define X_STEP_PIN              "gpio.12"
#define X_DIRECTION_PIN         "gpio.14"
#define Y_STEP_PIN              "gpio.26"
#define Y_DIRECTION_PIN         "gpio.15"
#define Z_STEP_PIN              "gpio.27"
#define Z_DIRECTION_PIN         "gpio.33"

#define X_LIMIT_PIN             "gpio.17"
#define Y_LIMIT_PIN             "gpio.4"
#define Z_LIMIT_PIN             "gpio.16"


// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    "gpio.13"

#define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_OUTPUT_PIN      "gpio.2"   // labeled SpinPWM
#define SPINDLE_ENABLE_PIN      "gpio.22"  // labeled SpinEnbl

#define COOLANT_MIST_PIN        "gpio.21"  // labeled Mist
#define COOLANT_FLOOD_PIN       "gpio.25"  // labeled Flood
#define PROBE_PIN               "gpio.32"  // labeled Probe


/*
#define CONTROL_SAFETY_DOOR_PIN "gpio.35"  // labeled Door,  needs external pullup
#define CONTROL_RESET_PIN       "gpio.34"  // labeled Reset, needs external pullup
#define CONTROL_FEED_HOLD_PIN   "gpio.36"  // labeled Hold,  needs external pullup
#define CONTROL_CYCLE_START_PIN "gpio.39"  // labeled Start, needs external pullup
*/


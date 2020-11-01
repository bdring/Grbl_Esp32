#pragma once
// clang-format off

/*
    3axis_xyx.h
    Part of Grbl_ESP32

    Pin assignments for the ESP32 Development Controller
    used to drive a dual motor gantry where the drivers
    labeled X, Y and Z drive the machine axes X, Y and X.
    https://github.com/bdring/Grbl_ESP32_Development_Controller
    https://www.tindie.com/products/33366583/grbl_esp32-cnc-development-board-v35/

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

#define MACHINE_NAME            "ESP32_V4_XYX"
#define X_STEP_PIN              "gpio.26"  /* labeled Y */
#define X_DIRECTION_PIN         "gpio.15"  /* labeled Y */
#define Y_STEP_PIN              "gpio.12"  /* labeled X */
#define Y_DIRECTION_PIN         "gpio.14"  /* labeled X */
#define Y2_STEP_PIN             "gpio.27"  /* labeled Z */
#define Y2_DIRECTION_PIN        "gpio.33"  /* labeled Z */

#define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_OUTPUT_PIN         "gpio.2"
#define SPINDLE_ENABLE_PIN      "gpio.22"

#define X_LIMIT_PIN             "gpio.17"
#define Y_LIMIT_PIN             "gpio.4"
// #define Z_LIMIT_PIN          "gpio.16"

#define STEPPERS_DISABLE_PIN    "gpio.13"

#define COOLANT_MIST_PIN        "gpio.21"
#define COOLANT_FLOOD_PIN       "gpio.25"



// see versions for X and Z
#define PROBE_PIN               "gpio.32"

#define CONTROL_SAFETY_DOOR_PIN "gpio.35"  // needs external pullup
#define CONTROL_RESET_PIN       "gpio.34"  // needs external pullup
#define CONTROL_FEED_HOLD_PIN   "gpio.36"  // needs external pullup
#define CONTROL_CYCLE_START_PIN "gpio.39"  // needs external pullup

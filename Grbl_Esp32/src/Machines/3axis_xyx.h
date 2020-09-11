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
#define X_STEP_PIN              GPIO_NUM_26  /* labeled Y */
#define X_DIRECTION_PIN         GPIO_NUM_15  /* labeled Y */
#define Y_STEP_PIN              GPIO_NUM_12  /* labeled X */
#define Y_DIRECTION_PIN         GPIO_NUM_14  /* labeled X */
#define Y2_STEP_PIN             GPIO_NUM_27  /* labeled Z */
#define Y2_DIRECTION_PIN        GPIO_NUM_33  /* labeled Z */

#define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_OUTPUT_PIN         GPIO_NUM_2
#define SPINDLE_ENABLE_PIN      GPIO_NUM_22

#define X_LIMIT_PIN             GPIO_NUM_17
#define Y_LIMIT_PIN             GPIO_NUM_4
// #define Z_LIMIT_PIN          GPIO_NUM_16

#define STEPPERS_DISABLE_PIN    GPIO_NUM_13

#define COOLANT_MIST_PIN        GPIO_NUM_21
#define COOLANT_FLOOD_PIN       GPIO_NUM_25



// see versions for X and Z
#define PROBE_PIN               GPIO_NUM_32

#define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_35  // needs external pullup
#define CONTROL_RESET_PIN       GPIO_NUM_34  // needs external pullup
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_36  // needs external pullup
#define CONTROL_CYCLE_START_PIN GPIO_NUM_39  // needs external pullup

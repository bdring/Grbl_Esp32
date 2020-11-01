#pragma once
// clang-format off

/*
    espduino.h
    Part of Grbl_ESP32

    Pin assignments for ESPDUINO-32 Boards and Protoneer V3 boards
    Note: Probe pin is mapped, but will require a 10k external pullup to 3.3V to work.

    Rebooting...See this issue https://github.com/bdring/Grbl_Esp32/issues/314
    !!!! Experimental Untested !!!!!

    2019    - Bart Dring
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

#define MACHINE_NAME "ESPDUINO_32"

#define X_STEP_PIN              "gpio.26"
#define X_DIRECTION_PIN         "gpio.16"

#define Y_STEP_PIN              "gpio.25"
#define Y_DIRECTION_PIN         "gpio.27"

#define Z_STEP_PIN              "gpio.17"
#define Z_DIRECTION_PIN         "gpio.14"

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    "gpio.12"

#define SPINDLE_TYPE SpindleType::PWM
#define SPINDLE_OUTPUT_PIN         "gpio.19"

#define SPINDLE_DIR_PIN         "gpio.18"

#define COOLANT_FLOOD_PIN       "gpio.34"
#define COOLANT_MIST_PIN        "gpio.36"

#define X_LIMIT_PIN             "gpio.13"
#define Y_LIMIT_PIN             "gpio.5"
#define Z_LIMIT_PIN             "gpio.19"

#define PROBE_PIN               "gpio.39"

#define CONTROL_RESET_PIN       "gpio.2"
#define CONTROL_FEED_HOLD_PIN   "gpio.4"
#define CONTROL_CYCLE_START_PIN "gpio.35" // ESP32 needs external pullup

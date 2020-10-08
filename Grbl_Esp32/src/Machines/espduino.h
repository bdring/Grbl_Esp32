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

#define X_STEP_PIN              GPIO_NUM_26
#define X_DIRECTION_PIN         GPIO_NUM_16

#define Y_STEP_PIN              GPIO_NUM_25
#define Y_DIRECTION_PIN         GPIO_NUM_27

#define Z_STEP_PIN              GPIO_NUM_17
#define Z_DIRECTION_PIN         GPIO_NUM_14

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_12

#define SPINDLE_TYPE SpindleType::PWM
#define SPINDLE_OUTPUT_PIN         GPIO_NUM_19

#define SPINDLE_DIR_PIN         GPIO_NUM_18

#define COOLANT_FLOOD_PIN       GPIO_NUM_34
#define COOLANT_MIST_PIN        GPIO_NUM_36

#define X_LIMIT_PIN             GPIO_NUM_13
#define Y_LIMIT_PIN             GPIO_NUM_5
#define Z_LIMIT_PIN             GPIO_NUM_19

#define PROBE_PIN               GPIO_NUM_39

#define CONTROL_RESET_PIN       GPIO_NUM_2
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_4
#define CONTROL_CYCLE_START_PIN GPIO_NUM_35 // ESP32 needs external pullup

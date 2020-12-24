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

#define X_MOTOR_TYPE            MotorType::StepStick
#define X_STEP_PIN              "gpio.12"
#define X_DIRECTION_PIN         "gpio.14"

#define Y_MOTOR_TYPE            MotorType::StepStick
#define Y_STEP_PIN              "gpio.26"
#define Y_DIRECTION_PIN         "gpio.15"

#define Z_MOTOR_TYPE            MotorType::StepStick
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

/*
$Machine/Name=Grbl_ESP32 Dev V4
$Axes=3

$X/Motor/Type=StepStick
$X/Step/Pin=gpio.12
$X/Direction/Pin=gpio.14:low

$Y/Motor/Type=StepStick
$Y/Step/Pin=gpio.26
$Y/Direction/Pin=gpio.15:high

$Z/Motor/Type=StepStick
$Z/Step/Pin=gpio.27
$Z/Direction/Pin=gpio.33:high

$X/Limit/Pin=gpio.17:pu:low
$Y/Limit/Pin=gpio.4:pu:low
$Z/Limit/Pin=gpio.16:pu:low

$Steppers/Disable/Pin=gpio.13

$Spindle/Type=PWM
$Spindle/Output/Pin=gpio.2
$Spindle/Enable/Pin=gpio.22

$Coolant/Mist/Pin=gpio.21
$Coolant/Flood/Pin=gpio.25
$Probe/Pin=gpio.32:pu:low

$Control/CycleStart/Pin=gpio.39:low
$Control/FeedHold/Pin=gpio.36:low
$Control/Reset/Pin=gpio.34:low
$Control/SafetyDoor/Pin=gpio.35:low

*/

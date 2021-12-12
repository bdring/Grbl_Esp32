#pragma once
// clang-format off

/*
    midtbot.h
    Part of Grbl_ESP32

    Pin assignments for the Buildlog.net midtbot
    https://github.com/bdring/midTbot_esp32

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

#define MACHINE_NAME "midTbot"

#define DISPLAY_CODE_FILENAME   "Custom/oled_basic.cpp"

#define CUSTOM_CODE_FILENAME    "../Custom/CoreXY.cpp"
#define MIDTBOT             // applies the geometry correction to the kinematics 
#define USE_KINEMATICS      // there are kinematic equations for this machine
#define USE_FWD_KINEMATICS  // report in cartesian
#define SPINDLE_TYPE    SpindleType::NONE

#define TRINAMIC_UART_RUN_MODE       TrinamicUartMode :: StealthChop
#define TRINAMIC_UART_HOMING_MODE    TrinamicUartMode :: StealthChop

#define TMC_UART                UART_NUM_1
#define TMC_UART_RX             GPIO_NUM_21
#define TMC_UART_TX             GPIO_NUM_22   

#define X_TRINAMIC_DRIVER       2209
#define X_STEP_PIN              GPIO_NUM_25 //GPIO_NUM_32
#define X_DIRECTION_PIN         GPIO_NUM_33 // GPIO_NUM_26
#define X_RSENSE                TMC2209_RSENSE_DEFAULT
#define X_DRIVER_ADDRESS        1
#define DEFAULT_X_MICROSTEPS    16

#define Y_TRINAMIC_DRIVER       2209
#define Y_STEP_PIN              GPIO_NUM_32 //GPIO_NUM_25
#define Y_DIRECTION_PIN         GPIO_NUM_26 //GPIO_NUM_33
#define Y_RSENSE                TMC2209_RSENSE_DEFAULT
#define Y_DRIVER_ADDRESS        0
#define DEFAULT_Y_MICROSTEPS    16

#define STEPPERS_DISABLE_PIN GPIO_NUM_27

#define X_LIMIT_PIN     GPIO_NUM_4
#define Y_LIMIT_PIN     GPIO_NUM_12

#define Z_SERVO_PIN             GPIO_NUM_15

// Set $Homing/Cycle0=Y and $Homing/Cycle1=X

#define SPINDLE_TYPE SpindleType::NONE

// ==================   defaults ================================
#define DEFAULT_HOMING_CYCLE_0      bit(Z_AXIS)
#define DEFAULT_HOMING_CYCLE_1      bit(Y_AXIS)
#define DEFAULT_HOMING_CYCLE_2      bit(X_AXIS)

#define DEFAULT_HOMING_DIR_MASK     (bit(X_AXIS) | bit (Z_AXIS)) // these home negative

#define DEFAULT_STEP_PULSE_MICROSECONDS 3
#define DEFAULT_STEPPER_IDLE_LOCK_TIME  255 // stay on

#define DEFAULT_STEPPING_INVERT_MASK    0 // uint8_t
#define DEFAULT_DIRECTION_INVERT_MASK   2 // uint8_t
#define DEFAULT_INVERT_ST_ENABLE        0 // boolean
#define DEFAULT_INVERT_LIMIT_PINS       0 // boolean
#define DEFAULT_INVERT_PROBE_PIN        0 // boolean

#define DEFAULT_STATUS_REPORT_MASK 1

#define DEFAULT_JUNCTION_DEVIATION  0.01 // mm
#define DEFAULT_ARC_TOLERANCE       0.002 // mm
#define DEFAULT_REPORT_INCHES       0 // false

#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
#define DEFAULT_HARD_LIMIT_ENABLE 0  // false

#define DEFAULT_HOMING_ENABLE           1
#define DEFAULT_HOMING_FEED_RATE        500.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE        2000.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY   250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF          3.0 // mm

#define DEFAULT_X_STEPS_PER_MM 100.0
#define DEFAULT_Y_STEPS_PER_MM 100.0
#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode

#define DEFAULT_X_MAX_RATE 8000.0 // mm/min
#define DEFAULT_Y_MAX_RATE 8000.0 // mm/min
#define DEFAULT_Z_MAX_RATE 5000.0 // mm/min

#define DEFAULT_X_ACCELERATION 200.0 // mm/sec^2. 200 mm/sec^2 = 720000 mm/min^2
#define DEFAULT_Y_ACCELERATION 200.0 // mm/sec^2
#define DEFAULT_Z_ACCELERATION 100.0 // mm/sec^2

#define DEFAULT_X_MAX_TRAVEL 100.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 100.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 5.0 // This is percent in servo mode

#define DEFAULT_X_HOMING_MPOS DEFAULT_Z_MAX_TRAVEL // stays up after homing



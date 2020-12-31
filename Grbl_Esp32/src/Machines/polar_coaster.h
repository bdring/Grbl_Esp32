#pragma once
// clang-format off

/*
    polar_coaster.h
    Part of Grbl_ESP32

    Pin assignments and other configuration for the buildlog.net Polar Coaster.
    https://github.com/bdring/Polar-Coaster

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

#define MACHINE_NAME "POLAR_COASTER"

// This causes the custom code file to be included in the build
// via ../custom_code.cpp
#define CUSTOM_CODE_FILENAME "Custom/polar_coaster.cpp"

#define SPINDLE_TYPE SpindleType::NONE

#define RADIUS_AXIS 0
#define POLAR_AXIS 1

#define SEGMENT_LENGTH 0.5 // segment length in mm
#define USE_KINEMATICS
#define USE_FWD_KINEMATICS // report in cartesian
#define USE_M30

#define X_STEP_PIN              "gpio.15"
#define Y_STEP_PIN              "gpio.2"
#define X_DIRECTION_PIN         "gpio.25"
#define Y_DIRECTION_PIN         "gpio.26"

#define STEPPERS_DISABLE_PIN    "gpio.17"

#define Z_MOTOR_TYPE             MotorType::RCServo
#define Z_SERVO_PIN             "gpio.16"

#define X_LIMIT_PIN             "gpio.4:low"

#define SPINDLE_TYPE SpindleType::NONE

#ifndef ENABLE_CONTROL_SW_DEBOUNCE
    #define ENABLE_CONTROL_SW_DEBOUNCE
#endif

#ifdef CONTROL_SW_DEBOUNCE_PERIOD
    #undef CONTROL_SW_DEBOUNCE_PERIOD
#endif
#define CONTROL_SW_DEBOUNCE_PERIOD 100 // really long debounce

#define MACRO_BUTTON_0_PIN      "gpio.13:low"
#define MACRO_BUTTON_1_PIN      "gpio.12:low"
#define MACRO_BUTTON_2_PIN      "gpio.14:low"


// this 'bot only homes the X axis
// Set $Homing/Cycle0=X

// ============= End CPU MAP ==================

// ============= Begin Default Settings ================
#define DEFAULT_STEP_PULSE_MICROSECONDS 3
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay on

#define DEFAULT_DIRECTION_INVERT_MASK 2 // uint8_t

#define DEFAULT_STATUS_REPORT_MASK 2 // MPos enabled

#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
#define DEFAULT_ARC_TOLERANCE 0.002 // mm
#define DEFAULT_REPORT_INCHES 0 // false

#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
#define DEFAULT_HARD_LIMIT_ENABLE 0  // false

#define DEFAULT_HOMING_ENABLE 1
#define DEFAULT_HOMING_DIR_MASK 0 // move positive dir Z, negative X,Y
#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE 1000.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF 3.0 // mm

#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm

#define DEFAULT_LASER_MODE 0 // false

#define DEFAULT_X_STEPS_PER_MM 200.0
#define DEFAULT_Y_STEPS_PER_MM 71.111
#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode

#define DEFAULT_X_MAX_RATE 5000.0 // mm/min
#define DEFAULT_Y_MAX_RATE 15000.0 // mm/min
#define DEFAULT_Z_MAX_RATE 3000.0 // mm/min

#define DEFAULT_X_ACCELERATION 200.0 // mm/sec^2. 200 mm/sec^2 = 720000 mm/min^2
#define DEFAULT_Y_ACCELERATION 200.0 // mm/sec^2
#define DEFAULT_Z_ACCELERATION 50.0 // mm/sec^2

#define DEFAULT_X_MAX_TRAVEL 50.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 100.0 // This is percent in servo mode

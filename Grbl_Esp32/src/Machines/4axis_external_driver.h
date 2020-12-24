#pragma once
// clang-format off

/*
    4axis_external_driver.h
    Part of Grbl_ESP32

    Pin assignments for the buildlog.net 4-axis external driver board
    https://github.com/bdring/4_Axis_SPI_CNC

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

#define MACHINE_NAME            "External 4 Axis Driver Board V2"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 4

#define X_MOTOR_TYPE            MotorType::External
#define X_STEP_PIN              "gpio.0"
#define X_DIRECTION_PIN         "gpio.2"

#define Y_MOTOR_TYPE            MotorType::External
#define Y_STEP_PIN              "gpio.26"
#define Y_DIRECTION_PIN         "gpio.15"

#define Z_MOTOR_TYPE            MotorType::External
#define Z_STEP_PIN              "gpio.27"
#define Z_DIRECTION_PIN         "gpio.33"

#define A_MOTOR_TYPE            MotorType::External
#define A_STEP_PIN              "gpio.12"
#define A_DIRECTION_PIN         "gpio.14"
#define STEPPERS_DISABLE_PIN    "gpio.13"

/*
#define SPINDLE_TYPE            SpindleType::PWM // only one spindle at a time
*/

#define SPINDLE_OUTPUT_PIN      "gpio.25"
#define SPINDLE_ENABLE_PIN      "gpio.22"


#define SPINDLE_TYPE            SpindleType::HUANYANG // only one spindle at a time
#define VFD_RS485_TXD_PIN		"gpio.17"
#define VFD_RS485_RXD_PIN		"gpio.4"
#define VFD_RS485_RTS_PIN		"gpio.16"

#define X_LIMIT_PIN             "gpio.34"
#define Y_LIMIT_PIN             "gpio.35"
#define Z_LIMIT_PIN             "gpio.36"

#if (N_AXIS != 3)
        #define A_LIMIT_PIN     "gpio.39"
#endif

#define PROBE_PIN               "gpio.32"
#define COOLANT_MIST_PIN        "gpio.21"

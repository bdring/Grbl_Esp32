#pragma once
// clang-format off

/*
    TMC2209_4x.h
    https://github.com/FYSETC/FYSETC-E4

    2020-12-29 B. Dring

    This is a machine definition file to use the FYSTEC E4 3D Printer controller
    This is a 4 motor controller. This is setup for XYZA, but XYYZ, could also be used.
    There are 5 inputs
    The controller has outputs for a Fan, Hotbed and Extruder. There are mapped to
    spindle, mist and flood coolant to drive an external relay.

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

#define MACHINE_NAME            "TMC2209 4x Controller"

#define N_AXIS 4

#define TRINAMIC_RUN_MODE       TrinamicMode :: StealthChop
#define TRINAMIC_HOMING_MODE    TrinamicMode :: StealthChop

#define TMC_UART                UART_NUM_1
//#define TMC_UART_RX             GPIO_NUM_21
//#define TMC_UART_TX             GPIO_NUM_22   

#define TMC_UART_TXD             "gpio.22"
#define TMC_UART_RXD             "gpio.21"

#define X_MOTOR_TYPE            MotorType::TMC2209
#define X_STEP_PIN              "gpio.26"
#define X_DIRECTION_PIN         "gpio.27"
#define X_RSENSE                0.11f
#define DEFAULT_X_MICROSTEPS    16

#define Y_MOTOR_TYPE            MotorType::TMC2209
#define Y_STEP_PIN              "gpio.33"
#define Y_DIRECTION_PIN         "gpio.32"
#define Y_RSENSE                0.11f
#define DEFAULT_Y_MICROSTEPS    16

#define Z_MOTOR_TYPE            MotorType::TMC2209
#define Z_STEP_PIN              "gpio.2"
#define Z_DIRECTION_PIN         "gpio.4"
#define Z_RSENSE                0.11f
#define DEFAULT_Z_MICROSTEPS    16

#define A_MOTOR_TYPE            MotorType::TMC2209
#define A_STEP_PIN              "gpio.16"
#define A_DIRECTION_PIN         "gpio.17"
#define A_RSENSE                0.11f
#define DEFAULT_A_MICROSTEPS    16

#define X_LIMIT_PIN             "gpio.36"
#define Y_LIMIT_PIN             "gpio.39"
#define Z_LIMIT_PIN             "gpio.34"
#define PROBE_PIN               "gpio.35"  // Labeled TE
/*
$X/Motor/Address=0
$Y/Motor/Address=1
$Z/Motor/Address=2
$A/Motor/Address=3
*/

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    "gpio.25"

#define SPINDLE_TYPE            SpindleType::RELAY
#define SPINDLE_OUTPUT_PIN      "gpio.14"  // labeled Fan
#define SPINDLE_ENABLE_PIN      "gpio.13"
#define LASER_OUTPUT_PIN        "gpio.15"  // labeled Fan
#define LASER_ENABLE_PIN        "gpio.12"

// ===================== defaults ======================
// https://github.com/bdring/Grbl_Esp32/wiki/Setting-Defaults

#define DEFAULT_INVERT_PROBE_PIN 1

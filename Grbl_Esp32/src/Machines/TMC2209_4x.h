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

#define TRINAMIC_UART_RUN_MODE       TrinamicUartMode :: StealthChop
#define TRINAMIC_UART_HOMING_MODE    TrinamicUartMode :: StallGuard

#define TMC_UART                UART_NUM_1
#define TMC_UART_RX             GPIO_NUM_21
#define TMC_UART_TX             GPIO_NUM_22   

#define X_TRINAMIC_DRIVER       2209
#define X_STEP_PIN              GPIO_NUM_26
#define X_DIRECTION_PIN         GPIO_NUM_27
#define X_RSENSE                TMC2209_RSENSE_DEFAULT
#define X_DRIVER_ADDRESS        0
#define DEFAULT_X_MICROSTEPS    16

#define Y_TRINAMIC_DRIVER       2209
#define Y_STEP_PIN              GPIO_NUM_33
#define Y_DIRECTION_PIN         GPIO_NUM_32
#define Y_RSENSE                TMC2209_RSENSE_DEFAULT
#define Y_DRIVER_ADDRESS        1
#define DEFAULT_Y_MICROSTEPS    16

#define Z_TRINAMIC_DRIVER       2209
#define Z_STEP_PIN              GPIO_NUM_2
#define Z_DIRECTION_PIN         GPIO_NUM_14
#define Z_RSENSE                TMC2209_RSENSE_DEFAULT
#define Z_DRIVER_ADDRESS        2
#define DEFAULT_Z_MICROSTEPS    16

#define A_TRINAMIC_DRIVER       2209
#define A_STEP_PIN              GPIO_NUM_16
#define A_DIRECTION_PIN         GPIO_NUM_15
#define A_RSENSE                TMC2209_RSENSE_DEFAULT
#define A_DRIVER_ADDRESS        3
#define DEFAULT_A_MICROSTEPS    16

#define X_LIMIT_PIN             GPIO_NUM_35
#define Y_LIMIT_PIN             GPIO_NUM_34
#define Z_LIMIT_PIN             GPIO_NUM_39
#define PROBE_PIN               GPIO_NUM_36

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_25


// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-5V-Buffered-Output-Module
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/Quad-MOSFET-Module
#define USER_DIGITAL_PIN_0      GPIO_NUM_4 //  M62 M63
#define USER_DIGITAL_PIN_1      GPIO_NUM_13 //  M62 M63
#define USER_DIGITAL_PIN_2      GPIO_NUM_17 //  M62 M63
#define USER_DIGITAL_PIN_3      GPIO_NUM_12 //  M62 M63


// ===================== defaults ======================
// https://github.com/bdring/Grbl_Esp32/wiki/Setting-Defaults

#define DEFAULT_INVERT_PROBE_PIN 1

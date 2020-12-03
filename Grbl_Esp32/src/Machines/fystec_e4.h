#pragma once
// clang-format off

/*
    fystec_e4.h
    Part of Grbl_ESP3
    

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

#define MACHINE_NAME            "FYSTEC E4"

#define N_AXIS 3

#define TRINAMIC_RUN_MODE           TrinamicMode :: CoolStep
#define TRINAMIC_HOMING_MODE        TrinamicMode :: CoolStep

#define HW_SERIAL_MOTORS

#ifdef SW_SERIAL_MOTORS
    #define RX_SW_SERIAL_MOTORS     GPIO_NUM_21
    #define TX_SW_SERIAL_MOTORS     GPIO_NUM_22
#endif

#define TMC_UART                UART_NUM_1
#define TMC_UART_RX             GPIO_NUM_21
#define TMC_UART_TX             GPIO_NUM_22

#ifdef HW_SERIAL_MOTORS
    #define SERIAL_FOR_MOTORS       &Serial2    
#endif

#define X_TRINAMIC_DRIVER       2209
#define X_STEP_PIN              GPIO_NUM_27
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_RSENSE                TMC2209_RSENSE_DEFAULT
#define X_DRIVER_ADDRESS        B10

#define Y_TRINAMIC_DRIVER       2209
#define Y_STEP_PIN              GPIO_NUM_33
#define Y_DIRECTION_PIN         GPIO_NUM_32
#define Y_RSENSE                TMC2209_RSENSE_DEFAULT
#define Y_DRIVER_ADDRESS        B11

#define Z_TRINAMIC_DRIVER       2209
#define Z_STEP_PIN              GPIO_NUM_14
#define Z_DIRECTION_PIN         GPIO_NUM_12
#define Z_RSENSE                TMC2209_RSENSE_DEFAULT
#define Z_DRIVER_ADDRESS        B00

// #define A_TRINAMIC_DRIVER       2209
// #define A_STEP_PIN              GPIO_NUM_16
// #define A_DIRECTION_PIN         GPIO_NUM_17
// #define A_RSENSE                TMC2209_RSENSE_DEFAULT
// #define A_DRIVER_ADDRESS        B01

#define X_LIMIT_PIN             GPIO_NUM_34
#define Y_LIMIT_PIN             GPIO_NUM_35
#define Z_LIMIT_PIN             GPIO_NUM_15


// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_25



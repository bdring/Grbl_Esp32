#pragma once
// clang-format off

/*
	6_pack_3axis_external_rs485.h

    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board, 4 axis XYYZ, 1 axis A
    Spindle is RS485, dual gantry endstops, and some other inputs like 
    probe.

    2018    - Bart Dring
    2020    - Mitch Bradley
    2020    - Michiyasu Odaki
	2021    - Stefan de Bruijn

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
#define MACHINE_NAME            "6 Pack Controller RS485 H2A XYYZA"

#define N_AXIS 3

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
// #define DEFAULT_STEPPER ST_I2S_STATIC

// === Default settings
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define I2S_OUT_BCK             GPIO_NUM_22
#define I2S_OUT_WS              GPIO_NUM_17
#define I2S_OUT_DATA            GPIO_NUM_21


// Motor Socket #1
#define X_DISABLE_PIN           I2SO(0)     // geel
#define X_DIRECTION_PIN         I2SO(1)
#define X_STEP_PIN              I2SO(2)
// #define X_STEPPER_MS3           I2SO(3)

// Motor Socket #2
#define Y_DISABLE_PIN           I2SO(7)
#define Y_DIRECTION_PIN         I2SO(4)     // groen
#define Y_STEP_PIN              I2SO(5)
// #define Y_STEPPER_MS3           I2SO(6)

// Motor Socket #3
#define Y2_DISABLE_PIN          I2SO(8)     // zwart
#define Y2_DIRECTION_PIN        I2SO(9)
#define Y2_STEP_PIN             I2SO(10)
// #define Y2_STEPPER_MS3          I2SO(11) 

// Motor Socket #4
#define Z_DISABLE_PIN           I2SO(15)
#define Z_DIRECTION_PIN         I2SO(12)    // rood
#define Z_STEP_PIN              I2SO(13)
// #define Z_STEPPER_MS3           I2SO(14) 

// Motor Socket #6
#define A_DIRECTION_PIN         I2SO(20)
#define A_STEP_PIN              I2SO(21)
#define A_DISABLE_PIN           I2SO(23)
// #define A_STEPPER_MS3           I2SO(22)

/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    Click on each module to get example for using the modules in the sockets


Socket #1           4x input
#1 GPIO_NUM_33
#2 GPIO_NUM_32
#3 GPIO_NUM_35 (input only)
#4 GPIO_NUM_34 (input only)

Socket #2           4x input
#1 GPIO_NUM_2
#2 GPIO_NUM_25
#3 GPIO_NUM_39 (input only)
#4 GPIO_NUM_36 (input only)

Socket #3           rs485
#1 GPIO_NUM_26
#2 GPIO_NUM_4
#3 GPIO_NUM_16
#4 GPIO_NUM_27

Socket #4           relay
#1 GPIO_NUM_14
#2 GPIO_NUM_13
#3 GPIO_NUM_15
#4 GPIO_NUM_12

Socket #5
#1 I2SO(24)  (output only)
#2 I2SO(25)  (output only)
#3 I2SO26)  (output only)

*/

// 4x Input Module in Socket #1
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define X_LIMIT_PIN             GPIO_NUM_33
#define Y_LIMIT_PIN             GPIO_NUM_32
#define Y2_LIMIT_PIN            GPIO_NUM_35
#define Z_LIMIT_PIN             GPIO_NUM_34

// #define DEFAULT_INVERT_LIMIT_PINS       0  // Sets the default for N.O. switches

// 4x Switch Input module  in socket #2
// // https://github.com/bdring/6-Pack_CNC_Controller/wiki/4x-Switch-Input-module
#define CONTROL_RESET_PIN       GPIO_NUM_2
#define CONTROL_FEED_HOLD_PIN   GPIO_NUM_25
#define CONTROL_CYCLE_START_PIN GPIO_NUM_39
#define PROBE_PIN               GPIO_NUM_36

// #ifdef INVERT_CONTROL_PIN_MASK
//     #undef INVERT_CONTROL_PIN_MASK
// #endif
// #define INVERT_CONTROL_PIN_MASK B00000000

// RS485 Modbus In socket #3
// https://github.com/bdring/6-Pack_CNC_Controller/wiki/RS485-Modbus-Module
#define SPINDLE_TYPE            SpindleType::H2A // default, check actual $Spindle/Type Setting
#define VFD_RS485_TXD_PIN       GPIO_NUM_26
#define VFD_RS485_RTS_PIN       GPIO_NUM_4
#define VFD_RS485_RXD_PIN       GPIO_NUM_16

// https://github.com/bdring/6-Pack_CNC_Controller/wiki/Relay-Module
// Relay module as Mist on CNC I/O Module socket #4
#define COOLANT_MIST_PIN        GPIO_NUM_14 // Relay for Mist on module socket #4

#define VFD_DEBUG_MODE
#define DEBUG_STARTUP_DELAY

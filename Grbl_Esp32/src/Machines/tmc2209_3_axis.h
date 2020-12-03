/*
    tmc2209_3_axis.h
    Part of Grbl_ESP32

    Pin assignments for the TMC2209 on 3 axis.

    2020    - The Ant Team

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


// Select a version to match your PCB

#define MACHINE_NAME    "ESP32_TMC2209 3 AXIS MOTOR"

#define SHOW_EXTENDED_SETTINGS

#define USE_TRINAMIC // Using at least 1 trinamic driver

/* Define SW_SERIAL_MOTORS or HW_SERIAL_MOTORS */
// #define SW_SERIAL_MOTORS
#define HW_SERIAL_MOTORS

#ifdef SW_SERIAL_MOTORS
#define RX_SW_SERIAL_MOTORS     GPIO_NUM_5
#define TX_SW_SERIAL_MOTORS     GPIO_NUM_5
#endif

#ifdef HW_SERIAL_MOTORS
#define SERIAL_FOR_MOTORS       &Serial1
#endif

#define TMC_UART                UART_NUM_1
#define TMC_UART_RX             GPIO_NUM_16
#define TMC_UART_TX             GPIO_NUM_17

#define X_DRIVER_ADDRESS        0
#define X_STEP_PIN              GPIO_NUM_4
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC              
#define X_TRINAMIC_DRIVER       2209                   // Which Driver Type?
#define X_RSENSE                TMC2209_RSENSE_DEFAULT // TODO: change this!
#define DEFAULT_X_CURRENT       0.20

#define Y_DRIVER_ADDRESS        1
#define Y_STEP_PIN              GPIO_NUM_21
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Y_TRINAMIC              
#define Y_TRINAMIC_DRIVER       2209                   // Which Driver Type?
#define Y_RSENSE                TMC2209_RSENSE_DEFAULT // TODO: change this!
#define DEFAULT_Y_CURRENT       0.5

#define Z_DRIVER_ADDRESS        2
#define Z_STEP_PIN              GPIO_NUM_27
#define Z_DIRECTION_PIN         GPIO_NUM_33
#define Z_TRINAMIC              
#define Z_TRINAMIC_DRIVER       2209                   // Which Driver Type?
#define Z_RSENSE                TMC2209_RSENSE_DEFAULT // TODO: change this!
#define DEFAULT_Z_CURRENT       0.25

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN    GPIO_NUM_32

#define USE_SPINDLE
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_2
// #define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_TYPE            SpindleType::BESC
// #define SPINDLE_TYPE            SpindleType::LASER
#define LASER_OUTPUT_PIN            GPIO_NUM_22 // optional

// #define LIMIT PINs 
#define X_LIMIT_PIN             GPIO_NUM_34
#define Y_LIMIT_PIN             GPIO_NUM_39
#define Z_LIMIT_PIN             GPIO_NUM_36
#define LIMIT_MASK              B111                   //TODO: to be checked

//PROBE 35
#define PROBE_PIN               GPIO_NUM_35

// defaults
#define DEFAULT_STEPPING_INVERT_MASK    0 // uint8_t
#define DEFAULT_DIRECTION_INVERT_MASK   0 // uint8_t
#define DEFAULT_INVERT_ST_ENABLE        0 // boolean
#define DEFAULT_INVERT_LIMIT_PINS       0 // boolean
#define DEFAULT_INVERT_PROBE_PIN        1 // boolean

#define DEFAULT_STATUS_REPORT_MASK      1

#define DEFAULT_JUNCTION_DEVIATION  0.01 // mm
#define DEFAULT_ARC_TOLERANCE       0.002 // mm
#define DEFAULT_REPORT_INCHES       0 // false

#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
#define DEFAULT_HARD_LIMIT_ENABLE 1 // true

#define DEFAULT_HOMING_ENABLE           1  // true
#define DEFAULT_HOMING_DIR_MASK         3 // move positive dir Z,negative X,Y
#define DEFAULT_HOMING_FEED_RATE        100.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE        200.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY   250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF          1.0 // mm


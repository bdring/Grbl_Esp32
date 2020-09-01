/*
    tmc2209_test.h
    Part of Grbl_ESP32

    Pin assignments for the TMC2209 test with a single motor driver.

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

#define MACHINE_NAME    "ESP32_TMC2209_TEST SINGLE MOTOR"

#define SHOW_EXTENDED_SETTINGS

#define USE_TRINAMIC // Using at least 1 trinamic driver

/* Define SW_SERIAL_MOTORS or HW_SERIAL_MOTORS */
//#define SW_SERIAL_MOTORS
#define HW_SERIAL_MOTORS

#ifdef SW_SERIAL_MOTORS
#define RX_SW_SERIAL_MOTORS     GPIO_NUM_5
#define TX_SW_SERIAL_MOTORS     GPIO_NUM_5
#endif

#ifdef HW_SERIAL_MOTORS
#define SERIAL_FOR_MOTORS       &Serial2
#endif

#define X_DRIVER_ADDRESS        0

#define X_STEP_PIN              GPIO_NUM_4
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC              
#define X_TRINAMIC_DRIVER       2209                   // Which Driver Type?
#define X_RSENSE                TMC2209_RSENSE_DEFAULT
#define DEFAULT_X_CURRENT       0.5

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN GPIO_NUM_32

// Define one of these 2 options for spindle or servo
//#define USE_SERVO_AXES
#define USE_SPINDLE

#define SPINDLE_TYPE            SPINDLE_TYPE_PWM
#define SPINDLE_OUTPUT_PIN      GPIO_NUM_2

// #define X_LIMIT_PIN 
#define X_LIMIT_PIN                 GPIO_NUM_39
#define LIMIT_MASK                  B1                  //TODO: to be checked

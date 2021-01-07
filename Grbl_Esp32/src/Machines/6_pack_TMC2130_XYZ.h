#pragma once
// clang-format off

/*
    6_pack_trinamic_V1.h
    Part of Grbl_ESP32
    Pin assignments for the ESP32 SPI 6-axis board
    2018    - Bart Dring
    2020    - Mitch Bradley
    2020    - Michiyasu Odaki
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
#define MACHINE_NAME            "6 Pack TMC2130 XYZ"

#define N_AXIS 3

#define I2S_OUT_BCK             "gpio.22"
#define I2S_OUT_WS              "gpio.17"
#define I2S_OUT_DATA            "gpio.21"

// Motor Socket #1
#define X_MOTOR_TYPE            MotorType::TMC2130
#define X_DISABLE_PIN           "i2so.0"
#define X_DIRECTION_PIN         "i2so.1:low"
#define X_STEP_PIN              "i2so.2"
#define X_CS_PIN                "i2so.3"
#define X_DRIVER_RSENSE         0.11

// Motor Socket #2
#define Y_MOTOR_TYPE            MotorType::TMC2130
#define Y_DIRECTION_PIN         "i2so.4:low"
#define Y_STEP_PIN              "i2so.5"
#define Y_DISABLE_PIN           "i2so.7"
#define Y_CS_PIN                "i2so.6"
#define Y_DRIVER_RSENSE         X_DRIVER_RSENSE

// Motor Socket #3
#define Z_MOTOR_TYPE            MotorType::TMC2130
#define Z_DISABLE_PIN           "i2so.8"
#define Z_DIRECTION_PIN         "i2so.9:low"
#define Z_STEP_PIN              "i2so.10"
#define Z_CS_PIN                "i2so.11"
#define Z_DRIVER_RSENSE         X_DRIVER_RSENSE


/*
    Socket I/O reference
    The list of modules is here...
    https://github.com/bdring/6-Pack_CNC_Controller/wiki/CNC-I-O-Module-List
    Click on each module to get example for using the modules in the sockets


*/

// Install StallGuard Jumpers
#define X_LIMIT_PIN             "gpio.33"  // Sg1
#define Y_LIMIT_PIN             "gpio.32"  // Sg2
#define Z_LIMIT_PIN             "gpio.35"  // Sg3



// === Default settings
// https://github.com/bdring/Grbl_Esp32/wiki/Setting-Defaults
#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define DEFAULT_HOMING_ENABLE 1
#define DEFAULT_HOMING_CYCLE_0  4  // Z
#define DEFAULT_HOMING_CYCLE_1  1  // X
#define DEFAULT_HOMING_CYCLE_2  2  // Y
#define DEFAULT_HOMING_DIR_MASK 1 // only X

#define TRINAMIC_RUN_MODE           TrinamicMode::CoolStep
#define TRINAMIC_HOMING_MODE        TrinamicMode::CoolStep

#define DEFAULT_X_STEPS_PER_MM 400
#define DEFAULT_Y_STEPS_PER_MM 400
#define DEFAULT_Z_STEPS_PER_MM 400

#define DEFAULT_X_MICROSTEPS   16
#define DEFAULT_Y_MICROSTEPS   16
#define DEFAULT_Z_MICROSTEPS   16

#define DEFAULT_X_CURRENT      1.0
#define DEFAULT_HOLD_X_CURRENT 0.5
#define DEFAULT_Y_CURRENT      1.0
#define DEFAULT_HOLD_Y_CURRENT 0.5
#define DEFAULT_Z_CURRENT      1.0
#define DEFAULT_HOLD_Z_CURRENT 0.5
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay locked

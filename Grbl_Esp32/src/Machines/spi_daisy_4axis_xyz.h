#pragma once
// clang-format off

/*
    spi_daisy_4axis_xyz.h
    Part of Grbl_ESP32

    Pin assignments for a 4-axis machine using Triaminic drivers
    in daisy-chained SPI mode.
    https://github.com/bdring/4_Axis_SPI_CNC

    2019    - Bart Dring
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

#define MACHINE_NAME "SPI_DAISY_4X_XYZ"

#define N_AXIS 3

#define TRINAMIC_DAISY_CHAIN

#define TRINAMIC_RUN_MODE           TrinamicMode :: CoolStep
#define TRINAMIC_HOMING_MODE        TrinamicMode :: CoolStep

// Use SPI enable instead of the enable pin
// The hardware enable pin is tied to ground
#define USE_TRINAMIC_ENABLE

#define X_TRINAMIC_DRIVER       MotorType::TMC2130        // Which Driver Type?
#define X_DRIVER_RSENSE         0.11
#define X_STEP_PIN              "gpio.12"
#define X_DIRECTION_PIN         "gpio.14"
#define X_CS_PIN                "gpio.17"  // Daisy Chain, all share same CS pin

#define Y_TRINAMIC_DRIVER       MotorType::TMC2130        // Which Driver Type?
#define Y_DRIVER_RSENSE         0.11
#define Y_STEP_PIN              "gpio.27"
#define Y_DIRECTION_PIN         "gpio.26"
#define Y_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin

#define Z_TRINAMIC_DRIVER       MotorType::TMC2130        // Which Driver Type?
#define Z_RSENSE                0.11
#define Z_STEP_PIN              "gpio.15"
#define Z_DIRECTION_PIN         "gpio.2"
#define Z_CS_PIN                X_CS_PIN  // Daisy Chain, all share same CS pin


// Mist is a 3.3V output
// Turn on with M7 and off with M9
#define COOLANT_MIST_PIN        "gpio.21"

#define SPINDLE_TYPE            SpindleType::PWM
#define SPINDLE_OUTPUT_PIN         "gpio.25"
#define SPINDLE_ENABLE_PIN      "gpio.4"

// Relay operation
// Install Jumper near relay
// For spindle Use max RPM of 1
// For PWM remove jumper and set MAX RPM to something higher ($30 setting)
// Interlock jumper along top edge needs to be installed for both versions
#define DEFAULT_SPINDLE_RPM_MAX     1 // Should be 1 for relay operation

#define PROBE_PIN               "gpio.22"

#define X_LIMIT_PIN             "gpio.36"
#define Y_LIMIT_PIN             "gpio.39"
#define Z_LIMIT_PIN             "gpio.34"


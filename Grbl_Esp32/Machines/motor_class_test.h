/*
    spi_daisy_4axis.h
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

#define MACHINE_NAME "MotorClass Test TMC2130 Pen board"

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 3 // can be 3 or 4. (if 3 install bypass jumper next to the A driver)

#define SPINDLE_TYPE                SPINDLE_TYPE_NONE

#define USE_TRINAMIC_CLASS
#define ENABLE_STALLGUARD_TUNING

//#define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_STEALTHCHOP 
#define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_COOLSTEP
//#define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_STALLGUARD

#define STEPPERS_DISABLE_PIN    GPIO_NUM_13

#define X_STEP_PIN              GPIO_NUM_12
#define X_DIRECTION_PIN         GPIO_NUM_26
#define X_TRINAMIC              // using SPI control
#define X_DRIVER                2130
#define X_CS_PIN                GPIO_NUM_17  //chip select
#define X_RSENSE                TMC2130_RSENSE_DEFAULT

#define Y_STEP_PIN              GPIO_NUM_14
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Y_TRINAMIC              // using SPI control
#define Y_DRIVER                2130
#define Y_CS_PIN                GPIO_NUM_16  //chip select
#define Y_RSENSE                TMC2130_RSENSE_DEFAULT

#define X_LIMIT_PIN             GPIO_NUM_32
#define Y_LIMIT_PIN             GPIO_NUM_4
#define LIMIT_MASK              B11

#pragma once
// clang-format off

/*
    i2s_out_xyzabc.h
    Part of Grbl_ESP32
    Pin assignments for the ESP32 I2S 6-axis board
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
#define MACHINE_NAME            "ESP32 I2S 5 Axis Driver Board"

#define CUSTOM_CODE_FILENAME    "../Custom/CoreXY.cpp"

// #define MIDTBOT             // applies the geometry correction to the kinematics 
#define USE_KINEMATICS      // there are kinematic equations for this machine
#define USE_FWD_KINEMATICS  // report in cartesian
// #define USE_MACHINE_INIT    // There is some custom initialization for this machine
// #define USE_CUSTOM_HOMING

#ifdef N_AXIS
        #undef N_AXIS
#endif
#define N_AXIS 5

#ifdef ENABLE_SD_CARD
    #undef ENABLE_SD_CARD
#endif

// === Special Features

// I2S (steppers & other output-only pins)
#define USE_I2S_OUT
#define USE_I2S_STEPS
//#define DEFAULT_STEPPER ST_I2S_STATIC

// #define USE_STEPSTICK   // makes sure MS1,2,3 !reset and !sleep are set

#define I2S_OUT_BCK      GPIO_NUM_0
#define I2S_OUT_WS       GPIO_NUM_16
#define I2S_OUT_DATA     GPIO_NUM_17


#define X_DISABLE_PIN           I2SO(2)
#define X_STEP_PIN              I2SO(1)
#define X_DIRECTION_PIN         I2SO(0)


#define Y_DISABLE_PIN           I2SO(5)
#define Y_STEP_PIN              I2SO(4)
#define Y_DIRECTION_PIN         I2SO(3)

#define Z_DISABLE_PIN           I2SO(8)
#define Z_STEP_PIN              I2SO(7)
#define Z_DIRECTION_PIN         I2SO(6)

#define A_DISABLE_PIN           I2SO(11)
#define A_STEP_PIN              I2SO(10)
#define A_DIRECTION_PIN         I2SO(9)

#define B_DISABLE_PIN           I2SO(14)
#define B_STEP_PIN              I2SO(13)
#define B_DIRECTION_PIN         I2SO(12)

#define SPINDLE_TYPE            SpindleType::LASER // only one spindle at a time
#define LASER_OUTPUT_PIN        GPIO_NUM_27

#define X_LIMIT_PIN             GPIO_NUM_35
#define Y_LIMIT_PIN             GPIO_NUM_32
#define Z_LIMIT_PIN             GPIO_NUM_33
//#define A_LIMIT_PIN             GPIO_NUM_35
//#define B_LIMIT_PIN             GPIO_NUM_32

//#define PROBE_PIN               GPIO_NUM_25

//#define COOLANT_MIST_PIN        GPIO_NUM_2

// The default value in config.h is wrong for this controller
// #ifdef INVERT_CONTROL_PIN_MASK
//     #undef INVERT_CONTROL_PIN_MASK
// #endif

// #define INVERT_CONTROL_PIN_MASK B1110

// #define CONTROL_RESET_PIN           GPIO_NUM_34  // needs external pullup
// #define CONTROL_FEED_HOLD_PIN       GPIO_NUM_36  // needs external pullup
// #define CONTROL_CYCLE_START_PIN     GPIO_NUM_39  // needs external pullup

// === Default settings
#define DEFAULT_INVERT_ST_ENABLE        1 // boolean

#define DEFAULT_STEP_PULSE_MICROSECONDS I2S_OUT_USEC_PER_PULSE

#define DEFAULT_LASER_MODE 1 // true

#define DEFAULT_X_STEPS_PER_MM 914.290
#define DEFAULT_Y_STEPS_PER_MM 914.290
#define DEFAULT_Z_STEPS_PER_MM 914.290

#define DEFAULT_X_MAX_RATE 8000.0 // mm/min
#define DEFAULT_Y_MAX_RATE 8000.0 // mm/min
#define DEFAULT_Z_MAX_RATE 8000.0 // mm/min

#define DEFAULT_X_ACCELERATION 200.0 // mm/sec^2
#define DEFAULT_Y_ACCELERATION 200.0 // mm/sec^2
#define DEFAULT_Z_ACCELERATION 200.0 // mm/sec^2

#define DEFAULT_X_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.

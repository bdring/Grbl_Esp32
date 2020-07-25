/*
    lowrider_v1p2.h
    Part of Grbl_ESP32
    Pin assignments for the Buildlog.net MPCNC controller
    used in lowrider mode. Low rider has (2) Y and Z and one X motor
    These will not match the silkscreen or schematic descriptions...see definitions below
    https://github.com/bdring/Grbl_ESP32_MPCNC_Controller
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



#define MACHINE_NAME "LOWRIDER_V1P2"

#define USE_GANGED_AXES // allow two motors on an axis

#define X_STEP_PIN          GPIO_NUM_27     // use Z labeled connector
#define X_DIRECTION_PIN     GPIO_NUM_33     // use Z labeled connector

#define Y_STEP_PIN          GPIO_NUM_14     // use Y labeled connector 
#define Y2_STEP_PIN         GPIO_NUM_21     // ganged motor
#define Y_DIRECTION_PIN     GPIO_NUM_25     // use Y labeled connector
#define Y2_DIRECTION_PIN    X_DIRECTION_PIN
#define Y_AXIS_SQUARING

#define Z_STEP_PIN          GPIO_NUM_12     // use X labeled connector
#define Z2_STEP_PIN         GPIO_NUM_22     // ganged motor
#define Z_DIRECTION_PIN     GPIO_NUM_26     // use X labeled connector
#define Z2_DIRECTION_PIN    Z_DIRECTION_PIN
#define Z_AXIS_SQUARING

// OK to comment out to use pin for other features
#define STEPPERS_DISABLE_PIN GPIO_NUM_13

// Note: if you use PWM rather than relay, you could map GPIO_NUM_2 to mist or flood
//#define USE_SPINDLE_RELAY

#ifdef USE_SPINDLE_RELAY
    #define SPINDLE_TYPE SPINDLE_TYPE_RELAY
    #define SPINDLE_OUTPUT_PIN GPIO_NUM_2
#else
    #define SPINDLE_TYPE SPINDLE_TYPE_PWM
    #define SPINDLE_OUTPUT_PIN         GPIO_NUM_16
    #define SPINDLE_ENABLE_PIN         GPIO_NUM_32
#endif

// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
// Relay can be used for Spindle or Coolant
//#define COOLANT_FLOOD_PIN             GPIO_NUM_17

#define X_LIMIT_PIN             GPIO_NUM_15
#define Y_LIMIT_PIN             GPIO_NUM_4
#define Z_LIMIT_PIN             GPIO_NUM_17

#ifndef ENABLE_SOFTWARE_DEBOUNCE   // V1P2 does not have R/C filters
    #define ENABLE_SOFTWARE_DEBOUNCE
#endif

#define PROBE_PIN               GPIO_NUM_35

// The default value in config.h is wrong for this controller
#ifdef INVERT_CONTROL_PIN_MASK
    #undef INVERT_CONTROL_PIN_MASK
#endif

#define INVERT_CONTROL_PIN_MASK   B1110

#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup
#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup

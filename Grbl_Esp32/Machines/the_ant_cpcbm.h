/*
    the_ant_cpcbm.h
    Part of Grbl_ESP32

    2020    - Angelo Di Chello for The Ant Team

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

// // Pin assignments for the The Ant machine controller



#define MACHINE_NAME "THE_ANT_COMPACT_PCB_MAKER"

#define X_STEP_PIN              GPIO_NUM_12
#define Y_STEP_PIN              GPIO_NUM_14
#define Z_STEP_PIN              GPIO_NUM_27

#define X_DIRECTION_PIN         GPIO_NUM_26
#define Y_DIRECTION_PIN         GPIO_NUM_25
#define Z_DIRECTION_PIN         GPIO_NUM_33

#define STEPPERS_DISABLE_PIN    GPIO_NUM_13

#define SPINDLE_PWM_PIN         GPIO_NUM_2
#define SPINDLE_ENABLE_PIN      GPIO_NUM_32

#define X_LIMIT_PIN             GPIO_NUM_17
#define Y_LIMIT_PIN             GPIO_NUM_4
#define Z_LIMIT_PIN             GPIO_NUM_15
#define LIMIT_MASK              B111

#ifndef ENABLE_SOFTWARE_DEBOUNCE
    #define ENABLE_SOFTWARE_DEBOUNCE
#endif

#define PROBE_PIN               GPIO_NUM_35

/*
#ifdef IGNORE_CONTROL_PINS
#undef IGNORE_CONTROL_PINS
#endif
*/

#define CONTROL_RESET_PIN           GPIO_NUM_34  // needs external pullup
#define CONTROL_FEED_HOLD_PIN       GPIO_NUM_36  // needs external pullup
#define CONTROL_CYCLE_START_PIN     GPIO_NUM_39  // needs external pullup

#define DEFAULT_STEP_PULSE_MICROSECONDS     10
#define DEFAULT_STEPPER_IDLE_LOCK_TIME      255 //  255 = Keep steppers on

#define DEFAULT_STEPPING_INVERT_MASK    0 // uint8_t
#define DEFAULT_DIRECTION_INVERT_MASK   0 // uint8_t
#define DEFAULT_INVERT_ST_ENABLE        0 // boolean
#define DEFAULT_INVERT_LIMIT_PINS       0 // boolean
#define DEFAULT_INVERT_PROBE_PIN        0 // boolean

#define DEFAULT_STATUS_REPORT_MASK      1

#define DEFAULT_JUNCTION_DEVIATION  0.01 // mm
#define DEFAULT_ARC_TOLERANCE       0.002 // mm
#define DEFAULT_REPORT_INCHES       0 // false

#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
#define DEFAULT_HARD_LIMIT_ENABLE 1  // true

#define DEFAULT_HOMING_ENABLE           1  // false
#define DEFAULT_HOMING_DIR_MASK         0 // move positive dir Z,negative X,Y
#define DEFAULT_HOMING_FEED_RATE        100.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE        200.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY   250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF          2.0 // mm

#ifdef USE_SPINDLE_RELAY
    #define DEFAULT_SPINDLE_RPM_MAX 1.0 // must be 1 so PWM duty is alway 100% to prevent relay damage
#else
    #define DEFAULT_SPINDLE_RPM_MAX 1000.0 // can be change to your spindle max
#endif

#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm

#define DEFAULT_LASER_MODE 0 // false

#define DEFAULT_X_STEPS_PER_MM 200.0
#define DEFAULT_Y_STEPS_PER_MM 200.0
#define DEFAULT_Z_STEPS_PER_MM 800.0

#define DEFAULT_X_MAX_RATE 1000.0 // mm/min
#define DEFAULT_Y_MAX_RATE 1000.0 // mm/min
#define DEFAULT_Z_MAX_RATE 500.0 // mm/min

#define DEFAULT_X_ACCELERATION (10.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
#define DEFAULT_Y_ACCELERATION (10.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
#define DEFAULT_Z_ACCELERATION (10.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2

#define DEFAULT_X_MAX_TRAVEL 200.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 200.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 200.0 // mm NOTE: Must be a positive value.

#pragma once
// clang-format off

/*
        atari_1020.h
        Part of Grbl_ESP32

        Pin assignments and other configuration for an Atari 1020 pen plotter

        The pen plotter uses several special options, including unipolar
        motors, custom homing and tool changing.  The file atari_1020.cpp
        defines custom functions to handle the special features.  As such,
        this file defines not only pin assignments but also many other
        things that are necessary to override default choices that are
        inappropriate for this particular machine.

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

#define MACHINE_NAME "MACHINE_ATARI_1020"

#define CUSTOM_CODE_FILENAME "Custom/atari_1020.cpp"

#ifdef USE_RMT_STEPS
    #undef USE_RMT_STEPS
#endif

#define X_UNIPOLAR
#define X_PIN_PHASE_0 GPIO_NUM_13
#define X_PIN_PHASE_1 GPIO_NUM_21
#define X_PIN_PHASE_2 GPIO_NUM_16
#define X_PIN_PHASE_3 GPIO_NUM_22

#define Y_UNIPOLAR
#define Y_PIN_PHASE_0 GPIO_NUM_25
#define Y_PIN_PHASE_1 GPIO_NUM_27
#define Y_PIN_PHASE_2 GPIO_NUM_26
#define Y_PIN_PHASE_3 GPIO_NUM_32

#define SOLENOID_DIRECTION_PIN GPIO_NUM_4
#define SOLENOID_PEN_PIN GPIO_NUM_2

// this 'bot only homes the X axis
// Set $Homing/Cycle0=0 

#define REED_SW_PIN GPIO_NUM_17

#ifndef ENABLE_CONTROL_SW_DEBOUNCE
    #define ENABLE_CONTROL_SW_DEBOUNCE
#endif

#ifdef INVERT_CONTROL_PIN_MASK
    #undef INVERT_CONTROL_PIN_MASK
#endif
#define INVERT_CONTROL_PIN_MASK B01110000

#define MACRO_BUTTON_0_PIN GPIO_NUM_34 // Pen Switch
#define MACRO_BUTTON_1_PIN GPIO_NUM_35 // Color Switch
#define MACRO_BUTTON_2_PIN GPIO_NUM_36 // Paper Switch

#ifdef DEFAULTS_GENERIC
    #undef DEFAULTS_GENERIC // undefine generic then define each default below
#endif
#define DEFAULT_STEP_PULSE_MICROSECONDS 3
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 200 // 200ms

#define DEFAULT_STEPPING_INVERT_MASK 0  // uint8_t
#define DEFAULT_DIRECTION_INVERT_MASK 0 // uint8_t
#define DEFAULT_INVERT_ST_ENABLE 0      // boolean
#define DEFAULT_INVERT_LIMIT_PINS 1     // boolean
#define DEFAULT_INVERT_PROBE_PIN 0      // boolean

#define DEFAULT_STATUS_REPORT_MASK 1

#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
#define DEFAULT_ARC_TOLERANCE 0.002     // mm
#define DEFAULT_REPORT_INCHES 0         // false

#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
#define DEFAULT_HARD_LIMIT_ENABLE 0 // false

#define DEFAULT_HOMING_ENABLE 1
#define DEFAULT_HOMING_DIR_MASK 0
#define DEFAULT_HOMING_FEED_RATE 3000.0   // mm/min
#define DEFAULT_HOMING_SEEK_RATE 3000.0   // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF 2.0        // mm

#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
#define DEFAULT_SPINDLE_RPM_MIN 0.0    // rpm

#define DEFAULT_LASER_MODE 0 // false

#define DEFAULT_X_STEPS_PER_MM 10
#define DEFAULT_Y_STEPS_PER_MM 10
#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode

#define DEFAULT_X_MAX_RATE 5000.0   // mm/min
#define DEFAULT_Y_MAX_RATE 5000.0   // mm/min
#define DEFAULT_Z_MAX_RATE 200000.0 // mm/min

#define DEFAULT_X_ACCELERATION 500.0 // mm/sec^2. 500 mm/sec^2 = 1800000 mm/min^2
#define DEFAULT_Y_ACCELERATION 500.0 // mm/sec^2
#define DEFAULT_Z_ACCELERATION 500.0 // mm/sec^2

#define DEFAULT_X_MAX_TRAVEL 120.0   // mm NOTE: Must be a positive value.
#define DEFAULT_Y_MAX_TRAVEL 20000.0 // mm NOTE: Must be a positive value.
#define DEFAULT_Z_MAX_TRAVEL 10.0    // This is percent in servo mode

#define ATARI_1020

#define SOLENOID_PWM_FREQ 5000
#define SOLENOID_PWM_RES_BITS 8

#define SOLENOID_PULSE_LEN_PULL 255
#define SOLENOID_PULL_DURATION 50  // in task counts...after this delay power will change to hold level see SOLENOID_TASK_FREQ
#define SOLENOID_PULSE_LEN_HOLD 40 // solenoid hold level ... typically a lower value to prevent overheating

#define SOLENOID_TASK_FREQ 50 // this is milliseconds

#define MAX_PEN_NUMBER 4
#define BUMPS_PER_PEN_CHANGE 3

#define ATARI_HOME_POS -10.0f    // this amound to the left of the paper 0
#define ATARI_PAPER_WIDTH 100.0f //
#define ATARI_HOMING_ATTEMPTS 13

// tells grbl we have some special functions to call
#define ATARI_TOOL_CHANGE_Z 5.0

#ifndef atari_h
    #define atari_h

    void solenoid_disable();
    void solenoidSyncTask(void* pvParameters);
    void calc_solenoid(float penZ);
    void atari_home_task(void* pvParameters);
    void atari_next_pen();
#endif

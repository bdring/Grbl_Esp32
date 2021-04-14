#pragma once

/*
  Defaults.h - defaults settings configuration file
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

/* The defaults.h file serves as a central default settings selector for different machine
   types, from DIY CNC mills to CNC conversions of off-the-shelf machines. The settings
   files listed here are supplied by users, so your results may vary. However, this should
   give you a good starting point as you get to know your machine and tweak the settings for
   your nefarious needs.
   NOTE: Ensure one and only one of these DEFAULTS_XXX values is defined in config.h */

/*
    All of these settings check to see if they have been defined already
    before defining them. This allows to to easily set them eslewhere.
    You only need to set ones that are important or unique to your
    machine. The rest will be pulled from here.
    */

// Grbl generic default settings. Should work across different machines.
#ifndef DEFAULT_STEP_PULSE_MICROSECONDS
#    define DEFAULT_STEP_PULSE_MICROSECONDS 3  // $0
#endif

#ifndef DEFAULT_STEP_ENABLE_DELAY
#    define DEFAULT_STEP_ENABLE_DELAY 0
#endif

#ifndef STEP_PULSE_DELAY
#    define STEP_PULSE_DELAY 0
#endif

#ifndef DEFAULT_STEPPER_IDLE_LOCK_TIME
#    define DEFAULT_STEPPER_IDLE_LOCK_TIME 250  // $1 msec (0-254, 255 keeps steppers enabled)
#endif

#ifndef DEFAULT_STEPPING_INVERT_MASK
#    define DEFAULT_STEPPING_INVERT_MASK 0  // $2 uint8_t
#endif

#ifndef DEFAULT_DIRECTION_INVERT_MASK
#    define DEFAULT_DIRECTION_INVERT_MASK 0  // $3 uint8_
#endif

#ifndef DEFAULT_INVERT_ST_ENABLE
#    define DEFAULT_INVERT_ST_ENABLE 0  // $4 boolean
#endif

#ifndef DEFAULT_INVERT_LIMIT_PINS
#    define DEFAULT_INVERT_LIMIT_PINS 1  // $5 boolean
#endif

#ifndef DEFAULT_INVERT_PROBE_PIN
#    define DEFAULT_INVERT_PROBE_PIN 0  // $6 boolean
#endif

#ifndef DEFAULT_STATUS_REPORT_MASK
#    define DEFAULT_STATUS_REPORT_MASK 1  // $10
#endif

#ifndef DEFAULT_VERBOSE_ERRORS
#    define DEFAULT_VERBOSE_ERRORS 0
#endif

#ifndef DEFAULT_JUNCTION_DEVIATION
#    define DEFAULT_JUNCTION_DEVIATION 0.01  // $11 mm
#endif

#ifndef DEFAULT_ARC_TOLERANCE
#    define DEFAULT_ARC_TOLERANCE 0.002  // $12 mm
#endif

#ifndef DEFAULT_REPORT_INCHES
#    define DEFAULT_REPORT_INCHES 0  // $13 false
#endif

#ifndef DEFAULT_SOFT_LIMIT_ENABLE
#    define DEFAULT_SOFT_LIMIT_ENABLE 0  // $20 false
#endif

#ifndef DEFAULT_HARD_LIMIT_ENABLE
#    define DEFAULT_HARD_LIMIT_ENABLE 0  // $21 false
#endif

#ifndef DEFAULT_HOMING_ENABLE
#    define DEFAULT_HOMING_ENABLE 0  // $22 false
#endif

#ifndef DEFAULT_HOMING_DIR_MASK
#    define DEFAULT_HOMING_DIR_MASK 3  // $23 move positive dir Z, negative X,Y
#endif

#ifndef DEFAULT_HOMING_FEED_RATE
#    define DEFAULT_HOMING_FEED_RATE 200.0  // $24 mm/min
#endif

#ifndef DEFAULT_HOMING_SEEK_RATE
#    define DEFAULT_HOMING_SEEK_RATE 2000.0  // $25 mm/min
#endif

#ifndef DEFAULT_HOMING_DEBOUNCE_DELAY
#    define DEFAULT_HOMING_DEBOUNCE_DELAY 250  // $26 msec (0-65k)
#endif

#ifndef DEFAULT_HOMING_PULLOFF
#    define DEFAULT_HOMING_PULLOFF 1.0  // $27 mm
#endif

#ifndef DEFAULT_HOMING_SQUARED_AXES
#    define DEFAULT_HOMING_SQUARED_AXES 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_0
#    define DEFAULT_HOMING_CYCLE_0 bit(Z_AXIS)
#endif

#ifndef DEFAULT_HOMING_CYCLE_1
#    define DEFAULT_HOMING_CYCLE_1 (bit(X_AXIS) | bit(Y_AXIS))
#endif

#ifndef DEFAULT_HOMING_CYCLE_2
#    define DEFAULT_HOMING_CYCLE_2 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_3
#    define DEFAULT_HOMING_CYCLE_3 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_4
#    define DEFAULT_HOMING_CYCLE_4 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_5
#    define DEFAULT_HOMING_CYCLE_5 0
#endif

// ======== SPINDLE STUFF ====================
#ifndef SPINDLE_TYPE
#    define SPINDLE_TYPE SpindleType::NONE
#endif

#ifndef DEFAULT_SPINDLE_RPM_MIN          // $31
#    define DEFAULT_SPINDLE_RPM_MIN 0.0  // rpm
#endif

#ifndef DEFAULT_LASER_MODE        // $32
#    define DEFAULT_LASER_MODE 0  // false
#endif

#ifndef DEFAULT_LASER_FULL_POWER
#    define DEFAULT_LASER_FULL_POWER 1000
#endif

#ifndef DEFAULT_SPINDLE_RPM_MAX             // $30
#    define DEFAULT_SPINDLE_RPM_MAX 1000.0  // rpm
#endif

#ifndef DEFAULT_SPINDLE_FREQ
#    define DEFAULT_SPINDLE_FREQ 5000.0  // $33 Hz (extended set)
#endif

#ifndef DEFAULT_SPINDLE_OFF_VALUE
#    define DEFAULT_SPINDLE_OFF_VALUE 0.0  // $34 Percent of full period(extended set)
#endif

#ifndef DEFAULT_SPINDLE_MIN_VALUE
#    define DEFAULT_SPINDLE_MIN_VALUE 0.0  // $35 Percent of full period (extended set)
#endif

#ifndef DEFAULT_SPINDLE_MAX_VALUE
#    define DEFAULT_SPINDLE_MAX_VALUE 100.0  // $36 Percent of full period (extended set)
#endif

#ifndef DEFAULT_SPINDLE_DELAY_SPINUP
#    define DEFAULT_SPINDLE_DELAY_SPINUP 0
#endif

#ifndef DEFAULT_COOLANT_DELAY_TURNON
#    define DEFAULT_COOLANT_DELAY_TURNON 1.0
#endif

#ifndef DEFAULT_SPINDLE_DELAY_SPINDOWN
#    define DEFAULT_SPINDLE_DELAY_SPINDOWN 0
#endif

#ifndef DEFAULT_INVERT_SPINDLE_OUTPUT_PIN
#    define DEFAULT_INVERT_SPINDLE_OUTPUT_PIN 0
#endif

#ifndef DEFAULT_INVERT_SPINDLE_ENABLE_PIN
#    define DEFAULT_INVERT_SPINDLE_ENABLE_PIN 0
#endif

#ifndef DEFAULT_SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
#    define DEFAULT_SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED 0
#endif

// ================  user settings =====================
#ifndef DEFAULT_USER_INT_80
#    define DEFAULT_USER_INT_80 0  // $80 User integer setting
#endif

#ifndef DEFAULT_USER_INT_81
#    define DEFAULT_USER_INT_81 0  // $81 User integer setting
#endif

#ifndef DEFAULT_USER_INT_82
#    define DEFAULT_USER_INT_82 0  // $82 User integer setting
#endif

#ifndef DEFAULT_USER_INT_83
#    define DEFAULT_USER_INT_83 0  // $83 User integer setting
#endif

#ifndef DEFAULT_USER_INT_84
#    define DEFAULT_USER_INT_84 0  // $84 User integer setting
#endif

#ifndef DEFAULT_USER_FLOAT_90
#    define DEFAULT_USER_FLOAT_90 0.0  // $90 User integer setting
#endif

#ifndef DEFAULT_USER_FLOAT_91
#    define DEFAULT_USER_FLOAT_91 0.0  // $92 User integer setting
#endif

#ifndef DEFAULT_USER_FLOAT_92
#    define DEFAULT_USER_FLOAT_92 0.0  // $92 User integer setting
#endif

#ifndef DEFAULT_USER_FLOAT_93
#    define DEFAULT_USER_FLOAT_93 0.0  // $93 User integer setting
#endif

#ifndef DEFAULT_USER_FLOAT_94
#    define DEFAULT_USER_FLOAT_94 0.0  // $94 User integer setting
#endif

// =========== 	AXIS RESOLUTION ======

#ifndef DEFAULT_X_STEPS_PER_MM
#    define DEFAULT_X_STEPS_PER_MM 100.0
#endif
#ifndef DEFAULT_Y_STEPS_PER_MM
#    define DEFAULT_Y_STEPS_PER_MM 100.0
#endif
#ifndef DEFAULT_Z_STEPS_PER_MM
#    define DEFAULT_Z_STEPS_PER_MM 100.0
#endif
#ifndef DEFAULT_A_STEPS_PER_MM
#    define DEFAULT_A_STEPS_PER_MM 100.0
#endif
#ifndef DEFAULT_B_STEPS_PER_MM
#    define DEFAULT_B_STEPS_PER_MM 100.0
#endif
#ifndef DEFAULT_C_STEPS_PER_MM
#    define DEFAULT_C_STEPS_PER_MM 100.0
#endif

// ============ AXIS MAX SPPED =========

#ifndef DEFAULT_X_MAX_RATE
#    define DEFAULT_X_MAX_RATE 1000.0  // mm/min
#endif
#ifndef DEFAULT_Y_MAX_RATE
#    define DEFAULT_Y_MAX_RATE 1000.0  // mm/min
#endif
#ifndef DEFAULT_Z_MAX_RATE
#    define DEFAULT_Z_MAX_RATE 1000.0  // mm/min
#endif
#ifndef DEFAULT_A_MAX_RATE
#    define DEFAULT_A_MAX_RATE 1000.0  // mm/min
#endif
#ifndef DEFAULT_B_MAX_RATE
#    define DEFAULT_B_MAX_RATE 1000.0  // mm/min
#endif
#ifndef DEFAULT_C_MAX_RATE
#    define DEFAULT_C_MAX_RATE 1000.0  // mm/min
#endif

// ============== Axis Acceleration =========
#define SEC_PER_MIN_SQ (60.0 * 60.0)  // Seconds Per Minute Squared, for acceleration conversion
// Default accelerations are expressed in mm/sec^2
#ifndef DEFAULT_X_ACCELERATION
#    define DEFAULT_X_ACCELERATION 200.0
#endif
#ifndef DEFAULT_Y_ACCELERATION
#    define DEFAULT_Y_ACCELERATION 200.0
#endif
#ifndef DEFAULT_Z_ACCELERATION
#    define DEFAULT_Z_ACCELERATION 200.0
#endif
#ifndef DEFAULT_A_ACCELERATION
#    define DEFAULT_A_ACCELERATION 200.0
#endif
#ifndef DEFAULT_B_ACCELERATION
#    define DEFAULT_B_ACCELERATION 200.0
#endif
#ifndef DEFAULT_C_ACCELERATION
#    define DEFAULT_C_ACCELERATION 200.0
#endif

// ========= AXIS MAX TRAVEL ============

#ifndef DEFAULT_X_MAX_TRAVEL
#    define DEFAULT_X_MAX_TRAVEL 300.0  // $130 mm NOTE: Must be a positive value.
#endif
#ifndef DEFAULT_Y_MAX_TRAVEL
#    define DEFAULT_Y_MAX_TRAVEL 300.0  // mm NOTE: Must be a positive value.
#endif
#ifndef DEFAULT_Z_MAX_TRAVEL
#    define DEFAULT_Z_MAX_TRAVEL 300.0  // mm NOTE: Must be a positive value.
#endif
#ifndef DEFAULT_A_MAX_TRAVEL
#    define DEFAULT_A_MAX_TRAVEL 300.0  // mm NOTE: Must be a positive value.
#endif
#ifndef DEFAULT_B_MAX_TRAVEL
#    define DEFAULT_B_MAX_TRAVEL 300.0  // mm NOTE: Must be a positive value.
#endif
#ifndef DEFAULT_C_MAX_TRAVEL
#    define DEFAULT_C_MAX_TRAVEL 300.0  // mm NOTE: Must be a positive value.
#endif

#ifndef DEFAULT_X_HOMING_MPOS
#    define DEFAULT_X_HOMING_MPOS 0.0
#endif
#ifndef DEFAULT_Y_HOMING_MPOS
#    define DEFAULT_Y_HOMING_MPOS 0.0
#endif
#ifndef DEFAULT_Z_HOMING_MPOS
#    define DEFAULT_Z_HOMING_MPOS 0.0
#endif
#ifndef DEFAULT_A_HOMING_MPOS
#    define DEFAULT_A_HOMING_MPOS 0.0
#endif
#ifndef DEFAULT_B_HOMING_MPOS
#    define DEFAULT_B_HOMING_MPOS 0.0
#endif
#ifndef DEFAULT_C_HOMING_MPOS
#    define DEFAULT_C_HOMING_MPOS 0.0
#endif

#ifndef DEFAULT_HOMING_CYCLE_0
#    define DEFAULT_HOMING_CYCLE_0 bit(Z_AXIS)
#endif

#ifndef DEFAULT_HOMING_CYCLE_1
#    define DEFAULT_HOMING_CYCLE_1 (bit(X_AXIS) | bit(Y_AXIS))
#endif

#ifndef DEFAULT_HOMING_CYCLE_2
#    define DEFAULT_HOMING_CYCLE_2 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_3
#    define DEFAULT_HOMING_CYCLE_3 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_4
#    define DEFAULT_HOMING_CYCLE_4 0
#endif

#ifndef DEFAULT_HOMING_CYCLE_5
#    define DEFAULT_HOMING_CYCLE_5 0
#endif

// ========== Motor current (SPI Drivers ) =============
#ifndef DEFAULT_X_CURRENT
#    define DEFAULT_X_CURRENT 0.25  // $140 current in amps (extended set)
#endif
#ifndef DEFAULT_Y_CURRENT
#    define DEFAULT_Y_CURRENT 0.25  // $141 current in amps (extended set)
#endif
#ifndef DEFAULT_Z_CURRENT
#    define DEFAULT_Z_CURRENT 0.25  // $142 current in amps (extended set)
#endif
#ifndef DEFAULT_A_CURRENT
#    define DEFAULT_A_CURRENT 0.25  // $143 current in amps (extended set)
#endif
#ifndef DEFAULT_B_CURRENT
#    define DEFAULT_B_CURRENT 0.25  // $144 current in amps (extended set)
#endif
#ifndef DEFAULT_C_CURRENT
#    define DEFAULT_C_CURRENT 0.25  // $145 current in amps (extended set)
#endif

// ========== Motor hold current (SPI Drivers ) =============

#ifndef DEFAULT_X_HOLD_CURRENT
#    define DEFAULT_X_HOLD_CURRENT 0.125  // $150 current in amps (extended set)
#endif
#ifndef DEFAULT_Y_HOLD_CURRENT
#    define DEFAULT_Y_HOLD_CURRENT 0.125  // $151 current in amps (extended set)
#endif
#ifndef DEFAULT_Z_HOLD_CURRENT
#    define DEFAULT_Z_HOLD_CURRENT 0.125  // $152 current in amps (extended set)
#endif
#ifndef DEFAULT_A_HOLD_CURRENT
#    define DEFAULT_A_HOLD_CURRENT 0.125  // $153 current in amps (extended set)
#endif
#ifndef DEFAULT_B_HOLD_CURRENT
#    define DEFAULT_B_HOLD_CURRENT 0.125  // $154 current in amps (extended set)
#endif
#ifndef DEFAULT_C_HOLD_CURRENT
#    define DEFAULT_C_HOLD_CURRENT 0.125  // $154 current in amps (extended set)
#endif

// ========== Microsteps (SPI Drivers ) ================

#ifndef DEFAULT_X_MICROSTEPS
#    define DEFAULT_X_MICROSTEPS 16  // $160 micro steps (extended set)
#endif
#ifndef DEFAULT_Y_MICROSTEPS
#    define DEFAULT_Y_MICROSTEPS 16  // $161 micro steps (extended set)
#endif
#ifndef DEFAULT_Z_MICROSTEPS
#    define DEFAULT_Z_MICROSTEPS 16  // $162 micro steps (extended set)
#endif
#ifndef DEFAULT_A_MICROSTEPS
#    define DEFAULT_A_MICROSTEPS 16  // $163 micro steps (extended set)
#endif
#ifndef DEFAULT_B_MICROSTEPS
#    define DEFAULT_B_MICROSTEPS 16  // $164 micro steps (extended set)
#endif
#ifndef DEFAULT_C_MICROSTEPS
#    define DEFAULT_C_MICROSTEPS 16  // $165 micro steps (extended set)
#endif

// ========== Stallguard (SPI Drivers ) ================

#ifndef DEFAULT_X_STALLGUARD
#    define DEFAULT_X_STALLGUARD 16  // $170 stallguard (extended set)
#endif
#ifndef DEFAULT_Y_STALLGUARD
#    define DEFAULT_Y_STALLGUARD 16  // $171 stallguard (extended set)
#endif
#ifndef DEFAULT_Z_STALLGUARD
#    define DEFAULT_Z_STALLGUARD 16  // $172 stallguard (extended set)
#endif
#ifndef DEFAULT_A_STALLGUARD
#    define DEFAULT_A_STALLGUARD 16  // $173 stallguard (extended set)
#endif
#ifndef DEFAULT_B_STALLGUARD
#    define DEFAULT_B_STALLGUARD 16  // $174 stallguard (extended set)
#endif
#ifndef DEFAULT_C_STALLGUARD
#    define DEFAULT_C_STALLGUARD 16  // $175 stallguard (extended set)
#endif

// ==================  pin defaults ========================

// Here is a place to default pins to UNDEFINED_PIN.
// This can eliminate checking to see if the pin is defined because
// the overridden pinMode and digitalWrite functions will deal with it.

#ifndef SDCARD_DET_PIN
#    define SDCARD_DET_PIN UNDEFINED_PIN
#endif

#ifndef STEPPERS_DISABLE_PIN
#    define STEPPERS_DISABLE_PIN UNDEFINED_PIN
#endif

#ifndef X_DISABLE_PIN
#    define X_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef Y_DISABLE_PIN
#    define Y_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef Z_DISABLE_PIN
#    define Z_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef A_DISABLE_PIN
#    define A_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef B_DISABLE_PIN
#    define B_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef C_DISABLE_PIN
#    define C_DISABLE_PIN UNDEFINED_PIN
#endif

#ifndef X2_DISABLE_PIN
#    define X2_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef Y2_DISABLE_PIN
#    define Y2_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef Z2_DISABLE_PIN
#    define Z2_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef A2_DISABLE_PIN
#    define A2_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef B2_DISABLE_PIN
#    define B2_DISABLE_PIN UNDEFINED_PIN
#endif
#ifndef C2_DISABLE_PIN
#    define C2_DISABLE_PIN UNDEFINED_PIN
#endif

#ifndef X_LIMIT_PIN
#    define X_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef Y_LIMIT_PIN
#    define Y_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef Z_LIMIT_PIN
#    define Z_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef A_LIMIT_PIN
#    define A_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef B_LIMIT_PIN
#    define B_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef C_LIMIT_PIN
#    define C_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef X2_LIMIT_PIN
#    define X2_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef Y2_LIMIT_PIN
#    define Y2_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef Z2_LIMIT_PIN
#    define Z2_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef A2_LIMIT_PIN
#    define A2_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef B2_LIMIT_PIN
#    define B2_LIMIT_PIN UNDEFINED_PIN
#endif
#ifndef C2_LIMIT_PIN
#    define C2_LIMIT_PIN UNDEFINED_PIN
#endif

// assigned all MS3 (microstep pin 3) to UNDEFINED_PIN

#ifndef X_STEPPER_MS3
#    define X_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef Y_STEPPER_MS3
#    define Y_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef Z_STEPPER_MS3
#    define Z_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef A_STEPPER_MS3
#    define A_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef B_STEPPER_MS3
#    define B_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef C_STEPPER_MS3
#    define C_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef X2_STEPPER_MS3
#    define X2_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef Y2_STEPPER_MS3
#    define Y2_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef Z2_STEPPER_MS3
#    define Z2_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef A2_STEPPER_MS3
#    define A2_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef B2_STEPPER_MS3
#    define B2_STEPPER_MS3 UNDEFINED_PIN
#endif
#ifndef C2_STEPPER_MS3
#    define C2_STEPPER_MS3 UNDEFINED_PIN
#endif

#ifndef SERVO_TIMER_INTERVAL
#    define SERVO_TIMER_INTERVAL 75.0  // Hz This is the update inveral in milliseconds
#endif

#ifndef DYNAMIXEL_TXD
#    define DYNAMIXEL_TXD UNDEFINED_PIN
#endif

#ifndef DYNAMIXEL_RXD
#    define DYNAMIXEL_RXD UNDEFINED_PIN
#endif

#ifndef DYNAMIXEL_RTS
#    define DYNAMIXEL_RTS UNDEFINED_PIN
#endif

// ================ User Digital I/O ==============================

#ifndef USER_DIGITAL_PIN_0
#    define USER_DIGITAL_PIN_0 UNDEFINED_PIN
#endif

#ifndef USER_DIGITAL_PIN_1
#    define USER_DIGITAL_PIN_1 UNDEFINED_PIN
#endif

#ifndef USER_DIGITAL_PIN_2
#    define USER_DIGITAL_PIN_2 UNDEFINED_PIN
#endif

#ifndef USER_DIGITAL_PIN_3
#    define USER_DIGITAL_PIN_3 UNDEFINED_PIN
#endif

// ================ User Analog I/O ==============================

#ifndef USER_ANALOG_PIN_0
#    define USER_ANALOG_PIN_0 UNDEFINED_PIN
#endif

#ifndef USER_ANALOG_PIN_1
#    define USER_ANALOG_PIN_1 UNDEFINED_PIN
#endif

#ifndef USER_ANALOG_PIN_2
#    define USER_ANALOG_PIN_2 UNDEFINED_PIN
#endif

#ifndef USER_ANALOG_PIN_3
#    define USER_ANALOG_PIN_3 UNDEFINED_PIN
#endif

#ifndef PROBE_PIN
#    define PROBE_PIN UNDEFINED_PIN
#endif

#ifndef USER_ANALOG_PIN_0_FREQ
#    define USER_ANALOG_PIN_0_FREQ 5000
#endif
#ifndef USER_ANALOG_PIN_1_FREQ
#    define USER_ANALOG_PIN_1_FREQ 5000
#endif
#ifndef USER_ANALOG_PIN_2_FREQ
#    define USER_ANALOG_PIN_2_FREQ 5000
#endif
#ifndef USER_ANALOG_PIN_3_FREQ
#    define USER_ANALOG_PIN_3_FREQ 5000
#endif

#ifndef DEFAULT_USER_MACRO0
#    define DEFAULT_USER_MACRO0 ""
#endif

#ifndef DEFAULT_USER_MACRO1
#    define DEFAULT_USER_MACRO1 ""
#endif

#ifndef DEFAULT_USER_MACRO2
#    define DEFAULT_USER_MACRO2 ""
#endif

#ifndef DEFAULT_USER_MACRO3
#    define DEFAULT_USER_MACRO3 ""
#endif

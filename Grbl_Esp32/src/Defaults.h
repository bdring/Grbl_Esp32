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
#ifndef DEFAULT_STATUS_REPORT_MASK
#    define DEFAULT_STATUS_REPORT_MASK 1  // $10
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

// ============== Axis Acceleration =========
#define SEC_PER_MIN_SQ (60.0 * 60.0)  // Seconds Per Minute Squared, for acceleration conversion

// ==================  pin defaults ========================

// Here is a place to default pins to UNDEFINED_PIN.
// This can eliminate checking to see if the pin is defined because
// the overridden pinMode and digitalWrite functions will deal with it.

#ifndef SDCARD_DET_PIN
#    define SDCARD_DET_PIN UNDEFINED_PIN
#endif

#ifndef SERVO_TIMER_INTERVAL
#    define SERVO_TIMER_INTERVAL 75.0  // Hz This is the update inveral in milliseconds
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

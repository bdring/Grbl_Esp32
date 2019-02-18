/*
  defaults.h - defaults settings configuration file
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

#ifndef defaults_h


#ifdef DEFAULTS_GENERIC
  // Grbl generic default settings. Should work across different machines.
  #define DEFAULT_STEP_PULSE_MICROSECONDS 3 
  #define DEFAULT_STEPPER_IDLE_LOCK_TIME 250 // msec (0-254, 255 keeps steppers enabled)
  
  #define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
  #define DEFAULT_DIRECTION_INVERT_MASK 0 // uint8_t
   #define DEFAULT_INVERT_ST_ENABLE 0 // boolean
  #define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
  #define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
  
  #define DEFAULT_STATUS_REPORT_MASK 2 // MPos enabled
  
  #define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
  #define DEFAULT_ARC_TOLERANCE 0.002 // mm
  #define DEFAULT_REPORT_INCHES 0 // false
  
  #define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
  #define DEFAULT_HARD_LIMIT_ENABLE 0  // false
  
  #define DEFAULT_HOMING_ENABLE 0  // false
  #define DEFAULT_HOMING_DIR_MASK 3 // move positive dir Z, negative X,Y
  #define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
  #define DEFAULT_HOMING_SEEK_RATE 2000.0 // mm/min
  #define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
  #define DEFAULT_HOMING_PULLOFF 1.0 // mm

  #define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
  #define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
  
  #define DEFAULT_LASER_MODE 0 // false
  
  #define DEFAULT_X_STEPS_PER_MM 800.0
  #define DEFAULT_Y_STEPS_PER_MM 800.0
  #define DEFAULT_Z_STEPS_PER_MM 800.0
  
  #define DEFAULT_X_MAX_RATE 5000.0 // mm/min
  #define DEFAULT_Y_MAX_RATE 4000.0 // mm/min
  #define DEFAULT_Z_MAX_RATE 3000.0 // mm/min
  
  #define DEFAULT_X_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  #define DEFAULT_Y_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  #define DEFAULT_Z_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  
  #define DEFAULT_X_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
  #define DEFAULT_Y_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
  #define DEFAULT_Z_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
  

#endif

#ifdef DEFAULTS_PEN_LASER
	// Grbl generic default settings. Should work across different machines.
  #define DEFAULT_STEP_PULSE_MICROSECONDS 3 
  #define DEFAULT_STEPPER_IDLE_LOCK_TIME 250 // msec (0-254, 255 keeps steppers enabled)
  
  #define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
  #define DEFAULT_DIRECTION_INVERT_MASK 0 // uint8_t
   #define DEFAULT_INVERT_ST_ENABLE 0 // boolean
  #define DEFAULT_INVERT_LIMIT_PINS 0 // boolean
  #define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
  
  #define DEFAULT_STATUS_REPORT_MASK 2 // MPos enabled
  
  #define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
  #define DEFAULT_ARC_TOLERANCE 0.002 // mm
  #define DEFAULT_REPORT_INCHES 0 // false
  
  #define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
  #define DEFAULT_HARD_LIMIT_ENABLE 0  // false
  
  #define DEFAULT_HOMING_ENABLE 0  // false
  #define DEFAULT_HOMING_DIR_MASK 3 // move positive dir Z, negative X,Y
  #define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
  #define DEFAULT_HOMING_SEEK_RATE 2000.0 // mm/min
  #define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
  #define DEFAULT_HOMING_PULLOFF 1.0 // mm

  #define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
  #define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
  
  #define DEFAULT_LASER_MODE 0 // false
  
  #define DEFAULT_X_STEPS_PER_MM 80.0
  #define DEFAULT_Y_STEPS_PER_MM 80.0
  #define DEFAULT_Z_STEPS_PER_MM 80.0
  
  #define DEFAULT_X_MAX_RATE 12000.0 // mm/min
  #define DEFAULT_Y_MAX_RATE 12000.0 // mm/min
  #define DEFAULT_Z_MAX_RATE 5000.0 // mm/min
  
  #define DEFAULT_X_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  #define DEFAULT_Y_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  #define DEFAULT_Z_ACCELERATION (50.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  
  #define DEFAULT_X_MAX_TRAVEL 100.0 // mm NOTE: Must be a positive value.
  #define DEFAULT_Y_MAX_TRAVEL 100.0 // mm NOTE: Must be a positive value.
  #define DEFAULT_Z_MAX_TRAVEL 10.0 // mm NOTE: Must be a positive value.
#endif

#ifdef DEFAULTS_MIDTBOT
	// Grbl generic default settings. Should work across different machines.
  #define DEFAULT_STEP_PULSE_MICROSECONDS 3 
  #define DEFAULT_STEPPER_IDLE_LOCK_TIME 250 // msec (0-254, 255 keeps steppers enabled)
  
  #define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
  #define DEFAULT_DIRECTION_INVERT_MASK 0 // uint8_t
   #define DEFAULT_INVERT_ST_ENABLE 0 // boolean
  #define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
  #define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
  
  #define DEFAULT_STATUS_REPORT_MASK 2 // MPos enabled
  
  #define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
  #define DEFAULT_ARC_TOLERANCE 0.002 // mm
  #define DEFAULT_REPORT_INCHES 0 // false
  
  #define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
  #define DEFAULT_HARD_LIMIT_ENABLE 0  // false
  
  #define DEFAULT_HOMING_ENABLE 1  // false
  #define DEFAULT_HOMING_DIR_MASK 1 // move neg in X, pos in Y
  #define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
  #define DEFAULT_HOMING_SEEK_RATE 3000.0 // mm/min
  #define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
  #define DEFAULT_HOMING_PULLOFF 1.0 // mm

  #define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
  #define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
  
  #define DEFAULT_LASER_MODE 0 // false
  
  #define DEFAULT_X_STEPS_PER_MM 400.0
  #define DEFAULT_Y_STEPS_PER_MM 200.0
  #define DEFAULT_Z_STEPS_PER_MM 100.0 // DEFAULT CAL VALUE !!
  
  #define DEFAULT_X_MAX_RATE 10000.0 // mm/min
  #define DEFAULT_Y_MAX_RATE 10000.0 // mm/min
  #define DEFAULT_Z_MAX_RATE 10000.0 // mm/min
  
  #define DEFAULT_X_ACCELERATION (400.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  #define DEFAULT_Y_ACCELERATION (400.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  #define DEFAULT_Z_ACCELERATION (400.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
  
  #define DEFAULT_X_MAX_TRAVEL 90.0 // mm NOTE: Must be a positive value.
  #define DEFAULT_Y_MAX_TRAVEL 90.0 // mm NOTE: Must be a positive value.
  #define DEFAULT_Z_MAX_TRAVEL 100.0 // DEFAULT CAL VALUE !!
#endif

// TODO: Calculate and verify all values
#ifdef DEFAULTS_WALL_PLOTTER
  // Settings only for custom wall plotter
  #define DEFAULT_X_STEPS_PER_MM 1000.0 // TODO: Calculate DEFAULT_X_STEPS_PER_MM
  #define DEFAULT_Y_STEPS_PER_MM 1000.0 // TODO: Calculate DEFAULT_Y_STEPS_PER_MM
  #define DEFAULT_Z_STEPS_PER_MM 1000.0 // mm - not used
  #define DEFAULT_X_MAX_RATE 6000.0 // mm/min
  #define DEFAULT_Y_MAX_RATE 6000.0 // mm/min
  #define DEFAULT_Z_MAX_RATE 6000.0 // mm/min - not used
  #define DEFAULT_X_ACCELERATION (250.0*60*60) // 250*60*60 mm/min^2 = 250 mm/sec^2
  #define DEFAULT_Y_ACCELERATION (250.0*60*60) // 250*60*60 mm/min^2 = 250 mm/sec^2
  #define DEFAULT_Z_ACCELERATION (250.0*60*60) // 250*60*60 mm/min^2 = 250 mm/sec^2 - not used
  #define DEFAULT_X_MAX_TRAVEL 1000.0 // mm
  #define DEFAULT_Y_MAX_TRAVEL 1000.0 // mm
  #define DEFAULT_Z_MAX_TRAVEL 1000.0 // mm - not used
  #define DEFAULT_SPINDLE_RPM_MAX 0.0 // rpm - not used
  #define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm - not used
  #define DEFAULT_STEP_PULSE_MICROSECONDS 10
  #define DEFAULT_STEPPING_INVERT_MASK 0
  #define DEFAULT_DIRECTION_INVERT_MASK 0
  #define DEFAULT_STEPPER_IDLE_LOCK_TIME 25 // msec (0-254, 255 keeps steppers enabled)
  #define DEFAULT_SOFT_LIMIT_ENABLE 0 // true
  #define DEFAULT_HARD_LIMIT_ENABLE 0  // false
  #define DEFAULT_INVERT_PROBE_PIN 0 // false
  #define DEFAULT_LASER_MODE 0 // false
  #define DEFAULT_HOMING_ENABLE 0  // true
  #define DEFAULT_HOMING_DIR_MASK 0 // move positive dir
  #define DEFAULT_HOMING_FEED_RATE 500.0 // mm/min
  #define DEFAULT_HOMING_SEEK_RATE 2000.0 // mm/min
  #define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
  #define DEFAULT_HOMING_PULLOFF 1.0 // mm
#endif

#endif

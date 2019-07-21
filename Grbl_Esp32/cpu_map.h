/*
  cpu_map.h - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC
	
	2018 -	Bart Dring This file was modified for use on the ESP32
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

#ifndef cpu_map_h
//#define cpu_map_h 

  /*
	Not all pins can can work for all functions.
	Check features like pull-ups, pwm, etc before
  re-assigning numbers
	
	(gpio34-39) are inputs only and don't have software pullup/down functions
	You MUST use external pull-ups or noise WILL cause problems.
	
	Unlike the AVR version certain pins are not forced into the same port. 
	Therefore, bit masks are not use the same way and typically should not be 
	changed. They are just preserved right now to make it easy to stay in sync
	with AVR grbl
	
	*/
		

#ifdef CPU_MAP_ESP32
	// This is the CPU Map for the ESP32 CNC Controller R2	
	
	  // It is OK to comment out any step and direction pins. This
    // won't affect operation except that there will be no output
		// form the pins. Grbl will virtually move the axis. This could 
		// be handy if you are using a servo, etc. for another axis.
		#define CPU_MAP_NAME "CPU_MAP_ESP32"	
		
		#define X_STEP_PIN      	GPIO_NUM_12
		#define X_DIRECTION_PIN  	GPIO_NUM_26
		#define X_RMT_CHANNEL		0
		
		#define Y_STEP_PIN      	GPIO_NUM_14
		#define Y_DIRECTION_PIN   	GPIO_NUM_25
		#define Y_RMT_CHANNEL		1
		
		#define Z_STEP_PIN      	GPIO_NUM_27		
		#define Z_DIRECTION_PIN   	GPIO_NUM_33 		 
		#define Z_RMT_CHANNEL		2		
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
		// *** the flood coolant feature code is activated by defining this pins
		// *** Comment it out to use the pin for other features
		#define COOLANT_FLOOD_PIN 	GPIO_NUM_16			
		//#define COOLANT_MIST_PIN   	GPIO_NUM_21
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		#define SPINDLE_PWM_PIN    GPIO_NUM_17 
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_ENABLE_PIN	GPIO_NUM_22
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)		
		
		// if these spindle function pins are defined, they will be activated in the code
		// comment them out to use the pins for other functions
		//#define SPINDLE_ENABLE_PIN	GPIO_NUM_16
		//#define SPINDLE_DIR_PIN			GPIO_NUM_16		
		
		#define X_LIMIT_PIN      	GPIO_NUM_2  
		#define Y_LIMIT_PIN      	GPIO_NUM_4  
		#define Z_LIMIT_PIN     	GPIO_NUM_15 	
		#define LIMIT_MASK      	B111
		
		#define PROBE_PIN       	GPIO_NUM_32  
		
		#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
		#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup    		
		
#endif

#ifdef CPU_MAP_ESP32_ESC_SPINDLE
	// This is the CPU Map for the ESP32 CNC Controller R2	
	
	  // It is OK to comment out any step and direction pins. This
    // won't affect operation except that there will be no output
		// form the pins. Grbl will virtually move the axis. This could 
		// be handy if you are using a servo, etc. for another axis.
		#define CPU_MAP_NAME "CPU_MAP_ESP32_ESC_SPINDLE"	
		
		#define X_STEP_PIN      	GPIO_NUM_12
		#define X_DIRECTION_PIN  	GPIO_NUM_26
		#define X_RMT_CHANNEL		0
		
		#define Y_STEP_PIN      	GPIO_NUM_14
		#define Y_DIRECTION_PIN   	GPIO_NUM_25
		#define Y_RMT_CHANNEL		1
		
		#define Z_STEP_PIN      	GPIO_NUM_27		
		#define Z_DIRECTION_PIN   	GPIO_NUM_33 		 
		#define Z_RMT_CHANNEL		2		
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
		// *** the flood coolant feature code is activated by defining this pins
		// *** Comment it out to use the pin for other features
		#define COOLANT_FLOOD_PIN 	GPIO_NUM_16			
		//#define COOLANT_MIST_PIN   	GPIO_NUM_21
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		#define SPINDLE_PWM_PIN    GPIO_NUM_17 
		#define SPINDLE_PWM_CHANNEL 0
		
		// RC ESC Based Spindle 
		// An ESC works like a hobby servo with 50Hz PWM 1ms to 2 ms pulse range		
		#define SPINDLE_PWM_BASE_FREQ 50 // Hz for ESC
		#define SPINDLE_PWM_BIT_PRECISION 16   // 16 bit required for ESC
		#define SPINDLE_PULSE_RES_COUNT 65535
		
		#define ESC_MIN_PULSE_SEC 0.001 // min pulse in seconds (OK to tune this one)
		#define ESC_MAX_PULSE_SEC 0.002 // max pulse in seconds (OK to tune this one)		
		#define ESC_TIME_PER_BIT  ((1.0 / (float)SPINDLE_PWM_BASE_FREQ) / ((float)SPINDLE_PULSE_RES_COUNT) ) // seconds

		#define SPINDLE_PWM_OFF_VALUE    (uint16_t)(ESC_MIN_PULSE_SEC / ESC_TIME_PER_BIT) // in timer counts
		#define SPINDLE_PWM_MAX_VALUE    (uint16_t)(ESC_MAX_PULSE_SEC / ESC_TIME_PER_BIT) // in timer counts
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   SPINDLE_PWM_OFF_VALUE   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)
		
		#define SPINDLE_ENABLE_PIN	GPIO_NUM_22			
		
		// if these spindle function pins are defined, they will be activated in the code
		// comment them out to use the pins for other functions
		//#define SPINDLE_ENABLE_PIN	GPIO_NUM_16
		//#define SPINDLE_DIR_PIN			GPIO_NUM_16		
		
		#define X_LIMIT_PIN      	GPIO_NUM_2  
		#define Y_LIMIT_PIN      	GPIO_NUM_4  
		#define Z_LIMIT_PIN     	GPIO_NUM_15 	
		#define LIMIT_MASK      	B111
		
		#define PROBE_PIN       	GPIO_NUM_32  
		
		#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
		#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup    		
		
#endif		
		
#ifdef CPU_MAP_PEN_LASER  // The Buildlog.net pen laser controller V1

		// For pen mode be sure to uncomment #define USE_PEN_SERVO in config.h
		// For solenoid  mode be sure to uncomment #define USE_PEN_SERVO in config.h
		// For laser mode, you do not need to change anything
		// Note: You can use all 3 modes at the same time if you want
		
		#define CPU_MAP_NAME "CPU_MAP_PEN_LASER"
		
		#define USE_RMT_STEPS
		
		// Pick a board version
		//#define PEN_LASER_V1
		#define PEN_LASER_V2

		#define X_STEP_PIN      	GPIO_NUM_12
		#define X_DIRECTION_PIN   	GPIO_NUM_26
		#define X_RMT_CHANNEL		0
		
		
		#define Y_STEP_PIN      	GPIO_NUM_14
		#define Y_DIRECTION_PIN   	GPIO_NUM_25
		#define Y_RMT_CHANNEL		1
		
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13
		
		#ifdef PEN_LASER_V1
			#define X_LIMIT_PIN      	GPIO_NUM_2
		#endif
		#ifdef PEN_LASER_V2
			#define X_LIMIT_PIN      	GPIO_NUM_15
		#endif		
		#define Y_LIMIT_PIN      	GPIO_NUM_4
		#define LIMIT_MASK      	B11				
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		#define SPINDLE_PWM_PIN    GPIO_NUM_17 // Laser PWM
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)			
		
		#define USING_SERVO  // uncommewnt to use this feature
		#define USING_SOLENOID // uncommewnt to use this feature
		
		#ifdef USING_SERVO
			#define USE_SERVO_AXES 
			#define SERVO_Z_PIN 	GPIO_NUM_27
			#define SERVO_Z_CHANNEL_NUM 3
			#define SERVO_Z_RANGE_MIN 0
			#define SERVO_Z_RANGE_MAX 10
		#endif
		
		#ifdef USING_SOLENOID
			#define USE_PEN_SOLENOID
			#define SOLENOID_PEN_PIN GPIO_NUM_16
			#define SOLENOID_CHANNEL_NUM 6
		#endif
		
		
		
		#ifdef DEFAULTS_GENERIC 
			#undef DEFAULTS_GENERIC  // undefine generic then define each default below
		#endif
		
		// defaults
		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
		#define DEFAULT_STEPPER_IDLE_LOCK_TIME 250 // stay on
		
		#define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
		#define DEFAULT_DIRECTION_INVERT_MASK 0 // uint8_t
		#define DEFAULT_INVERT_ST_ENABLE 0 // boolean
		#define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
		#define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
		
		#define DEFAULT_STATUS_REPORT_MASK 1
		
		#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
		#define DEFAULT_ARC_TOLERANCE 0.002 // mm
		#define DEFAULT_REPORT_INCHES 0 // false
		
		#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
		#define DEFAULT_HARD_LIMIT_ENABLE 0  // false
		
		#define DEFAULT_HOMING_ENABLE 0  
		#define DEFAULT_HOMING_DIR_MASK 0 // move positive dir Z, negative X,Y
		#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
		#define DEFAULT_HOMING_SEEK_RATE 1000.0 // mm/min
		#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
		#define DEFAULT_HOMING_PULLOFF 3.0 // mm

		#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
		#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
		
		#define DEFAULT_LASER_MODE 0 // false
		
		#define DEFAULT_X_STEPS_PER_MM 80
		#define DEFAULT_Y_STEPS_PER_MM 80
		#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode...used for calibration
		
		#define DEFAULT_X_MAX_RATE 5000.0 // mm/min
		#define DEFAULT_Y_MAX_RATE 5000.0 // mm/min
		#define DEFAULT_Z_MAX_RATE 5000.0 // mm/min
		
		#define DEFAULT_X_ACCELERATION (50.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Y_ACCELERATION (50.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Z_ACCELERATION (50.0*60*60) 
		
		#define DEFAULT_X_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Y_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Z_MAX_TRAVEL 100.0 // This is percent in servo mode...used for calibration
		
		
#endif

#ifdef CPU_MAP_MIDTBOT  // Buildlog.net midtbot

	#define CPU_MAP_NAME "CPU_MAP_MIDTBOT"

		#define X_STEP_PIN      GPIO_NUM_12
		#define Y_STEP_PIN      GPIO_NUM_14		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		#define Y_DIRECTION_PIN   GPIO_NUM_25  
		
		#ifndef COREXY // maybe set in config.h
			#define COREXY
		#endif
		
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13
		
		#define X_LIMIT_PIN      	GPIO_NUM_2  
		#define Y_LIMIT_PIN      	GPIO_NUM_4
		#define LIMIT_MASK      	B11		
		
		#ifndef USE_SERVO_AXES  // maybe set in config.h
			#define USE_SERVO_AXES
		#endif
		
		#define SERVO_Z_PIN 					GPIO_NUM_27
		#define SERVO_Z_CHANNEL_NUM 		5
		#define SERVO_Z_RANGE_MIN			0.0
		#define SERVO_Z_RANGE_MAX			5.0
		
		#ifndef IGNORE_CONTROL_PINS // maybe set in config.h
			#define IGNORE_CONTROL_PINS
		#endif
		//#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
		//#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		//#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		//#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup		
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		//#define SPINDLE_PWM_PIN    GPIO_NUM_17 // Laser PWM
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		// redefine some stuff from config.h
		#define HOMING_CYCLE_0 (1<<Y_AXIS)
		#define HOMING_CYCLE_1 (1<<X_AXIS)
		#ifdef HOMING_CYCLE_2
			#undef HOMING_CYCLE_2
		#endif
		
		// redefine the defaults
		#ifdef DEFAULTS_GENERIC
			#undef DEFAULTS_GENERIC
		#endif		
		#define DEFAULTS_MIDTBOT		
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)	
		
		#define SERVO_PEN_PIN 					GPIO_NUM_27
		
		#ifdef DEFAULTS_GENERIC 
			#undef DEFAULTS_GENERIC  // undefine generic then define each default below
		#endif
		
		// defaults
		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
		#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay on
		
		#define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
		#define DEFAULT_DIRECTION_INVERT_MASK 2 // uint8_t
		#define DEFAULT_INVERT_ST_ENABLE 0 // boolean
		#define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
		#define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
		
		#define DEFAULT_STATUS_REPORT_MASK 1
		
		#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
		#define DEFAULT_ARC_TOLERANCE 0.002 // mm
		#define DEFAULT_REPORT_INCHES 0 // false
		
		#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
		#define DEFAULT_HARD_LIMIT_ENABLE 0  // false
		
		#define DEFAULT_HOMING_ENABLE 1  
		#define DEFAULT_HOMING_DIR_MASK 1 
		#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
		#define DEFAULT_HOMING_SEEK_RATE 1000.0 // mm/min
		#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
		#define DEFAULT_HOMING_PULLOFF 3.0 // mm

		#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
		#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
		
		#define DEFAULT_LASER_MODE 0 // false
		
		#define DEFAULT_X_STEPS_PER_MM 200.0
		#define DEFAULT_Y_STEPS_PER_MM 100.0
		#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode
		
		#define DEFAULT_X_MAX_RATE 8000.0 // mm/min
		#define DEFAULT_Y_MAX_RATE 8000.0 // mm/min
		#define DEFAULT_Z_MAX_RATE 5000.0 // mm/min
		
		#define DEFAULT_X_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Y_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Z_ACCELERATION (100.0*60*60) 
		
		#define DEFAULT_X_MAX_TRAVEL 100.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Y_MAX_TRAVEL 100.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Z_MAX_TRAVEL 100.0 // This is percent in servo mode		
		
#endif

#ifdef CPU_MAP_POLAR_COASTER  // The Buildlog.net pen polar coaster controller V1
		#define CPU_MAP_NAME "CPU_MAP_POLAR_COASTER"

		#define X_STEP_PIN      GPIO_NUM_15
		#define Y_STEP_PIN      GPIO_NUM_2
		#define X_DIRECTION_PIN   GPIO_NUM_25
		#define Y_DIRECTION_PIN   GPIO_NUM_26
		
		#define STEPPERS_DISABLE_PIN GPIO_NUM_17
		
		#define X_LIMIT_PIN      	GPIO_NUM_4
		#define LIMIT_MASK      	B1
		
		#define CONTROL_RESET_PIN         GPIO_NUM_13
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		//#define SPINDLE_PWM_PIN    GPIO_NUM_17
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)	
		
		#define USE_PEN_SERVO
		#define SERVO_PEN_PIN 					GPIO_NUM_16
		
		// redefine some stuff from config.h
		#define HOMING_CYCLE_0 (1<<X_AXIS) // this 'bot only homes the X axis
		#ifdef HOMING_CYCLE_1
			#undef HOMING_CYCLE_1
		#endif
		#ifdef HOMING_CYCLE_2
			#undef HOMING_CYCLE_2
		#endif		
		
		#ifdef DEFAULTS_GENERIC 
			#undef DEFAULTS_GENERIC  // undefine generic then define each default below
		#endif
		
		// defaults
		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
		#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay on
		
		#define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
		#define DEFAULT_DIRECTION_INVERT_MASK 2 // uint8_t
		#define DEFAULT_INVERT_ST_ENABLE 0 // boolean
		#define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
		#define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
		
		#define DEFAULT_STATUS_REPORT_MASK 1
		
		#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
		#define DEFAULT_ARC_TOLERANCE 0.002 // mm
		#define DEFAULT_REPORT_INCHES 0 // false
		
		#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
		#define DEFAULT_HARD_LIMIT_ENABLE 0  // false
		
		#define DEFAULT_HOMING_ENABLE 1  
		#define DEFAULT_HOMING_DIR_MASK 0 // move positive dir Z, negative X,Y
		#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
		#define DEFAULT_HOMING_SEEK_RATE 1000.0 // mm/min
		#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
		#define DEFAULT_HOMING_PULLOFF 3.0 // mm

		#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
		#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
		
		#define DEFAULT_LASER_MODE 0 // false
		
		#define DEFAULT_X_STEPS_PER_MM 200.0
		#define DEFAULT_Y_STEPS_PER_MM 71.111
		#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode
		
		#define DEFAULT_X_MAX_RATE 5000.0 // mm/min
		#define DEFAULT_Y_MAX_RATE 15000.0 // mm/min
		#define DEFAULT_Z_MAX_RATE 3000.0 // mm/min
		
		#define DEFAULT_X_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Y_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Z_ACCELERATION (50.0*60*60) 
		
		#define DEFAULT_X_MAX_TRAVEL 50.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Y_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Z_MAX_TRAVEL 100.0 // This is percent in servo mode
		
		
		
#endif

#ifdef CPU_MAP_SERVO_AXIS  // The Buildlog.net pen laser controller V1

		// For pen mode be sure to uncomment #define USE_PEN_SERVO in config.h
		// For solenoid  mode be sure to uncomment #define USE_PEN_SERVO in config.h
		// For laser mode, you do not need to change anything
		// Note: You can use all 3 modes at the same time if you want
		
		#define CPU_MAP_NAME "CPU_MAP_SERVO_AXIS"
		
		// Pick a board version
		//#define PEN_LASER_V1
		#define PEN_LASER_V2

		#define X_STEP_PIN      GPIO_NUM_12
		//#define Y_STEP_PIN      GPIO_NUM_14		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		//#define Y_DIRECTION_PIN   GPIO_NUM_25  
		
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13
		
		#ifdef PEN_LASER_V1
			#define X_LIMIT_PIN      	GPIO_NUM_2
		#endif
		#ifdef PEN_LASER_V2
			#define X_LIMIT_PIN      	GPIO_NUM_15
		#endif		
		#define Y_LIMIT_PIN      	GPIO_NUM_4
		#define LIMIT_MASK      	B11				
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		#define SPINDLE_PWM_PIN    GPIO_NUM_17 // Laser PWM
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)			
		
		#define SERVO_Y_PIN 					GPIO_NUM_14
		#define SERVO_Y_CHANNEL_NUM 	6
		#define SERVO_Y_RANGE_MIN			0.0
		#define SERVO_Y_RANGE_MAX			30.0
		
		#define SERVO_Z_PIN 					GPIO_NUM_27
		#define SERVO_Z_CHANNEL_NUM 	5
		#define SERVO_Z_RANGE_MIN			0.0
		#define SERVO_Z_RANGE_MAX			20.0
		
		
		#ifdef DEFAULTS_GENERIC 
			#undef DEFAULTS_GENERIC  // undefine generic then define each default below
		#endif
		
		// defaults
		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
		#define DEFAULT_STEPPER_IDLE_LOCK_TIME 250 // stay on
		
		#define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
		#define DEFAULT_DIRECTION_INVERT_MASK 0 // uint8_t
		#define DEFAULT_INVERT_ST_ENABLE 0 // boolean
		#define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
		#define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
		
		#define DEFAULT_STATUS_REPORT_MASK 1
		
		#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
		#define DEFAULT_ARC_TOLERANCE 0.002 // mm
		#define DEFAULT_REPORT_INCHES 0 // false
		
		#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
		#define DEFAULT_HARD_LIMIT_ENABLE 0  // false
		
		#define DEFAULT_HOMING_ENABLE 0  
		#define DEFAULT_HOMING_DIR_MASK 0 // move positive dir Z, negative X,Y
		#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
		#define DEFAULT_HOMING_SEEK_RATE 1000.0 // mm/min
		#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
		#define DEFAULT_HOMING_PULLOFF 3.0 // mm

		#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
		#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
		
		#define DEFAULT_LASER_MODE 0 // false
		
		#define DEFAULT_X_STEPS_PER_MM 40      // half turn on a stepper
		#define DEFAULT_Y_STEPS_PER_MM 100.0  // default calibration value
		#define DEFAULT_Z_STEPS_PER_MM 100.0 // default calibration value
		
		#define DEFAULT_X_MAX_RATE 2000.0 // mm/min
		#define DEFAULT_Y_MAX_RATE 2000.0 // mm/min
		#define DEFAULT_Z_MAX_RATE 2000.0 // mm/min
		
		#define DEFAULT_X_ACCELERATION (50.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Y_ACCELERATION (50.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Z_ACCELERATION (50.0*60*60) 
		
		#define DEFAULT_X_MAX_TRAVEL 300.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Y_MAX_TRAVEL 100.0 // default calibration value
		#define DEFAULT_Z_MAX_TRAVEL 100.0 // default calibration value
		
		
#endif

#ifdef CPU_MAP_SM // String art machine definition
	

		#define X_STEP_PIN      GPIO_NUM_12
		#define X_DIRECTION_PIN   GPIO_NUM_26
		
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
		#define X_LIMIT_PIN      	GPIO_NUM_15
		#define LIMIT_MASK      	B1			
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		#define SPINDLE_PWM_PIN    GPIO_NUM_17 // Laser PWM
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)			
		
		// define Y and Z as servos
		#define USE_SERVO_AXES
		#define SERVO_Y_PIN 					GPIO_NUM_27
		#define SERVO_Y_CHANNEL_NUM 	6
		#define SERVO_Y_RANGE_MIN			0.0
		#define SERVO_Y_RANGE_MAX			20.0
		
		#define SERVO_Z_PIN 					GPIO_NUM_14
		#define SERVO_Z_CHANNEL_NUM 	5
		#define SERVO_Z_RANGE_MIN			0.0
		#define SERVO_Z_RANGE_MAX			28.0		
		
		// define the control pins
		//#define CONTROL_RESET_PIN         GPIO_NUM_36  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_34  // needs external pullup
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup
		
		// change some things that may have been defined in config.h
		#ifndef IGNORE_CONTROL_PINS
			#define IGNORE_CONTROL_PINS
		#endif
		#ifdef INVERT_CONTROL_PIN_MASK
			#undef INVERT_CONTROL_PIN_MASK
		#endif
		#define INVERT_CONTROL_PIN_MASK   B1100
		

		// clear any default homings and set X as the only one
		#ifdef HOMING_CYCLE_0
			#undef HOMING_CYCLE_0
		#endif
		#define HOMING_CYCLE_0 (1<<X_AXIS) 
		#ifdef HOMING_CYCLE_1
			#undef HOMING_CYCLE_1
		#endif
		#ifdef HOMING_CYCLE_2
			#undef HOMING_CYCLE_2
		#endif		
				
		#ifdef DEFAULTS_GENERIC 
			#undef DEFAULTS_GENERIC  // undefine generic then define each default below
		#endif
		
		// defaults
		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
		#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay on
		
		#define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
		#define DEFAULT_DIRECTION_INVERT_MASK 6 // Y and Z need to be inverted
		#define DEFAULT_INVERT_ST_ENABLE 0 // boolean
		#define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
		#define DEFAULT_INVERT_PROBE_PIN 0 // boolean 
		
		#define DEFAULT_STATUS_REPORT_MASK 1
		
		#define DEFAULT_JUNCTION_DEVIATION 0.01 // mm
		#define DEFAULT_ARC_TOLERANCE 0.002 // mm
		#define DEFAULT_REPORT_INCHES 0 // false
		
		#define DEFAULT_SOFT_LIMIT_ENABLE 0 // false
		#define DEFAULT_HARD_LIMIT_ENABLE 0  // false
		
		#define DEFAULT_HOMING_ENABLE 1  
		#define DEFAULT_HOMING_DIR_MASK 0 // move positive dir Z, negative X,Y
		#define DEFAULT_HOMING_FEED_RATE 25.0 // mm/min
		#define DEFAULT_HOMING_SEEK_RATE 600.0 // mm/min
		#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
		#define DEFAULT_HOMING_PULLOFF 1.0 // mm

		#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
		#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
		
		#define DEFAULT_LASER_MODE 0 // false
		
		#define DEFAULT_X_STEPS_PER_MM 300.0
		#define DEFAULT_Y_STEPS_PER_MM 100.0   // This is percent in servo mode...used for calibration
		#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode...used for calibration
		
		#define DEFAULT_X_MAX_RATE 8000.0 // mm/min
		#define DEFAULT_Y_MAX_RATE 4000.0 // mm/min
		#define DEFAULT_Z_MAX_RATE 4000.0 // mm/min
		
		#define DEFAULT_X_ACCELERATION (150.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Y_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Z_ACCELERATION (200.0*60*60) 
		
		#define DEFAULT_X_MAX_TRAVEL 100000.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Y_MAX_TRAVEL 100.0 // This is percent in servo mode...used for calibration
		#define DEFAULT_Z_MAX_TRAVEL 100.0 // This is percent in servo mode...used for calibration
		
		
#endif

#ifdef CPU_MAP_MPCNC_V1P2
	// This is the CPU Map for the Buildlog.net MPCNC controller
	
		#define CPU_MAP_NAME "CPU_MAP_MPCNC_V1P2"
	
		// switch to the correct default settings
		#ifdef DEFAULTS_GENERIC
			#undef DEFAULTS_GENERIC
		#endif
		#define DEFAULTS_MPCNC
		
	
		#define USE_GANGED_AXES // allow two motors on an axis 
	  
		#define X_STEP_PIN      GPIO_NUM_12
		#define X_STEP_B_PIN    GPIO_NUM_22	 // ganged motor
		#define X_AXIS_SQUARING
		
		#define Y_STEP_PIN      GPIO_NUM_14
		#define Y_STEP_B_PIN    GPIO_NUM_21  // ganged motor
		#define Y_AXIS_SQUARING
		
		#define Z_STEP_PIN      GPIO_NUM_27
		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		#define Y_DIRECTION_PIN   GPIO_NUM_25  
		#define Z_DIRECTION_PIN   GPIO_NUM_33 
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13				
				
		// Note: if you use PWM rather than relay, you could map GPIO_NUM_2 to mist or flood 
		//#define USE_SPINDLE_RELAY
		
		#ifdef USE_SPINDLE_RELAY		
			#define SPINDLE_PWM_PIN    GPIO_NUM_2
		#else
			#define SPINDLE_PWM_PIN    GPIO_NUM_16
			#define SPINDLE_ENABLE_PIN	GPIO_NUM_32
		#endif
		
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)		
		
		// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
		// Relay can be used for Spindle or Coolant
		//#define COOLANT_FLOOD_PIN 	GPIO_NUM_2
		
		#define X_LIMIT_PIN      	GPIO_NUM_17 
		#define Y_LIMIT_PIN      	GPIO_NUM_4  
		#define Z_LIMIT_PIN     	GPIO_NUM_15 	
		#define LIMIT_MASK      	B111
		
		#ifndef ENABLE_SOFTWARE_DEBOUNCE   // V1P2 does not have R/C filters
			#define ENABLE_SOFTWARE_DEBOUNCE
		#endif
		
		#define PROBE_PIN       	GPIO_NUM_35  
		
		// The default value in config.h is wrong for this controller
		#ifdef INVERT_CONTROL_PIN_MASK
			#undef INVERT_CONTROL_PIN_MASK			
		#endif
		
		#define INVERT_CONTROL_PIN_MASK   B1110
		
		// Note: defualt is #define IGNORE_CONTROL_PINS in config.h
		// uncomment to these lines to use them		
		#ifdef IGNORE_CONTROL_PINS
			#undef IGNORE_CONTROL_PINS
		#endif
		
		
		
		
		#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup    		
		
#endif

#ifdef CPU_MAP_MPCNC_V1P1
	// This is the CPU Map for the Buildlog.net MPCNC controller
	
		#define CPU_MAP_NAME "CPU_MAP_MPCNC_V1P1"
	
		// switch to the correct default settings
		#ifdef DEFAULTS_GENERIC
			#undef DEFAULTS_GENERIC
		#endif
		#define DEFAULTS_MPCNC
		
	
		#define USE_GANGED_AXES // allow two motors on an axis 
	  
		#define X_STEP_PIN      GPIO_NUM_12
		#define X_STEP_B_PIN    GPIO_NUM_22	 // ganged motor
		#define X_AXIS_SQUARING
		
		#define Y_STEP_PIN      GPIO_NUM_14
		#define Y_STEP_B_PIN    GPIO_NUM_21  // ganged motor
		#define Y_AXIS_SQUARING
		
		#define Z_STEP_PIN      GPIO_NUM_27
		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		#define Y_DIRECTION_PIN   GPIO_NUM_25  
		#define Z_DIRECTION_PIN   GPIO_NUM_33 
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
				
		// Note: if you use PWM rather than relay, you could map GPIO_NUM_17 to mist or flood 
		#define USE_SPINDLE_RELAY
		
		#ifdef USE_SPINDLE_RELAY		
			#define SPINDLE_PWM_PIN    GPIO_NUM_17
		#else
			#define SPINDLE_PWM_PIN    GPIO_NUM_16
			#define SPINDLE_ENABLE_PIN	GPIO_NUM_32
		#endif
		
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)		
		
		// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
		// Relay can be used for Spindle or Coolant
		//#define COOLANT_FLOOD_PIN 	GPIO_NUM_17
		
		#define X_LIMIT_PIN      	GPIO_NUM_34 
		#define Y_LIMIT_PIN      	GPIO_NUM_4  
		#define Z_LIMIT_PIN     	GPIO_NUM_15 	
		#define LIMIT_MASK      	B111
		
		#define PROBE_PIN       	GPIO_NUM_35  
		
		// The default value in config.h is wrong for this controller
		#ifdef INVERT_CONTROL_PIN_MASK
			#undef INVERT_CONTROL_PIN_MASK			
		#endif
		
		#define INVERT_CONTROL_PIN_MASK   B1100
		
		// Note: check the #define IGNORE_CONTROL_PINS is the way you want in config.h
		//#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup    		
		
#endif

#ifdef CPU_MAP_LOWRIDER // !!!!!!!!!!!!!!!!! Warning: Untested !!!!!!!!!!!!!!!!! //
	// This is the CPU Map for the Buildlog.net MPCNC controller
	// used in lowrider mode. Low rider has (2) Y and Z and one X motor
	// These will not match the silkscreen or schematic descriptions	
		#define CPU_MAP_NAME "CPU_MAP_LOWRIDER"
	
	
		#define USE_GANGED_AXES // allow two motors on an axis 
	  
		#define X_STEP_PIN      GPIO_NUM_27    	// use Z labeled connector
		#define X_DIRECTION_PIN   GPIO_NUM_33 	// use Z labeled connector
		
		#define Y_STEP_PIN      GPIO_NUM_14
		#define Y_STEP_B_PIN    GPIO_NUM_21  	// ganged motor
		#define Y_DIRECTION_PIN   GPIO_NUM_25 
		#define Y_AXIS_SQUARING
		
		#define Z_STEP_PIN      GPIO_NUM_12  	// use X labeled connector
		#define Z_STEP_B_PIN    GPIO_NUM_22		// use X labeled connector
		#define Z_DIRECTION_PIN   GPIO_NUM_26 	// use X labeled connector
		#define Z_AXIS_SQUARING
		
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
				
		// Note: if you use PWM rather than relay, you could map GPIO_NUM_17 to mist or flood 
		#define USE_SPINDLE_RELAY
		
		#ifdef USE_SPINDLE_RELAY		
			#define SPINDLE_PWM_PIN    GPIO_NUM_17
		#else
			#define SPINDLE_PWM_PIN    GPIO_NUM_16
			#define SPINDLE_ENABLE_PIN	GPIO_NUM_32
		#endif
		
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)		
		
		// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
		// Relay can be used for Spindle or Coolant
		//#define COOLANT_FLOOD_PIN 	GPIO_NUM_17
		
		#define X_LIMIT_PIN      	GPIO_NUM_15  
		#define Y_LIMIT_PIN      	GPIO_NUM_4  
		#define Z_LIMIT_PIN     	GPIO_NUM_2 	
		#define LIMIT_MASK      	B111
		
		#define PROBE_PIN       	GPIO_NUM_35  
		
		// The default value in config.h is wrong for this controller
		#ifdef INVERT_CONTROL_PIN_MASK
			#undef INVERT_CONTROL_PIN_MASK			
		#endif
		
		#define INVERT_CONTROL_PIN_MASK   B1110
		
		// Note: check the #define IGNORE_CONTROL_PINS is the way you want in config.h
		#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup    		
		
#endif

#ifdef CPU_MAP_TMC2130_PEN
		
		#define CPU_MAP_NAME "ESP32_TMC2130_PEN"
		
		#define USE_RMT_STEPS
		
		#define USE_TMC2130 // make sure you assign chip select pins to each axis
		
		#define X_STEP_PIN      	GPIO_NUM_12
		#define X_DIRECTION_PIN   	GPIO_NUM_26
		#define X_CS_PIN    		GPIO_NUM_17  //chip select
		#define X_RMT_CHANNEL		0		
		
		#define Y_STEP_PIN      	GPIO_NUM_14   
		#define Y_DIRECTION_PIN   	GPIO_NUM_25  
		#define Y_CS_PIN    		GPIO_NUM_16  //chip select	
		#define Y_RMT_CHANNEL		1		
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
		// *** the flood coolant feature code is activated by defining this pins
		// *** Comment it out to use the pin for other features
		//#define COOLANT_FLOOD_PIN 	GPIO_NUM_16			
		//#define COOLANT_MIST_PIN   	GPIO_NUM_21
		
		// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
		// use a virtual spindle. Do not comment out the other parameters for the spindle.
		//#define SPINDLE_PWM_PIN    GPIO_NUM_17 
		#define SPINDLE_PWM_CHANNEL 0
		// PWM Generator is based on 80,000,000 Hz counter
		// Therefor the freq determines the resolution
		// 80,000,000 / freq = max resolution
		// For 5000 that is 80,000,000 / 5000 = 16000 
		// round down to nearest bit count for SPINDLE_PWM_MAX_VALUE = 13bits (8192)
		#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif
		
		#define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)		
		
		#define X_LIMIT_PIN      	GPIO_NUM_2  
		#define Y_LIMIT_PIN      	GPIO_NUM_4  
		#define LIMIT_MASK      	B11
		
#endif



	// ================= common to all machines ================================
	
	// These are some ESP32 CPU Settings that the program needs, but are generally not changed
		#define F_TIMERS	80000000    // a reference to the speed of ESP32 timers
		#define F_STEPPER_TIMER 20000000  // frequency of step pulse timer
		#define STEPPER_OFF_TIMER_PRESCALE 8 // gives a frequency of 10MHz
		#define STEPPER_OFF_PERIOD_uSEC  3  // each tick is
		
		#define STEP_PULSE_MIN 2   // uSeconds
		#define STEP_PULSE_MAX 10  // uSeconds
		
		// =============== Don't change or comment these out ======================
		// They are for legacy purposes and will not affect your I/O 
		
		#define X_STEP_BIT    0  // don't change
		#define Y_STEP_BIT    1  // don't change
		#define Z_STEP_BIT    2  // don't change
		#define STEP_MASK       B111 // don't change
		
		#define X_DIRECTION_BIT   0 // don't change
		#define Y_DIRECTION_BIT   1  // don't change
		#define Z_DIRECTION_BIT   2  // don't change
		
		#define X_LIMIT_BIT      	0  // don't change
		#define Y_LIMIT_BIT      	1  // don't change
		#define Z_LIMIT_BIT     	2  // don't change
		
		
		#define PROBE_MASK        1 // don't change		
		
		#define CONTROL_MASK      				B1111  	// don't change
		
		// =======================================================================
		
		
#endif

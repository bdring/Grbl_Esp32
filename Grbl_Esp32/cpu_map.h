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
		#define X_STEP_PIN      GPIO_NUM_12
		#define Y_STEP_PIN      GPIO_NUM_14
    #define Z_STEP_PIN      GPIO_NUM_27		
		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		#define Y_DIRECTION_PIN   GPIO_NUM_25  
		#define Z_DIRECTION_PIN   GPIO_NUM_33 
		
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
		
		
#ifdef CPU_MAP_PEN_LASER  // The Buildlog.net pen laser controller V1
		#define X_STEP_PIN      GPIO_NUM_12
		#define Y_STEP_PIN      GPIO_NUM_14		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		#define Y_DIRECTION_PIN   GPIO_NUM_25  
		
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13
		
		#define X_LIMIT_PIN      	GPIO_NUM_2  
		#define Y_LIMIT_PIN      	GPIO_NUM_4
		#define LIMIT_MASK      	B11
		
		// ignored via config.h
		#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
		#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup
		
		
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
		
		#define SERVO_PEN_PIN 					GPIO_NUM_27
		
		
#endif

#ifdef CPU_MAP_MIDTBOT  // Buildlog.net midtbot
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
		
		#ifndef USE_PEN_SERVO  // maybe set in config.h
			#define USE_PEN_SERVO
		#endif
		
		#ifndef IGNORE_CONTROL_PINS // maybe set in config.h
			#define IGNORE_CONTROL_PINS
		#endif
		#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
		#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
		#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
		#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup		
		
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
		#define HOMING_CYCLE_0 (1<<X_AXIS)
		#define HOMING_CYCLE_1 (1<<Y_AXIS)
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
		#define INVERT_CONTROL_PIN_MASK   B1110		// don't change
		
		// =======================================================================
		
		
#endif

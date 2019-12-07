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
	
//Set your pin definition 
//let -1 to use default board pin
#define GRBL_SPI_SS -1
#define GRBL_SPI_MOSI -1
#define GRBL_SPI_MISO -1
#define GRBL_SPI_SCK -1
//Set your frequency 
#define GRBL_SPI_FREQ  4000000

#ifdef CPU_MAP_TEST_DRIVE
	/*
	This is just a demo CPU_MAP for test driving. It creates a basic 3 axis machine, but
	no actual i/o is used. It will appear that axes are moving, but they are virtual

	This can be uploaded to an unattached ESP32 or attached to unknown hardware. There is no risk
	pins trying to output signals into a short, etc that could dmamge the ESP32

	Assuming no changes have been made to the config.h file it is also a way to get he basic program
	running so OTA (over the air) firmware loading can be done.

	*/
	#define CPU_MAP_NAME "CPU_MAP_DEFAULT - Demo Only No I/O!"	
	
	#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_21  // Uno A1 
	
	#define LIMIT_MASK 0  // no limit pins		
#endif

#ifdef CPU_MAP_ESP32
	// This is the CPU Map for the ESP32 Development Controller
	// https://github.com/bdring/Grbl_ESP32_Development_Controller
	// https://www.tindie.com/products/33366583/grbl_esp32-cnc-development-board-v35/
	
	// Select the version (uncomment one of them)
	//#define CPU_MAP_V3p5 // version 3.5 and earlier
	#define CPU_MAP_V4 // version 4 or higher (in developement)
	
	#define USE_RMT_STEPS
	
	// It is OK to comment out any step and direction pins. This
    // won't affect operation except that there will be no output
	// form the pins. Grbl will virtually move the axis. This could 
	// be handy if you are using a servo, etc. for another axis.
	#if (defined CPU_MAP_V4)
		#define CPU_MAP_NAME 		"CPU_MAP_ESP32_V4"
		#define X_DIRECTION_PIN  	GPIO_NUM_14
		#define Y_STEP_PIN      	GPIO_NUM_26
		#define Y_DIRECTION_PIN   	GPIO_NUM_15
		//#define COOLANT_FLOOD_PIN 	GPIO_NUM_25
		#define SPINDLE_PWM_PIN    	GPIO_NUM_2
		#define X_LIMIT_PIN      	GPIO_NUM_17
		#define Z_LIMIT_PIN     	GPIO_NUM_16		
	#elif (defined CPU_MAP_V3p5)
		#define CPU_MAP_NAME 		"CPU_MAP_ESP32_V3.5"
		#define X_DIRECTION_PIN  	GPIO_NUM_26
		#define Y_STEP_PIN      	GPIO_NUM_14
		#define Y_DIRECTION_PIN   	GPIO_NUM_25
		//#define COOLANT_FLOOD_PIN 	GPIO_NUM_16
		#define SPINDLE_PWM_PIN    	GPIO_NUM_17
		#define X_LIMIT_PIN      	GPIO_NUM_2
		#define Z_LIMIT_PIN     	GPIO_NUM_15 
	#endif
	
	#define X_STEP_PIN      	GPIO_NUM_12
	
	#define X_RMT_CHANNEL		0
	
	// #define Y_STEP_PIN (see versions above) 	
	#define Y_RMT_CHANNEL		1
	
	#define Z_STEP_PIN      	GPIO_NUM_27		
	#define Z_DIRECTION_PIN   	GPIO_NUM_33		 
	#define Z_RMT_CHANNEL		2		
	
	// OK to comment out to use pin for other features
	#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
	//#define COOLANT_MIST_PIN   	GPIO_NUM_21
	#define USER_DIGITAL_PIN_1	GPIO_NUM_21
	#define USER_DIGITAL_PIN_2	GPIO_NUM_25
	
	
	#define SPINDLE_PWM_CHANNEL 0
	#define SPINDLE_PWM_BIT_PRECISION 8	
	#define SPINDLE_ENABLE_PIN	GPIO_NUM_22
	
	// see versions for X and Z
	#define Y_LIMIT_PIN      	GPIO_NUM_4	
	#define LIMIT_MASK      	B111
	
	#define PROBE_PIN       	GPIO_NUM_32  
	
	#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
	#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
	#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
	#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup 
#endif

#ifdef CPU_MAP_ESPDUINO_32
    // !!!! Experimental Untested !!!!!
	// This is a CPU MAP for ESPDUINO-32 Boards and Protoneer V3 boards
	// Note: Probe pin is mapped, but will require a 10k external pullup to 3.3V to work.
	#define CPU_MAP_NAME "CPU_MAP_ESPDUINO_32"	
	
	#define USE_RMT_STEPS
	
	#define X_STEP_PIN      		GPIO_NUM_26  // Uno D2
	#define X_DIRECTION_PIN  		GPIO_NUM_16  // Uno D5
	#define X_RMT_CHANNEL			0
	
	#define Y_STEP_PIN      		GPIO_NUM_25  // Uno D3
	#define Y_DIRECTION_PIN   	GPIO_NUM_27  // Uno D6
	#define Y_RMT_CHANNEL			1
	
	#define Z_STEP_PIN      		GPIO_NUM_17	// Uno D4
	#define Z_DIRECTION_PIN   	GPIO_NUM_14	// Uno D7
	#define Z_RMT_CHANNEL			2		
	
	// OK to comment out to use pin for other features
	#define STEPPERS_DISABLE_PIN GPIO_NUM_12	// Uno D8
			
	#define SPINDLE_PWM_PIN    GPIO_NUM_19    // Uno D11
	#define SPINDLE_PWM_CHANNEL 0	
	#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE	
	#define SPINDLE_DIR_PIN		GPIO_NUM_18	// Uno D13
	
	#define COOLANT_FLOOD_PIN 	GPIO_NUM_34  // Uno A3
	#define COOLANT_MIST_PIN   	GPIO_NUM_36// Uno A4
	
	#define X_LIMIT_PIN      		GPIO_NUM_13  // Uno D9
	#define Y_LIMIT_PIN      		GPIO_NUM_5   // Uno D10
	#define Z_LIMIT_PIN     		GPIO_NUM_19  // Uno D12	
	#define LIMIT_MASK      		B111
	
	#define PROBE_PIN       		GPIO_NUM_39  // Uno A5		
	
	// comment out #define IGNORE_CONTROL_PINS in config.h to use control pins  	
	#define CONTROL_RESET_PIN         GPIO_NUM_2  // Uno A0
	#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_4  // Uno A1 
	#define CONTROL_CYCLE_START_PIN   GPIO_NUM_35  // Uno A2 ... ESP32 needs external pullup
#endif

#ifdef CPU_MAP_ESP32_ESC_SPINDLE
	// This is an example of using a Brushless DC Hobby motor as 
	// a spindle motor
	// See this wiki page for more info
	// https://github.com/bdring/Grbl_Esp32/wiki/BESC-Spindle-Feature
	
	#define CPU_MAP_NAME 		"CPU_MAP_ESP32_ESC_SPINDLE"	
		
	#define USE_RMT_STEPS	
	
	#define X_STEP_PIN      	GPIO_NUM_12
	#define X_DIRECTION_PIN  	GPIO_NUM_14	
	#define X_RMT_CHANNEL		0
	
	#define Y_STEP_PIN      	GPIO_NUM_26
	#define Y_DIRECTION_PIN   	GPIO_NUM_15// #define Y_STEP_PIN (see versions above) 	
	#define Y_RMT_CHANNEL		1
	
	#define Z_STEP_PIN      	GPIO_NUM_27		
	#define Z_DIRECTION_PIN   	GPIO_NUM_33		 
	#define Z_RMT_CHANNEL		2		
	
	// OK to comment out to use pin for other features
	#define STEPPERS_DISABLE_PIN GPIO_NUM_13	
		
	#define SPINDLE_PWM_PIN    	GPIO_NUM_2
	#define SPINDLE_ENABLE_PIN	GPIO_NUM_22
	#define SPINDLE_PWM_CHANNEL 0	
			
	// Begin RC ESC Based Spindle Information ======================	
	#define SPINDLE_PWM_BIT_PRECISION 16   // 16 bit recommended for ESC (don't change)
	/*	
		Important ESC Settings
		$33=50 // Hz this is the typical good frequency for an ESC
		
		Determine the typical min and max pulse length of your ESC
		min_pulse is typically 1ms (0.001 sec) or less
		max_pulse is typically 2ms (0.002 sec) or more
		
		determine PWM_period. It is (1/freq) if freq = 50...period = 0.02 
		
		determine pulse length for min_pulse and max_pulse in percent.
		
		(pulse / PWM_period)
		
		min_pulse = (0.001 / 0.02) = 0.035 = 3.5%  so ... $33 and $34 = 3.5
		max_pulse = (0.002 / .02) = 0.1 = 10%  so ... $36=10

	*/
	
	
	// End RC ESC Based Spindle #defines ===========================
	
	#define X_LIMIT_PIN      	GPIO_NUM_17
	#define Y_LIMIT_PIN      	GPIO_NUM_4
	#define Z_LIMIT_PIN     	GPIO_NUM_16		
	#define LIMIT_MASK      	B111
	
	#define PROBE_PIN       	GPIO_NUM_32  
	
	#define CONTROL_SAFETY_DOOR_PIN   GPIO_NUM_35  // needs external pullup
	#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
	#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
	#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup 
#endif		
		
#ifdef CPU_MAP_PEN_LASER  // The Buildlog.net pen laser controller V1 & V2

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
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
			
		
		#define USING_SERVO  // uncomment to use this feature
		//#define USING_SOLENOID // uncomment to use this feature
		
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
	
	#define USE_RMT_STEPS

		#define X_STEP_PIN      GPIO_NUM_12
		#define Y_STEP_PIN      GPIO_NUM_14
		#define X_RMT_CHANNEL		0
		
		#define X_DIRECTION_PIN   GPIO_NUM_26
		#define Y_DIRECTION_PIN   GPIO_NUM_25
		#define Y_RMT_CHANNEL		1		
		
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
		#define SERVO_Z_HOMING_TYPE		SERVO_HOMING_TARGET // during homing it will instantly move to a target value
		#define SERVO_Z_HOME_POS			SERVO_Z_RANGE_MAX // move to max during homing
		#define SERVO_Z_MPOS					false		// will not use mpos, uses work coordinates
		
		#ifndef IGNORE_CONTROL_PINS // maybe set in config.h
			#define IGNORE_CONTROL_PINS
		#endif
		
			
		
		// redefine some stuff from config.h
		#ifdef HOMING_CYCLE_0
			#undef HOMING_CYCLE_0
		#endif
		#define HOMING_CYCLE_0 (1<<Y_AXIS)
		#ifdef HOMING_CYCLE_1
			#undef HOMING_CYCLE_1
		#endif
		#define HOMING_CYCLE_1 (1<<X_AXIS)
		#ifdef HOMING_CYCLE_2
			#undef HOMING_CYCLE_2
		#endif
				
		#define SERVO_PEN_PIN 					GPIO_NUM_27
		
		
		
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
	#include "polar_coaster.h"	
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
		//#define SPINDLE_PWM_BASE_FREQ 5000 // Hz
		#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
		#define SPINDLE_PWM_OFF_VALUE     0
		#define SPINDLE_PWM_MAX_VALUE     255 // (2^SPINDLE_PWM_BIT_PRECISION)
		
		#ifndef SPINDLE_PWM_MIN_VALUE
				#define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
		#endif	
		
		#define SERVO_Y_PIN 					GPIO_NUM_14
		#define SERVO_Y_CHANNEL_NUM 	6
		#define SERVO_Y_RANGE_MIN			0.0
		#define SERVO_Y_RANGE_MAX			30.0
		
		#define SERVO_Z_PIN 					GPIO_NUM_27
		#define SERVO_Z_CHANNEL_NUM 	5
		#define SERVO_Z_RANGE_MIN			0.0
		#define SERVO_Z_RANGE_MAX			20.0
		
		
		
		
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

#ifdef CPU_MAP_MPCNC  // all versions...select below
	// This is the CPU Map for the Buildlog.net MPCNC controller
	
	// uncomment ONE of the following versions
	//#define V1P1
	#define V1P2  // works for V1.2.1 as well
		
	#ifdef V1P1
		#define CPU_MAP_NAME "CPU_MAP_MPCNC_V1P1"
	#else // V1P2
		#define CPU_MAP_NAME "CPU_MAP_MPCNC_V1P2"
	#endif	
		
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
		#ifdef V1P1
			#define SPINDLE_PWM_PIN    GPIO_NUM_17
		#else // V1p2
			#define SPINDLE_PWM_PIN    GPIO_NUM_2
		#endif			
	#else
		#define SPINDLE_PWM_PIN    GPIO_NUM_16
		#define SPINDLE_ENABLE_PIN	GPIO_NUM_32
	#endif
	
	#define SPINDLE_PWM_CHANNEL 0	
	#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
	
	
	// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
	// Relay can be used for spindle or either coolant
	//#define COOLANT_FLOOD_PIN 	GPIO_NUM_2
	//#define COOLANT_MIST_PIN   	GPIO_NUM_2
	
	#ifdef V1P1	//v1p1
		#define X_LIMIT_PIN      	GPIO_NUM_2
	#else
		#define X_LIMIT_PIN      	GPIO_NUM_17
	#endif	
	
	#define Y_LIMIT_PIN      	GPIO_NUM_4  
	#define Z_LIMIT_PIN     	GPIO_NUM_15 	
	#define LIMIT_MASK      	B111
	
	#ifdef V1P2
		#ifndef ENABLE_SOFTWARE_DEBOUNCE   // V1P2 does not have R/C filters
			#define ENABLE_SOFTWARE_DEBOUNCE
		#endif
	#endif
	
	#define PROBE_PIN       	GPIO_NUM_35  
	
	// The default value in config.h is wrong for this controller
	#ifdef INVERT_CONTROL_PIN_MASK
		#undef INVERT_CONTROL_PIN_MASK			
	#endif
	
	#define INVERT_CONTROL_PIN_MASK   B1110
	
	// Note: default is #define IGNORE_CONTROL_PINS in config.h
	// uncomment to these lines to use them		
	#ifdef IGNORE_CONTROL_PINS
		#undef IGNORE_CONTROL_PINS
	#endif
	
	#define CONTROL_RESET_PIN         GPIO_NUM_34  // needs external pullup
	#define CONTROL_FEED_HOLD_PIN     GPIO_NUM_36  // needs external pullup 
	#define CONTROL_CYCLE_START_PIN   GPIO_NUM_39  // needs external pullup    

		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
	#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 //  255 = Keep steppers on 

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

	#define DEFAULT_HOMING_ENABLE 1  // false
	#define DEFAULT_HOMING_DIR_MASK 3 // move positive dir Z,negative X,Y
	#define DEFAULT_HOMING_FEED_RATE 600.0 // mm/min
	#define DEFAULT_HOMING_SEEK_RATE 2000.0 // mm/min
	#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
	#define DEFAULT_HOMING_PULLOFF 1.5 // mm

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

	#define DEFAULT_X_MAX_RATE 8000.0 // mm/min
	#define DEFAULT_Y_MAX_RATE 8000.0 // mm/min
	#define DEFAULT_Z_MAX_RATE 3000.0 // mm/min

	#define DEFAULT_X_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
	#define DEFAULT_Y_ACCELERATION (200.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
	#define DEFAULT_Z_ACCELERATION (100.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2

	#define DEFAULT_X_MAX_TRAVEL 500.0 // mm NOTE: Must be a positive value.
	#define DEFAULT_Y_MAX_TRAVEL 500.0 // mm NOTE: Must be a positive value.
	#define DEFAULT_Z_MAX_TRAVEL 80.0 // mm NOTE: Must be a positive value.
		
#endif

#ifdef CPU_MAP_LOWRIDER // !!!!!!!!!!!!!!!!! Warning: Untested !!!!!!!!!!!!!!!!! //
	// This is the CPU Map for the Buildlog.net MPCNC controller
	// used in lowrider mode. Low rider has (2) Y and Z and one X motor
	// These will not match the silkscreen or schematic descriptions	
	//#define V1P1
	#define V1P2  // works for V1.2.1 as well
		
	#ifdef V1P1
		#define CPU_MAP_NAME "CPU_MAP_LOWRIDER_V1P1"
	#else // V1P2
		#define CPU_MAP_NAME "CPU_MAP_LOWRIDER_V1P2"
	#endif	
		
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
			
	// Note: if you use PWM rather than relay, you could map GPIO_NUM_2 to mist or flood 
	//#define USE_SPINDLE_RELAY
	
	#ifdef USE_SPINDLE_RELAY
		#ifdef V1P1
			#define SPINDLE_PWM_PIN    GPIO_NUM_17
		#else // V1p2
			#define SPINDLE_PWM_PIN    GPIO_NUM_2
		#endif			
	#else
		#define SPINDLE_PWM_PIN    GPIO_NUM_16
		#define SPINDLE_ENABLE_PIN	GPIO_NUM_32
	#endif
		
	#define SPINDLE_PWM_CHANNEL 0	
	#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
	
	
	// Note: Only uncomment this if USE_SPINDLE_RELAY is commented out.
	// Relay can be used for Spindle or Coolant
	//#define COOLANT_FLOOD_PIN 	GPIO_NUM_17
	
	#define X_LIMIT_PIN      	GPIO_NUM_15  
	#define Y_LIMIT_PIN     	GPIO_NUM_4
	
	#ifdef V1P1	//v1p1
		#define Z_LIMIT_PIN      	GPIO_NUM_2
	#else
		#define Z_LIMIT_PIN      	GPIO_NUM_17
	#endif	
	
	#define LIMIT_MASK      	B111
	
	#ifdef V1P2
		#ifndef ENABLE_SOFTWARE_DEBOUNCE   // V1P2 does not have R/C filters
			#define ENABLE_SOFTWARE_DEBOUNCE
		#endif
	#endif	
	
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

	// Select a version to match your PCB
	//#define CPU_MAP_V1 // version 1 PCB
	#define CPU_MAP_V2 // version 2 PCB
	
	#ifdef CPU_MAP_V1
		#define CPU_MAP_NAME "ESP32_TMC2130_PEN V1"
		#define X_LIMIT_PIN      	GPIO_NUM_2
	#else
		#define CPU_MAP_NAME "ESP32_TMC2130_PEN V2"
		#define X_LIMIT_PIN      	GPIO_NUM_32
	#endif		
		
	#define USE_RMT_STEPS
	
	#define USE_TRINAMIC // Using at least 1 trinamic driver
	
	#define X_STEP_PIN      	GPIO_NUM_12
	#define X_DIRECTION_PIN   	GPIO_NUM_26
	#define X_TRINAMIC   	   	// using SPI control
	#define X_DRIVER_TMC2130 	// Which Driver Type?
	#define X_CS_PIN    			GPIO_NUM_17  //chip select
	#define X_RSENSE				0.11f   // .11 Ohm
	#define X_RMT_CHANNEL		0
	
	#define Y_STEP_PIN      	GPIO_NUM_14   
	#define Y_DIRECTION_PIN   	GPIO_NUM_25 
	#define Y_TRINAMIC   	   	// using SPI control
	#define Y_DRIVER_TMC2130 	// Which Driver Type?
	#define Y_CS_PIN    			GPIO_NUM_16  //chip select
	#define Y_RSENSE				0.11f   // .11 Ohm
	#define Y_RMT_CHANNEL		1		
	
	// OK to comment out to use pin for other features
	#define STEPPERS_DISABLE_PIN GPIO_NUM_13	
	
	#ifndef USE_SERVO_AXES  // maybe set in config.h
		#define USE_SERVO_AXES
	#endif
	
	#define SERVO_Z_PIN 				GPIO_NUM_27
	#define SERVO_Z_CHANNEL_NUM 		5
	#define SERVO_Z_RANGE_MIN			0.0
	#define SERVO_Z_RANGE_MAX			5.0
	#define SERVO_Z_HOMING_TYPE			SERVO_HOMING_TARGET // during homing it will instantly move to a target value
	#define SERVO_Z_HOME_POS			SERVO_Z_RANGE_MAX // move to max during homing
	#define SERVO_Z_MPOS				false		// will not use mpos, uses work coordinates
	
	// *** the flood coolant feature code is activated by defining this pins
	// *** Comment it out to use the pin for other features
	//#define COOLANT_FLOOD_PIN 	GPIO_NUM_16			
	//#define COOLANT_MIST_PIN   	GPIO_NUM_21
	
	// If SPINDLE_PWM_PIN is commented out, this frees up the pin, but Grbl will still
	// use a virtual spindle. Do not comment out the other parameters for the spindle.
	//#define SPINDLE_PWM_PIN    GPIO_NUM_17 
	#define SPINDLE_PWM_CHANNEL 0		
	#define SPINDLE_PWM_BIT_PRECISION 8   // be sure to match this with SPINDLE_PWM_MAX_VALUE
	
	
	// #define X_LIMIT_PIN      	See version section
	#define Y_LIMIT_PIN      	GPIO_NUM_4
	#define LIMIT_MASK      	B11	
		
#endif

#ifdef CPU_MAP_FOO_6X
	#define CPU_MAP_NAME "CPU_MAP_FOO_6X"	
	
	// Be sure to change to N_AXIS 6 in nuts_bolts.h
	#ifdef N_AXIS
		#undef N_AXIS
	#endif
	#define N_AXIS 6
	
	// stepper motors
	#define USE_RMT_STEPS	
	
	#define X_STEP_PIN      	GPIO_NUM_12
	#define X_DIRECTION_PIN   GPIO_NUM_26
	#define X_RMT_CHANNEL		0		
	
	#define Y_STEP_PIN      	GPIO_NUM_14   
	#define Y_DIRECTION_PIN   GPIO_NUM_25
	#define Y_RMT_CHANNEL		1

	// Z is a servo
	
	#define A_STEP_PIN      	GPIO_NUM_27   
	#define A_DIRECTION_PIN   GPIO_NUM_33
	#define A_RMT_CHANNEL		2	
	
	#define B_STEP_PIN      	GPIO_NUM_15
	#define B_DIRECTION_PIN   GPIO_NUM_32
	#define B_RMT_CHANNEL		3	
	
	// C is a servo
		
		// servos
		#define USE_SERVO_AXES
		#define SERVO_Z_PIN 			GPIO_NUM_22
		#define SERVO_Z_CHANNEL_NUM 	6
		#define SERVO_Z_RANGE_MIN		0.0
		#define SERVO_Z_RANGE_MAX		5.0
		#define SERVO_Z_HOMING_TYPE	SERVO_HOMING_TARGET // during homing it will instantly move to a target value
		#define SERVO_Z_HOME_POS		SERVO_Z_RANGE_MAX // move to max during homing
		#define SERVO_Z_MPOS				false		// will not use mpos, uses work coordinates
		
		#define SERVO_C_PIN 			GPIO_NUM_2
		#define SERVO_C_CHANNEL_NUM 	7
		#define SERVO_C_RANGE_MIN		0.0
		#define SERVO_C_RANGE_MAX		5.0
		#define SERVO_C_HOMING_TYPE	SERVO_HOMING_TARGET // during homing it will instantly move to a target value
		#define SERVO_C_HOME_POS		SERVO_C_RANGE_MAX // move to max during homing
		#define SERVO_C_MPOS				false		// will not use mpos, uses work coordinates		
		
		// limit switches
		#define X_LIMIT_PIN      	GPIO_NUM_21
		#define Y_LIMIT_PIN      	GPIO_NUM_17
		#define A_LIMIT_PIN      	GPIO_NUM_16
		#define B_LIMIT_PIN      	GPIO_NUM_4
		#define LIMIT_MASK      	B11011
		
		// OK to comment out to use pin for other features
		#define STEPPERS_DISABLE_PIN GPIO_NUM_13		
		
		#ifdef HOMING_CYCLE_0	// undefine from config.h
			#undef HOMING_CYCLE_0
		#endif
		//#define HOMING_CYCLE_0 (1<<X_AXIS)
		#define HOMING_CYCLE_0 ((1<<X_AXIS)|(1<<Y_AXIS))
		//#define HOMING_CYCLE_0 ((1<<X_AXIS)|(1<<Y_AXIS) |(1<<A_AXIS)|(1<<B_AXIS))
		
		#ifdef HOMING_CYCLE_1	// undefine from config.h
			#undef HOMING_CYCLE_1
		#endif
		//#define HOMING_CYCLE_1 (1<<A_AXIS)  
		#define HOMING_CYCLE_1 ((1<<A_AXIS)|(1<<B_AXIS))
				
		#ifdef HOMING_CYCLE_2	// undefine from config.h
			#undef HOMING_CYCLE_2
		#endif
		/*
		#define HOMING_CYCLE_2 (1<<Y_AXIS)
		
		#ifdef HOMING_CYCLE_3	// undefine from config.h
			#undef HOMING_CYCLE_3
		#endif
		#define HOMING_CYCLE_3 (1<<B_AXIS)
		*/
				
		#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
		#define DEFAULT_STEPPER_IDLE_LOCK_TIME 200 // 200ms
		
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
		#define DEFAULT_HOMING_DIR_MASK 17 
		#define DEFAULT_HOMING_FEED_RATE 200.0 // mm/min
		#define DEFAULT_HOMING_SEEK_RATE 2000.0 // mm/min
		#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
		#define DEFAULT_HOMING_PULLOFF 3.0 // mm

		#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
		#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm
		
		#define DEFAULT_LASER_MODE 0 // false
		
		#define DEFAULT_X_STEPS_PER_MM 400.0
		#define DEFAULT_Y_STEPS_PER_MM 400.0
		#define DEFAULT_Z_STEPS_PER_MM 100.0 // This is percent in servo mode
		#define DEFAULT_A_STEPS_PER_MM 400.0
		#define DEFAULT_B_STEPS_PER_MM 400.0
		#define DEFAULT_C_STEPS_PER_MM 100.0 // This is percent in servo mode
		
		
		#define DEFAULT_X_MAX_RATE 30000.0 // mm/min
		#define DEFAULT_Y_MAX_RATE 30000.0 // mm/min
		#define DEFAULT_Z_MAX_RATE 8000.0 // mm/min
		#define DEFAULT_A_MAX_RATE 30000.0 // mm/min
		#define DEFAULT_B_MAX_RATE 30000.0 // mm/min
		#define DEFAULT_C_MAX_RATE 8000.0 // mm/min
		
		#define DEFAULT_X_ACCELERATION (1500.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Y_ACCELERATION (1500.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_Z_ACCELERATION (100.0*60*60) 
		#define DEFAULT_A_ACCELERATION (1500.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_B_ACCELERATION (1500.0*60*60) // 10*60*60 mm/min^2 = 10 mm/sec^2
		#define DEFAULT_C_ACCELERATION (100.0*60*60) 
		
		#define DEFAULT_X_MAX_TRAVEL 250.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Y_MAX_TRAVEL 250.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_Z_MAX_TRAVEL 100.0 // This is percent in servo mode	
		#define DEFAULT_A_MAX_TRAVEL 250.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_B_MAX_TRAVEL 250.0 // mm NOTE: Must be a positive value.
		#define DEFAULT_C_MAX_TRAVEL 100.0 // This is percent in servo mode				
		
#endif


#ifdef CPU_MAP_SPI_DAISY_4X
	#define CPU_MAP_NAME "SPI_DAISY_4X"
	
	#ifdef N_AXIS
		#undef N_AXIS
	#endif
	#define N_AXIS 4
	
	#define USE_TRINAMIC  
	#define TRINAMIC_DAISY_CHAIN
	#define USE_RMT_STEPS
	
	#define X_STEP_PIN      	GPIO_NUM_12
	#define X_DIRECTION_PIN   	GPIO_NUM_14
	#define X_TRINAMIC   	   	// using SPI control
	#define X_DRIVER_TMC2130 	// Which Driver Type?
	#define X_CS_PIN    		GPIO_NUM_17  // Daisy Chain, all share same CS pin
	#define X_RSENSE			0.11f   // .11 Ohm
	#define X_RMT_CHANNEL		0
	
	#define Y_STEP_PIN      	GPIO_NUM_27
	#define Y_DIRECTION_PIN   	GPIO_NUM_26
	#define Y_TRINAMIC   	   	// using SPI control
	#define Y_DRIVER_TMC2130 	// Which Driver Type?
	#define Y_CS_PIN    		X_CS_PIN  // Daisy Chain, all share same CS pin
	#define Y_RSENSE			0.11f   // .11 Ohm
	#define Y_RMT_CHANNEL		1	
	
	#define Z_STEP_PIN      	GPIO_NUM_15
	#define Z_DIRECTION_PIN   	GPIO_NUM_2
	#define Z_TRINAMIC   	   	// using SPI control
	#define Z_DRIVER_TMC2130 	// Which Driver Type?
	#define Z_CS_PIN    		X_CS_PIN  // Daisy Chain, all share same CS pin
	#define Z_RSENSE			0.11f   // .11 Ohm
	#define Z_RMT_CHANNEL		2

	#define A_STEP_PIN      	GPIO_NUM_33
	#define A_DIRECTION_PIN   	GPIO_NUM_32
	#define A_TRINAMIC   	   	// using SPI control
	#define A_DRIVER_TMC2130 	// Which Driver Type?
	#define A_CS_PIN    		X_CS_PIN  // Daisy Chain, all share same CS pin
	#define A_RSENSE			0.11f   // .11 Ohm
	#define A_RMT_CHANNEL		3
	
	#define STEPPERS_DISABLE_PIN GPIO_NUM_13
	
	#define COOLANT_MIST_PIN   	GPIO_NUM_21
	
	#define SPINDLE_PWM_PIN    			GPIO_NUM_25
	#define SPINDLE_ENABLE_PIN			GPIO_NUM_4
	#define SPINDLE_PWM_CHANNEL 		0
	#define SPINDLE_PWM_BIT_PRECISION 	8	
	
	#define PROBE_PIN       	GPIO_NUM_22
	
	#define X_LIMIT_PIN      	GPIO_NUM_36
	#define Y_LIMIT_PIN      	GPIO_NUM_39
	#define Z_LIMIT_PIN      	GPIO_NUM_34
	#define A_LIMIT_PIN      	GPIO_NUM_35
	#define LIMIT_MASK      	B1111
	
#endif

#ifdef CPU_MAP_ATARI_1020
	#include "atari_1020.h"	
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
		#define A_STEP_BIT    3  // don't change
		#define B_STEP_BIT    4  // don't change
		#define C_STEP_BIT    5  // don't change
		#define STEP_MASK       B111111 // don't change
		
		#define X_DIRECTION_BIT   0 // don't change
		#define Y_DIRECTION_BIT   1  // don't change
		#define Z_DIRECTION_BIT   2  // don't change
		#define A_DIRECTION_BIT   3 // don't change
		#define B_DIRECTION_BIT   4  // don't change
		#define C_DIRECTION_BIT   5  // don't change

		#define X_LIMIT_BIT      	0  // don't change
		#define Y_LIMIT_BIT      	1  // don't change
		#define Z_LIMIT_BIT     	2  // don't change
		#define A_LIMIT_BIT      	3  // don't change
		#define B_LIMIT_BIT      	4  // don't change
		#define C_LIMIT_BIT     	5  // don't change		
		
		#define PROBE_MASK        1 // don't change		
		
		#define CONTROL_MASK      				B1111  	// don't change
		
		// =======================================================================
		
		
#endif

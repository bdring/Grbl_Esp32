/*
  kinematics_polar_coaster.h - Implements simple kinematics for Grbl_ESP32
  Part of Grbl_ESP32

  Copyright (c) 2019 Barton Dring @buildlog
      
	 
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

#define RADIUS_AXIS 0
#define POLAR_AXIS 1

#define SEGMENT_LENGTH 0.5 // segment length in mm
#define USE_KINEMATICS
#define FWD_KINEMATICS_REPORTING // report in cartesian	
#define USE_M30

// ============= Begin CPU MAP ================
#define CPU_MAP_NAME "CPU_MAP_POLAR_COASTER"	

#define USE_RMT_STEPS

#define X_STEP_PIN			GPIO_NUM_15
#define X_RMT_CHANNEL		0
#define Y_STEP_PIN      	GPIO_NUM_2
#define Y_RMT_CHANNEL		1
#define X_DIRECTION_PIN 	GPIO_NUM_25
#define Y_DIRECTION_PIN 	GPIO_NUM_26

#define STEPPERS_DISABLE_PIN GPIO_NUM_17

#ifndef USE_SERVO_AXES  // maybe set in config.h
  #define USE_SERVO_AXES
#endif

#define SERVO_Z_PIN 					GPIO_NUM_16
#define SERVO_Z_CHANNEL_NUM 		5
#define SERVO_Z_RANGE_MIN			0.0
#define SERVO_Z_RANGE_MAX			5.0
#define SERVO_Z_HOMING_TYPE		SERVO_HOMING_TARGET // during homing it will instantly move to a target value
#define SERVO_Z_HOME_POS			SERVO_Z_RANGE_MAX // move to max during homing
#define SERVO_Z_MPOS					false		// will not use mpos, uses work coordinates
  
#define X_LIMIT_PIN      	GPIO_NUM_4
#define LIMIT_MASK      	B1

#ifdef IGNORE_CONTROL_PINS // maybe set in config.h
  #undef IGNORE_CONTROL_PINS
#endif

#ifndef ENABLE_CONTROL_SW_DEBOUNCE
  #define ENABLE_CONTROL_SW_DEBOUNCE
#endif

#ifdef CONTROL_SW_DEBOUNCE_PERIOD
  #undef CONTROL_SW_DEBOUNCE_PERIOD
#endif
#define CONTROL_SW_DEBOUNCE_PERIOD 100 // really long debounce

#ifdef INVERT_CONTROL_PIN_MASK
  #undef INVERT_CONTROL_PIN_MASK
#endif
#define INVERT_CONTROL_PIN_MASK   B11111111

#define MACRO_BUTTON_0_PIN		GPIO_NUM_13
#define MACRO_BUTTON_1_PIN		GPIO_NUM_12
#define MACRO_BUTTON_2_PIN		GPIO_NUM_14
  
// redefine some stuff from config.h
#ifdef HOMING_CYCLE_0
  #undef HOMING_CYCLE_0
#endif
#define HOMING_CYCLE_0 (1<<X_AXIS) // this 'bot only homes the X axis
#ifdef HOMING_CYCLE_1
  #undef HOMING_CYCLE_1
#endif
#ifdef HOMING_CYCLE_2
  #undef HOMING_CYCLE_2
#endif		
	
// ============= End CPU MAP ==================

// ============= Begin Default Settings ================
#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay on

#define DEFAULT_STEPPING_INVERT_MASK 0 // uint8_t
#define DEFAULT_DIRECTION_INVERT_MASK 2 // uint8_t
#define DEFAULT_INVERT_ST_ENABLE 0 // boolean
#define DEFAULT_INVERT_LIMIT_PINS 1 // boolean
#define DEFAULT_INVERT_PROBE_PIN 0 // boolean 

#define DEFAULT_STATUS_REPORT_MASK 2 // MPos enabled

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
// ============= End Default Settings ==================

#ifndef kinematics_h
  #define kinematics_h
    
	#include "grbl.h"
		
bool kinematics_pre_homing(uint8_t cycle_mask);
void kinematics_post_homing();
void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position);
void calc_polar(float *target_xyz, float *polar, float last_angle);
float abs_angle(float ang);
void user_defined_macro(uint8_t index);

void forward_kinematics(float *position);
void user_m30();

#endif
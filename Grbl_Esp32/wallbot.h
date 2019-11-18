/*
	wallbot.h
	Part of Grbl_ESP32

	copyright (c) 2018 -	Bart Dring This file was modified for use on the ESP32
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

	This contains all the special features required to control a wall
	mounted pen plotter
	
*/

#define CPU_MAP_NAME "CPU_MAP_WALLBOT"

// Only turn these on during tuning....
//#define WALLBOT_SHOW_STALLGUARD_X  // to stream X motor stallgurd tuning data
#define WALLBOT_SHOW_STALLGUARD_Y  // to stream Y motor stallgurd tuning data

#define MAX_SEGMENT_LENGTH 0.5 				// kinematics segment length in mm
//#define BELT_LENGTH_CAL        		33.66 // 38.72	// how much belt length to add to home val to get to motor center width
#define WIDTH_CALIBRATION_INDEX		0
#define ARM_LENGTH_CAL_INDEX        1     // how much to add to belts for effector arms
#define HOMING_AXIS_LOCATE_SCALAR  	100.0	  // need it long because we use the y axis max as the Y zero offset (below)
// limit work area to what the machine can effectively do
#define WALLBOT_WORK_VERT_OFFSET 	75        // vertical work area limit...top limit is this much  below the pulleys
#define WALLBOT_WORK_HORIZ_OFFSET	40        // horizontal work area offset. It can get this close to the pulleys.

// the cooolstep setting for the different modes
#define NORMAL_TCOOLTHRS 		0xFFFFF // 20 bit is max
#define NORMAL_THIGH 			0
#define HOMING_TCOOLTHRS 		500
#define HOMING_THIGH 			300

// enable these special machine functions to be called from the main program
#define USE_KINEMATICS				// there are kinematic equations for this machine
//#define FWD_KINEMATICS_REPORTING   // report in cartesian
#define USE_RMT_STEPS   			// Use the RMT periferal to generate step pulses
#define USE_TRINAMIC				// some Trinamic motors are used on this machine
#define USE_MACHINE_TRINAMIC_INIT	// there is a machine specific setup for the drivers
#define USE_MACHINE_INIT			// There is some custom initialization for this machine

// ================== CPU MAP ============================
#define X_TRINAMIC				// The X axis uses a Trinamic SPI stepper driver
#define X_DRIVER_TMC2130 		// Which Driver Type?
#define X_STEP_PIN      		GPIO_NUM_12
#define X_DIRECTION_PIN			GPIO_NUM_26
#define X_CS_PIN    			GPIO_NUM_17  //chip select
#define X_RSENSE				0.11f   // .11 Ohm
#define DEFAULT_X_CURRENT 		0.600   // Settings default for current
#define DEFAULT_X_HOLD_CURRENT 	50  // 50% of run current
#define DEFAULT_X_MICROSTEPS 	16	
#define X_STALLGUARD	        8
#define X_RMT_CHANNEL 			0

#define Y_TRINAMIC
#define Y_DRIVER_TMC2130 	// Which Driver Type?
#define Y_STEP_PIN      		GPIO_NUM_14
#define Y_DIRECTION_PIN			GPIO_NUM_25
#define Y_CS_PIN    			GPIO_NUM_16  //chip select
#define Y_RSENSE				0.11f   // .11 Ohm
#define DEFAULT_Y_CURRENT 		0.600
#define DEFAULT_Y_HOLD_CURRENT 	50  // 50% of run current
#define DEFAULT_Y_MICROSTEPS 	16
#define Y_STALLGUARD	        30
#define Y_RMT_CHANNEL 			1

#define STEPPERS_DISABLE_PIN GPIO_NUM_13		

#define X_LIMIT_PIN      	GPIO_NUM_15
#define Y_LIMIT_PIN      	GPIO_NUM_4
#define LIMIT_MASK      	B11	

#ifndef ENABLE_CONTROL_SW_DEBOUNCE
	#define ENABLE_CONTROL_SW_DEBOUNCE
#endif

#ifdef INVERT_CONTROL_PIN_MASK
	#undef IGNORE_CONTROL_PINS
#endif

#define INVERT_CONTROL_PIN_MASK   B11110000

#define MACRO_BUTTON_0_PIN		GPIO_NUM_34
#define MACRO_BUTTON_1_PIN		GPIO_NUM_35
#define MACRO_BUTTON_2_PIN		GPIO_NUM_36
#define MACRO_BUTTON_3_PIN		GPIO_NUM_39	


/*
#define USE_SERVO_AXES
#define SERVO_Z_PIN 			GPIO_NUM_27
#define SERVO_Z_CHANNEL_NUM 	6
#define SERVO_Z_RANGE_MIN		0.0
#define SERVO_Z_RANGE_MAX		5.0
*/

// ================== CPU MAP ============================

// we just want (1) homing cycle defined. kinematics_homing(...) will do the rest 
#ifdef HOMING_CYCLE_1
	#undef HOMING_CYCLE_2
#endif
#ifdef HOMING_CYCLE_0
	#undef HOMING_CYCLE_3
#endif

// the homing touch, retract, touch again is not needed on this machine.
#ifdef N_HOMING_LOCATE_CYCLE
	#undef N_HOMING_LOCATE_CYCLE
	#define N_HOMING_LOCATE_CYCLE 0 // touch once and done.
#endif

#define HOMING_FORCE_SET_ORIGIN

#define SHOW_EXTENDED_SETTINGS // show the extra $$ settings

// defaults
#define DEFAULT_STEP_PULSE_MICROSECONDS 3 
#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // stay on

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

#define DEFAULT_HOMING_ENABLE 1  
#define DEFAULT_HOMING_DIR_MASK 3 // $23
#define DEFAULT_HOMING_FEED_RATE 750.0 // mm/min
#define DEFAULT_HOMING_SEEK_RATE 1687.0 // mm/min
#define DEFAULT_HOMING_DEBOUNCE_DELAY 250 // msec (0-65k)
#define DEFAULT_HOMING_PULLOFF 5.0 // mm

#define DEFAULT_SPINDLE_RPM_MAX 1000.0 // rpm
#define DEFAULT_SPINDLE_RPM_MIN 0.0 // rpm

#define DEFAULT_LASER_MODE 0 // false

#define DEFAULT_X_STEPS_PER_MM 88.889  
#define DEFAULT_Y_STEPS_PER_MM 88.889
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

#define DEFAULT_X_STALLGUARD 20
#define DEFAULT_Y_STALLGUARD 32  // needs a bigger value than x due to the long cable

#ifndef wallbot_h
	#define wallbot_h
	
	#include "grbl.h"

	void machine_init();
	//bool user_defined_homing();
	void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position);
	void forward_kinematics(float *position);
	bool kinematics_homing(uint8_t cycle_mask);
	bool wallbot_homing(uint8_t cycle_mask);
	void machine_trinamic_setup();
	void readSgTask(void *pvParameters);
	void user_defined_macro(uint8_t index);
	float get_angle(float oppo, float right, float left);
#endif
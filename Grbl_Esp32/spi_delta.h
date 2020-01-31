/*
	spi_delta.h
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

	
*/

#define CPU_MAP_NAME "CPU_MAP_SPI_DELTA"		

	// the cooolstep setting for the different modes
	#define DELTA_MOTOR_RUN_MODE	0  	// setup motor params for normal run mode
	#define DELTA_MOTOR_HOME_MODE	1	// setup motor params for sensorless homing mode
	#define NORMAL_TCOOLTHRS 		0 	// 20 bit is max
	#define NORMAL_THIGH 			0

	// enable these special machine functions to be called from the main program
	#define USE_KINEMATICS				// there are kinematic equations for this machine
	#define FWD_KINEMATICS_REPORTING   // report in cartesian
	#define USE_RMT_STEPS   			// Use the RMT periferal to generate step pulses
	#define USE_TRINAMIC				// some Trinamic motors are used on this machine
	#define USE_MACHINE_TRINAMIC_INIT	// there is a machine specific setup for the drivers
	#define USE_MACHINE_INIT			// There is some custom initialization for this machine	
	
	#define SEGMENT_LENGTH 0.5 // segment length in mm
	#define KIN_ANGLE_CALC_OK   0
	#define KIN_ANGLE_ERROR     -1
	
	#define MAX_NEGATIVE_ANGLE -0.75 // in radians how far can the arms go up?
	
// ================== Config ======================

	#ifdef HOMING_CYCLE_0
		#undef HOMING_CYCLE_0		
	#endif
	#define HOMING_CYCLE_0 ((1<<X_AXIS)|(1<<Y_AXIS)|(1<<Z_AXIS))
	
	#ifdef HOMING_CYCLE_1
		#undef HOMING_CYCLE_1
	#endif
	
	#ifdef HOMING_CYCLE_2
		#undef HOMING_CYCLE_2
	#endif
	
	// we only want one homing sequence...no backoff and repeat
	#ifdef N_HOMING_LOCATE_CYCLE
		#undef N_HOMING_LOCATE_CYCLE
	#endif
	#define N_HOMING_LOCATE_CYCLE 0 
	
	#define SHOW_EXTENDED_SETTINGS
	
// ================== CPU MAP ======================
	
	//#define N_AXIS 3
	
	#define USE_TRINAMIC  
	#define TRINAMIC_DAISY_CHAIN
	#define USE_RMT_STEPS
	
	#define X_STEP_PIN      	GPIO_NUM_12
	#define X_DIRECTION_PIN   	GPIO_NUM_14
	#define X_TRINAMIC   	   	// using SPI control
	#define X_DRIVER_TMC2130 	// Which Driver Type?
	#define X_CS_PIN    		GPIO_NUM_17  // Daisy Chain, all share same CS pin
	#define X_RSENSE			0.11f   // .11 Ohm
	
	#define Y_STEP_PIN      	GPIO_NUM_27
	#define Y_DIRECTION_PIN   	GPIO_NUM_26
	#define Y_TRINAMIC   	   	// using SPI control
	#define Y_DRIVER_TMC2130 	// Which Driver Type?
	#define Y_CS_PIN    		X_CS_PIN  // Daisy Chain, all share same CS pin
	#define Y_RSENSE			0.11f   // .11 Ohm
	
	#define Z_STEP_PIN      	GPIO_NUM_15
	#define Z_DIRECTION_PIN   	GPIO_NUM_2
	#define Z_TRINAMIC   	   	// using SPI control
	#define Z_DRIVER_TMC2130 	// Which Driver Type?
	#define Z_CS_PIN    		X_CS_PIN  // Daisy Chain, all share same CS pin
	#define Z_RSENSE			0.11f   // .11 Ohm
	
	#define STEPPERS_DISABLE_PIN GPIO_NUM_13	
	
	#define X_LIMIT_PIN      	GPIO_NUM_36
	#define Y_LIMIT_PIN      	GPIO_NUM_39
	#define Z_LIMIT_PIN      	GPIO_NUM_34
	#define LIMIT_MASK      	B111
	
// ================= defaults ===========================
	#define DEFAULT_STEPPER_IDLE_LOCK_TIME 255 // keep them on, the trinamics will reduce power at idle


	#define DEFAULT_X_MICROSTEPS 8
	#define DEFAULT_Y_MICROSTEPS DEFAULT_X_MICROSTEPS
	#define DEFAULT_Z_MICROSTEPS DEFAULT_X_MICROSTEPS
	
	// some math to figure out microsteps per unit // units could bedegrees or radians
	#define UNITS_PER_REV 6.2831853   // 360.0 degrees or 6.2831853 radians 
	#define STEPS_PER_REV 400.0
	#define REDUCTION_RATIO (60.0 / 16.0)
	#define MICROSTEPS_PER_REV (STEPS_PER_REV * (float)DEFAULT_X_MICROSTEPS * REDUCTION_RATIO)
	
	#define DEFAULT_X_STEPS_PER_MM (MICROSTEPS_PER_REV / UNITS_PER_REV)
	#define DEFAULT_Y_STEPS_PER_MM DEFAULT_X_STEPS_PER_MM
	#define DEFAULT_Z_STEPS_PER_MM DEFAULT_X_STEPS_PER_MM
	
	#define DEFAULT_X_MAX_RATE 400.0 // mm/min
	#define DEFAULT_Y_MAX_RATE DEFAULT_X_MAX_RATE
	#define DEFAULT_Z_MAX_RATE DEFAULT_X_MAX_RATE
	
	#define DEFAULT_X_ACCELERATION (90.0*60*60)
	#define DEFAULT_Y_ACCELERATION DEFAULT_X_ACCELERATION
	#define DEFAULT_Z_ACCELERATION DEFAULT_X_ACCELERATION	
	
	//  homing 
	#define DEFAULT_HOMING_FEED_RATE 25
	#define DEFAULT_HOMING_SEEK_RATE 50
	#define DEFAULT_HOMING_DIR_MASK 7
	// homing
	
	#define DEFAULT_USER_INT_80 800 // $80 User integer setting
	#define DEFAULT_USER_INT_81 350 // $80 User integer setting
	
	#define DEFAULT_X_MAX_TRAVEL 1.75 // 100 dgrees in radians
	#define DEFAULT_Y_MAX_TRAVEL DEFAULT_X_MAX_TRAVEL
	#define DEFAULT_Z_MAX_TRAVEL DEFAULT_X_MAX_TRAVEL
	
	#define DEFAULT_X_CURRENT 1.0
	#define DEFAULT_Y_CURRENT DEFAULT_X_CURRENT
	#define DEFAULT_Z_CURRENT DEFAULT_X_CURRENT
	
	#define DEFAULT_X_HOLD_CURRENT 25
	#define DEFAULT_Y_HOLD_CURRENT DEFAULT_X_HOLD_CURRENT
	#define DEFAULT_Z_HOLD_CURRENT DEFAULT_X_HOLD_CURRENT
	
	#define DEFAULT_X_STALLGUARD 3
	#define DEFAULT_Y_STALLGUARD 3
	#define DEFAULT_Z_STALLGUARD 3

#ifndef spi_delta_h
	
	#define spi_delta_h	
	
	#include "grbl.h"
	// function prototypes
	void machine_init();
	void readSgTask(void *pvParameters);
	void machine_trinamic_setup();
	void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position);
	void forward_kinematics(float *position);
	bool kinematics_pre_homing(uint8_t cycle_mask);
	void kinematics_post_homing();
	bool kinematics_homing(uint8_t cycle_mask);
	bool spi_delta_homing(uint8_t cycle_mask);
	int delta_calcInverse(float x0, float y0, float z0, float &theta1, float &theta2, float &theta3);
	int delta_calcAngleYZ(float x0, float y0, float z0, float &theta);
	int delta_calcForward(float theta1, float theta2, float theta3, float &x0, float &y0, float &z0) ;
	float three_axis_dist(float *pt1, float *pt2);
	void delta_motor_mode(uint8_t mode);
#endif
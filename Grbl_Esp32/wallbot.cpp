/*
	wallbot.cpp
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

	--------------------------------------------------------------
  
	This contains all the special features required to control a wall
	mounted pen plotter
	
	The work center is centered between the (2) motors in X
	The Y center is set by settings.max_travel[Y_AXIS]
	
	Display 'realtime' Stallgaurd data by turning on laser mode $32

	
	Some calibration values are stored in settings.max_travel[...];
		settings.machine_float[WIDTH_CALIBRATION_INDEX] = The amount we add to belt travel to get machine_width;		
		settings.machine_float[ARM_LENGTH_CAL_INDEX]= The distance added to the belt length to get to the pen
		settings.max_travel[Y_AXIS] = The distance below the motors we want our work Y0

	TODO
		Change the stallguard display to use $179
		Check valid work area

	
*/
#include "grbl.h"

#ifdef CPU_MAP_WALLBOT

float home_travel[N_AXIS];    				// how far did it travel during homing
float wallbot_machine_width = 0;				// the X distance between the motors
static TaskHandle_t readSgTaskHandle = 0;   // for realtime stallguard data diaplay


void machine_init()
{		

	//grbl_sendf(CLIENT_SERIAL, "[MSG:Home max dist %4.3f]\r\n", HOMING_AXIS_SEARCH_SCALAR * settings.max_travel[Y_AXIS]);

	#if (defined WALLBOT_SHOW_STALLGUARD_X || defined WALLBOT_SHOW_STALLGUARD_Y)
	// setup a task that will calculate solenoid position		
	xTaskCreatePinnedToCore(	readSgTask,    // task
								"readSgTask", // name for task
								4096,   // size of task stack
								NULL,   // parameters
								1, // priority
								&readSgTaskHandle,
								0 // core
							);
	#endif
}

void readSgTask(void *pvParameters)
{		
	TickType_t xLastWakeTime;
	const TickType_t xreadSg = 50;  // in ticks (typically ms)
	uint32_t x_tstep, y_tstep;
	uint8_t x_sg, y_sg;

	xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
	while(true) { // don't ever return from this or the task dies
		if (bit_istrue(settings.flags,BITFLAG_LASER_MODE)) { // use laser mode as a way to turn off this data

				x_tstep = TRINAMIC_X.TSTEP();
				x_sg = TRINAMIC_X.sg_result();
				y_tstep = TRINAMIC_Y.TSTEP();
				y_sg = TRINAMIC_Y.sg_result();

				if (x_tstep != 0xFFFFF || y_tstep != 0xFFFFF) { // if axis is moving 
					grbl_sendf(CLIENT_ALL, "SG Debug   SGX:%3d TSX:%5d    SGY:%3d TSY%5d \r\n", x_sg, x_tstep, y_sg, y_tstep);
				}				
		}						
		vTaskDelayUntil(&xLastWakeTime, xreadSg);
    }	
}

void machine_trinamic_setup() {
	grbl_send(CLIENT_SERIAL, "[MSG:Machine Trinamic Setup]\r\n");
	TRINAMIC_X.begin(); // Initiate pins and registries
	TRINAMIC_X.tbl(1);
	TRINAMIC_X.toff(3);
	//driver.pwm_autoscale(false);
	//TRINAMIC_X.blank_time(24);	
	TRINAMIC_X.microsteps(settings.microsteps[X_AXIS]);
	TRINAMIC_X.rms_current(settings.current[X_AXIS] * 1000.0, settings.hold_current[X_AXIS]/100.0);	
	TRINAMIC_X.TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
	TRINAMIC_X.THIGH(NORMAL_THIGH);
	TRINAMIC_X.hysteresis_start(4);
	TRINAMIC_X.hysteresis_end(-2);
	//TRINAMIC_X.diag1_stall(1); // stallguard i/o is on diag1
	//TRINAMIC_X.diag1_pushpull(0); // 0 = active low	
	TRINAMIC_X.sfilt(1);   
	//TRINAMIC_X.semin(5);
	//TRINAMIC_X.semax(2);
	//TRINAMIC_X.sedn(0b01);
	TRINAMIC_X.sgt(settings.stallguard[X_AXIS]);


	TRINAMIC_Y.begin(); // Initiate pins and registries
	TRINAMIC_Y.tbl(1);
	TRINAMIC_Y.toff(3);
	//driver.pwm_autoscale(false);
	//TRINAMIC_X.blank_time(24);	
	TRINAMIC_Y.microsteps(settings.microsteps[Y_AXIS]);
	TRINAMIC_Y.rms_current(settings.current[Y_AXIS] * 1000.0, settings.hold_current[Y_AXIS]/100.0);	
	TRINAMIC_Y.TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
	TRINAMIC_X.THIGH(NORMAL_THIGH);
	TRINAMIC_Y.hysteresis_start(4);
	TRINAMIC_Y.hysteresis_end(-2);	
	//TRINAMIC_Y.diag1_stall(1); // stallguard i/o is on diag1
	//TRINAMIC_Y.diag1_pushpull(0); // 0 = active low
	//
	TRINAMIC_Y.sfilt(1);   
	//TRINAMIC_X.semin(5);
	//TRINAMIC_X.semax(2);
	//TRINAMIC_X.sedn(0b01);
	TRINAMIC_Y.sgt(settings.stallguard[Y_AXIS]);

}

void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position)
{	
	float dx, dy, dz; 				// move distances in each cartesian axis
	float dist;		  				// total move length in xyz
	uint32_t segment_count;  		// number of segments the move will be broken in to.
	float seg_target[N_AXIS]; 		// The target of the current segment
	float converted_target[N_AXIS]; // target location in the new coordinates
	float cart_feedrate; 			// the cartesian feedrate from the original gcode
	bool work_area_warning = false; // Target location is not within work area



	if (wallbot_machine_width == 0) {
		grbl_send(CLIENT_SERIAL, "[MSG:Kinematics Alert: Machine width not known. Did you home it?]\r\n");
		mc_line(target, pl_data); // do not apply kinematics
		return;
	}

	// TO DO move these to homing
	float half_width = wallbot_machine_width / 2.0;
	float height = - settings.max_travel[Y_AXIS];

	grbl_sendf(CLIENT_SERIAL, "[MSG:Kins Half_Width %4.2f Height %4.2f]\r\n", half_width, height);
	
	float x_offset = gc_state.coord_system[X_AXIS] + gc_state.coord_offset[X_AXIS]; // offset from machine coordinate system
	float y_offset = gc_state.coord_system[Y_AXIS] + gc_state.coord_offset[Y_AXIS]; // offset from machine coordinate system
	float z_offset = gc_state.coord_system[Z_AXIS] + gc_state.coord_offset[Z_AXIS]; // offset from machine coordinate system
	
	grbl_sendf(CLIENT_SERIAL, "[MSG:Offsets X:%4.2f Y:%4.3f Z:%4.3f]\r\n", x_offset, y_offset, z_offset);

	// check X axis target
	/*
	if ((half_width - fabs(target[X_AXIS])) < WALLBOT_WORK_HORIZ_OFFSET) { // are into the edge margins?
		grbl_send(CLIENT_SERIAL, "[MSG:X move exceeds work area]\r\n");
		// TO DO raise a soft limit alarm
		work_area_warning = true;
	}

	// check Y axis too close to pulley level 
	// (belts getting close to horizontal requires too much torque)
	if ((height - target[Y_AXIS]) < WALLBOT_WORK_VERT_OFFSET) {
		grbl_send(CLIENT_SERIAL, "[MSG:Y move to high]\r\n");
		work_area_warning = true;
	}
	*/

	if (work_area_warning) {
		// raise a soft limit error
		return;
	}


	// =============== range checking !!! =========================

	

	// calculate cartesian move distance for each axis
	dx = target[X_AXIS] - position[X_AXIS];
	dy = target[Y_AXIS] - position[Y_AXIS];
	dz = target[Z_AXIS] - position[Z_AXIS];
	
	// calculate the total X,Y, Z axis move distance
	dist = sqrt( (dx * dx) + (dy * dy) + (dz * dz));
	
	// determine the segment count we need based on whether it is a G0 (rapid) or G1 move
	if (pl_data->condition & PL_COND_FLAG_RAPID_MOTION) {
		segment_count = 1;    // rapid G0 motion is not used to draw, so skip the segmentation
	} else {
		segment_count = ceil(dist / MAX_SEGMENT_LENGTH);    // determine the number of segments we need	... round up so there is at least 1
		dist /= segment_count;  // segment distance
	}
	
	cart_feedrate = pl_data->feed_rate; // save the original feedrate...it will be used to compensate
	
	for(uint32_t segment = 1; segment <= segment_count; segment++) {
		float dist_cart, dist_conv;  // used to comapre distance in cartesian to the new converted system for feed rate comp
		
		// determine this segment's target
		seg_target[X_AXIS] = position[X_AXIS] + (dx / float(segment_count) * segment) - x_offset;
		seg_target[Y_AXIS] = position[Y_AXIS] + (dy / float(segment_count) * segment) - y_offset;
		seg_target[Z_AXIS] = position[Z_AXIS] + (dz / float(segment_count) * segment) - z_offset;		
		
		converted_target[X_AXIS] = hypot_f(half_width + seg_target[X_AXIS], height - seg_target[Y_AXIS]); //- ARM_LENGTH_CAL;
		converted_target[Y_AXIS] = hypot_f(half_width - seg_target[X_AXIS], height - seg_target[Y_AXIS]); // - ARM_LENGTH_CAL;
		converted_target[Z_AXIS] = seg_target[Z_AXIS];
		
		// Deal with feed rate later
		//dist_conv = hypot_f(converted_target[X_AXIS] - position[X_AXIS], converted_target[Y_AXIS] - position[Y_AXIS]);  // distance of this segment in converted system		
		//grbl_sendf(CLIENT_SERIAL, "[MSG:Feedrate adjustment %2.3f]\r\n", (dist_conv / dist));
		//pl_data->feed_rate = cart_feedrate * (dist_conv / dist);  // apply the distance ratio between coord systems to the feed rate
		
		
		mc_line(converted_target, pl_data);
	}
	
	//mc_line(target, pl_data);
}

/*
	This returns the current position in the cartesian coordinate system.
	This can be used to display position data to the user.
	Grbl is working in belt lengths.
*/
void forward_kinematics(float *position)
{
	int32_t original_position_steps[N_AXIS]; // temporary storage of original in step
	float original_position_mm[N_AXIS];  // original postion in mm
	int32_t current_position[N_AXIS]; // Copy current state of the system position variable

	float ang_x, ang_y;
	
	memcpy(original_position_steps,sys_position,sizeof(sys_position));
	system_convert_array_steps_to_mpos(original_position_mm, original_position_steps);
	
    ang_x = get_angle(original_position_mm[Y_AXIS], original_position_mm[X_AXIS], wallbot_machine_width);
	//ang_y = get_angle(original_position_mm[X_AXIS], wallbot_machine_width,  original_position_mm[Y_AXIS]);




	// return in the new system
	
	position[X_AXIS] = (cos(ang_x) * original_position_mm[X_AXIS]) - (wallbot_machine_width / 2.0);
	position[X_AXIS] = (-settings.max_travel[Y_AXIS]) - (sin(ang_x) * original_position_mm[X_AXIS]); 
	position[Z_AXIS] = 0;  // unchanged

}

/*
	get angle using law of cosines
*/
float get_angle(float oppo, float right, float left) 
{
	float ang;

	ang = (left*left + right*right - oppo*oppo)/(2 * right * left);
	ang = acos(ang);

	return ang;

}


/*
	Pick your ideal homing speed and set both $25 and $24 to that speed
	Run a G1 move with that speed and look at the TSTEP values
	Set the TCOOLTHRS about 10% above that and THIGH about 10% below
	These values are in clock counts between steps on the drive, so
	a lower number represents a higher step rate.
*/
bool kinematics_homing(uint8_t cycle_mask) 
{
	//grbl_send(CLIENT_SERIAL, "[MSG:Kinematics homing]\r\n");

	// setup the homing mode motor settings
	TRINAMIC_X.TCOOLTHRS(HOMING_TCOOLTHRS) ; 
	TRINAMIC_X.THIGH(HOMING_THIGH);
	TRINAMIC_X.diag1_stall(1); // stallguard i/o is on diag1
	TRINAMIC_X.diag1_pushpull(0); // 0 = active low

	TRINAMIC_Y.TCOOLTHRS(HOMING_TCOOLTHRS) ; //(0xFFFFF); // 20bit max
	TRINAMIC_Y.THIGH(HOMING_THIGH);
	TRINAMIC_Y.diag1_stall(1); // stallguard i/o is on diag1
	TRINAMIC_Y.diag1_pushpull(0); // 0 = active low



	if (wallbot_homing(cycle_mask)) { // if successfully homes
		// return to the normal run settings
		TRINAMIC_X.TCOOLTHRS(NORMAL_TCOOLTHRS) ;
		TRINAMIC_X.THIGH(NORMAL_THIGH);
		TRINAMIC_X.diag1_stall(0); // stallguard i/o is not on diag1

		TRINAMIC_Y.TCOOLTHRS(NORMAL_TCOOLTHRS) ; //(0xFFFFF); // 20bit max
		TRINAMIC_Y.THIGH(NORMAL_THIGH);
		TRINAMIC_Y.diag1_stall(0); // stallguard i/o is not on diag1
	}
	else {
		set_stepper_disable(true); // disable motors to allow them to free wheel
	}

	return false; // false...we don't want to finish normal homing cycle
}


bool wallbot_homing(uint8_t cycle_mask) 
{		// home X_AXIS with belt installed in right clip

	//grbl_sendf(CLIENT_SERIAL, "[MSG:Homing Mask %d]\r\n", cycle_mask);

	// =================== home X axis ======================
	if (cycle_mask == HOMING_CYCLE_ALL ||  cycle_mask == HOMING_CYCLE_X) {
		limits_go_home(1<<X_AXIS);
		if (sys_rt_exec_alarm) {
			// if last homing failed quit
			grbl_send(CLIENT_SERIAL, "[MSG:Kinematics homing failure]\r\n");
			return false;	
		}
		
		// pause to give time for the user to release the belt pin
		grbl_send(CLIENT_SERIAL, "[MSG: Release belt from clip]\r\n");
		delay_ms(2000);
		
		// Home X_AXIS again to get travel distance.
		limits_go_home(1<<X_AXIS);
		if (sys_rt_exec_alarm) {		
			// if last homing failed quit
			return false;
		}

		//grbl_sendf(CLIENT_SERIAL, "[MSG: homing travel %4.3f Pulloff %4.3f Cal factor %4.3f]\r\n", last_homing_info.travel_dist[X_AXIS], settings.homing_pulloff, settings.machine_float[WIDTH_CALIBRATION_INDEX]);

		// after the belt homes in the socket it feeds out more by the pull off distance. That number can be changed
		// by the user, so it will be subtracted from move ditance
		wallbot_machine_width = last_homing_info.travel_dist[X_AXIS] - settings.homing_pulloff - settings.machine_float[WIDTH_CALIBRATION_INDEX]; // Note: max_travel is stored as a negative value
		grbl_sendf(CLIENT_SERIAL, "[MSG:Wallbot width value: %4.3f]\r\n", wallbot_machine_width);
		
		
	} // if homing X axis

	// ======================== Home Y_AXIS axis ==================

	if (cycle_mask == HOMING_CYCLE_ALL ||  cycle_mask == HOMING_CYCLE_Y) {
		limits_go_home(1<<Y_AXIS);
	}

	// now set the current machine position. Add add a compensation factor for the arms on the carriage
	sys_position[X_AXIS] = (settings.machine_float[WIDTH_CALIBRATION_INDEX] + settings.homing_pulloff) * settings.machine_float[ARM_LENGTH_CAL_INDEX]; // sys_position is in steps
	sys_position[Y_AXIS] = (settings.machine_float[WIDTH_CALIBRATION_INDEX] + settings.homing_pulloff) * settings.machine_float[ARM_LENGTH_CAL_INDEX]; // sys_position is in steps
	//sys_position[Z_AXIS] = 1.0 * settings.steps_per_mm[Y_AXIS];
	
	gc_sync_position();
	plan_sync_position();
			
	return true;
}

void user_defined_macro(uint8_t index)
{
	char gcode_line[20];
	
	switch (index) {
		#ifdef MACRO_BUTTON_0_PIN
		case CONTROL_PIN_INDEX_MACRO_0:
			grbl_send(CLIENT_ALL, "[[MSG: User Button Homing]\r\n");
			inputBuffer.push("$H\r");			
		break;
		#endif
		
		#ifdef MACRO_BUTTON_1_PIN
		case CONTROL_PIN_INDEX_MACRO_1:
			grbl_send(CLIENT_SERIAL, "[[MSG: User Button 1]\r\n");													
			inputBuffer.push("G0X0Y0\r");
		break;
		#endif
		
		#ifdef MACRO_BUTTON_2_PIN
		case CONTROL_PIN_INDEX_MACRO_2:
			grbl_send(CLIENT_SERIAL, "[[MSG: User Button 2]\r\n");		
			//inputBuffer.push("G0Y-25\r");
		break;
		#endif

		#ifdef MACRO_BUTTON_3_PIN
		case CONTROL_PIN_INDEX_MACRO_3:
			grbl_send(CLIENT_SERIAL, "[MSG: User Button...Disable Drivers]\r\n");
			set_stepper_disable(true);
		break;
		#endif	
		
		default:
			grbl_sendf(CLIENT_SERIAL, "[MSG: Unknown Switch %d]\r\n", index);
		break;
	}
}

#endif
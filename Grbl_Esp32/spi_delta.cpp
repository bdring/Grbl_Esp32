#include "grbl.h"

/*

	The motors use an angle for the units. The 0 angle is
	when the motor crank arm is horizontal. The positive direction
	is down.
	
	The end effector is always below the motors, so its Z is negative with respect
	to the motors.
*/

static float last_angle[N_AXIS] = {0.0, 0.0, 0.0}; // save the last motor angles for distance calcs

#ifdef CPU_MAP_SPI_DELTA

 // a bunch of precalculated math ... change to defines soon
 const float sqrt3 = 1.732050807;
 const float pi = 3.141592653;    // PI
 const float dtr = pi/(float)180.0;
 const float sin120 = sqrt3/2.0;   
 const float cos120 = -0.5;        
 const float tan60 = sqrt3;
 const float sin30 = 0.5;
 const float tan30 = 1.0/sqrt3;

 // the geometry of the robot
 const float rf = 100; // radius (length) of the motor crank arms
 const float re = 220; // radius (length) of the linkages

const float f = 294.449; // triangle (length) of the head (motor mount)
const float e = 86.6025f; // ....triangle (length) of the end effector

float delta_z_offset; // the position of Z in relation to 0,0,0 in motor angles

static TaskHandle_t readSgTaskHandle = 0;   // for optional realtime stallguard data diaplay

// initialize the specific features of this machine. This happens once at startup
void machine_init()
{		
	
	// determine where z would be (delta_z_offset) when the motor angles are all zero
	float x0, y0;
	delta_calcForward(0.0, 0.0, 0.0, x0, y0, delta_z_offset) ;	
	grbl_sendf(CLIENT_SERIAL, "[MSG:Zero Angs in Cart X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", x0, y0, delta_z_offset);
	

	// setup a task that will display stallguard info when that feature is enabled		
	xTaskCreatePinnedToCore(	readSgTask,    // task
								"readSgTask", // name for task
								4096,   // size of task stack
								NULL,   // parameters
								1, // priority
								&readSgTaskHandle,
								0 // core
							);
}

// read and display the stallguard information when enabled
void readSgTask(void *pvParameters)
{		
	TickType_t xLastWakeTime;
	const TickType_t xreadSg = 100;  // in ticks between readings (typically ms)
	uint32_t x_tstep, y_tstep, z_tstep;
	uint8_t x_sg, y_sg, z_sg;

	xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
	while(true) { // don't ever return from this or the task dies
		if (bit_istrue(settings.flags,BITFLAG_LASER_MODE)) { // use laser mode as a way to turn off this data

				// read from drivers
				x_tstep = TRINAMIC_X.TSTEP();
				x_sg = TRINAMIC_X.sg_result();
				y_tstep = TRINAMIC_Y.TSTEP();
				y_sg = TRINAMIC_Y.sg_result();
				z_tstep = TRINAMIC_Z.TSTEP();
				z_sg = TRINAMIC_Z.sg_result();
				
				// only display if the motor is running
				if (x_tstep != 0xFFFFF && x_tstep != 0) { // used for homing just need to check X axis 
					grbl_sendf(CLIENT_ALL, 
								"SG Debug   SGX:%03d TSX:%07d   SGY:%3d TSY:%07d   SGZ:%3d TSZ:%07d\r\n",
								x_sg, x_tstep, y_sg, y_tstep, z_sg, z_tstep);
				}				
		}						
		vTaskDelayUntil(&xLastWakeTime, xreadSg);
    }	
}

// this init replaces the basic Trinamic one in Grbl so we can exactly setup our motors
void machine_trinamic_setup() 
{
	uint8_t testResult;
	
	grbl_send(CLIENT_SERIAL, "[MSG:Machine Trinamic Setup]\r\n");
	TRINAMIC_X.begin(); // Initiate pins and registries
	trinamic_test_response(testResult, "X");
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
	trinamic_test_response(testResult, "Y");
	TRINAMIC_Y.tbl(1);
	TRINAMIC_Y.toff(3);
	//driver.pwm_autoscale(false);
	//TRINAMIC_X.blank_time(24);	
	TRINAMIC_Y.microsteps(settings.microsteps[Y_AXIS]);
	TRINAMIC_Y.rms_current(settings.current[Y_AXIS] * 1000.0, settings.hold_current[Y_AXIS]/100.0);	
	TRINAMIC_Y.TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
	TRINAMIC_Y.THIGH(NORMAL_THIGH);
	TRINAMIC_Y.hysteresis_start(4);
	TRINAMIC_Y.hysteresis_end(-2);	
	//TRINAMIC_Y.diag1_stall(1); // stallguard i/o is on diag1
	//TRINAMIC_Y.diag1_pushpull(0); // 0 = active low
	//
	TRINAMIC_Z.sfilt(1);   
	//TRINAMIC_Z.semin(5);
	//TRINAMIC_Z.semax(2);
	//TRINAMIC_Z.sedn(0b01);
	TRINAMIC_Z.sgt(settings.stallguard[Z_AXIS]);
	
	TRINAMIC_Z.begin(); // Initiate pins and registries
	trinamic_test_response(testResult, "Z");
	TRINAMIC_Z.tbl(1);
	TRINAMIC_Z.toff(3);
	//driver.pwm_autoscale(false);
	//TRINAMIC_Z.blank_time(24);	
	TRINAMIC_Z.microsteps(settings.microsteps[Z_AXIS]);
	TRINAMIC_Z.rms_current(settings.current[Z_AXIS] * 1000.0, settings.hold_current[Z_AXIS]/100.0);	
	TRINAMIC_Z.TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
	TRINAMIC_Z.THIGH(NORMAL_THIGH);
	TRINAMIC_Z.hysteresis_start(4);
	TRINAMIC_Z.hysteresis_end(-2);	
	//TRINAMIC_Z.diag1_stall(1); // stallguard i/o is on diag1
	//TRINAMIC_Z.diag1_pushpull(0); // 0 = active low
	//
	TRINAMIC_Z.sfilt(1);   
	//TRINAMIC_Z.semin(5);
	//TRINAMIC_Z.semax(2);
	//TRINAMIC_Z.sedn(0b01);
	TRINAMIC_Z.sgt(settings.stallguard[Z_AXIS]);

}

/*
	convert desired cartesian moves of the end effector to delta motor movements.
	This is done by breaking the moves into tiny segment and converting the endpoint to motor angles.
	The feedrate also needs to be converted
	All cnversions are done in machine space. This prevents G5x offsets from messing things up
	This function gets called by Grbl	
*/
void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position) //The target and position are provided in machine space
{	
	//grbl_sendf(CLIENT_SERIAL, "[MSG:Start X:%4.3f Y:%4.3f Z:%4.3f]\r\n", position[X_AXIS], position[Y_AXIS], position[Z_AXIS]);
	//grbl_sendf(CLIENT_SERIAL, "[MSG:Target X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", target[X_AXIS], target[Y_AXIS], target[Z_AXIS]);

	float dx, dy, dz; // distances in each cartesian axis
	float theta1, theta2, theta3; // returned angles from calculations
	float motor_angles[N_AXIS];

	float dist; // total cartesian distance
	float segment_dist; // segment cartesian distance
	float delta_distance; // the"distance" from prev angle to next angle

	uint32_t segment_count;  // number of segments the move will be broken in to.
	
	float seg_target[N_AXIS]; // The target of the current segment
	
	// adjust the end effector location
	//target[Z_AXIS] += delta_z_offset; // Note: For typical (fixed top) deltas, the offset is negative

	// calculate cartesian move distance for each axis
	dx = target[X_AXIS] - position[X_AXIS];
	dy = target[Y_AXIS] - position[Y_AXIS];
	dz = target[Z_AXIS] - position[Z_AXIS];

	// calculate the total X,Y,Z axis move distance
	dist = sqrt( (dx * dx) + (dy * dy) + (dz * dz));

	segment_count = ceil(dist / SEGMENT_LENGTH);    // determine the number of segments we need	... round up so there is at least 1 (except when dist is 0)

	segment_dist /= segment_count;  // distanse of each segment...will be used for feedrate conversion

	for(uint32_t segment = 1; segment <= segment_count; segment++) {
		// determine this segment's target
		seg_target[X_AXIS] = position[X_AXIS] + (dx / float(segment_count) * segment);
		seg_target[Y_AXIS] = position[Y_AXIS] + (dy / float(segment_count) * segment);
		seg_target[Z_AXIS] = position[Z_AXIS] + (dz / float(segment_count) * segment);
		
		//grbl_sendf(	CLIENT_SERIAL, "[MSG:Kin Segment Target Cart X:%4.3f Y:%4.3f Z:%4.3f]\r\n",seg_target[X_AXIS], seg_target[Y_AXIS],seg_target[Z_AXIS]);
		
		int status = delta_calcInverse(seg_target[X_AXIS] , seg_target[Y_AXIS] , seg_target[Z_AXIS] + delta_z_offset, motor_angles[0], motor_angles[1], motor_angles[2]);
		
		if(status == KIN_ANGLE_CALC_OK) {
			//grbl_sendf(	CLIENT_SERIAL, "[MSG:Kin Segment Target Angs X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", motor_angles[0], motor_angles[1], motor_angles[2]);
			
			motor_angles[0] *= dtr;
			motor_angles[1] *= dtr;
			motor_angles[2] *= dtr;
			
			
			dist = sqrt(    ((motor_angles[0] - last_angle[0]) * (motor_angles[0] - last_angle[0])) + 
							((motor_angles[1] - last_angle[1]) * (motor_angles[1] - last_angle[1])) +
							((motor_angles[2] - last_angle[2]) * (motor_angles[2] - last_angle[2])));
			
			
			// todo use an memcpy
			last_angle[0] = motor_angles[0];
			last_angle[1] = motor_angles[1];
			last_angle[2] = motor_angles[2];
			
			//calc_polar(seg_target, polar, last_angle);
			
			// calculate the targets for the delta
			
			// determine the move distance in the angular units
			
			// determine the new feed rate (if G1)
			
			// begin determining new feed rate		
					
			mc_line(motor_angles, pl_data);
			
		}
		else {
			//grbl_sendf(	CLIENT_SERIAL, "[MSG:Kin Angle Error]\r\n\r\n");
		}
		
		
	}
	//mc_line(target, pl_data);
	
	// TO DO don't need a feedrate for rapids
}



 // inverse kinematics: (x0, y0, z0) -> (theta1, theta2, theta3)
 // returned status: 0=OK, -1=non-existing position
 int delta_calcInverse(float x0, float y0, float z0, float &theta1, float &theta2, float &theta3) {
	 //grbl_sendf(CLIENT_SERIAL, "[MSG:Calc X:%4.3f Y:%4.3f Z:%4.3f]\r\n\r\n", x0, y0, z0);			
			
     theta1 = theta2 = theta3 = 0;
	 int status;
	 
	status = delta_calcAngleYZ(x0, y0, z0, theta1);

	if (status != KIN_ANGLE_CALC_OK) {
		grbl_sendf(	CLIENT_SERIAL, "[MSG:theta1 Error]\r\n");
		return status;
	}
		
	
    status = delta_calcAngleYZ(x0*cos120 + y0*sin120, y0*cos120-x0*sin120, z0, theta2);  // rotate coords to +120 deg

    if (status != KIN_ANGLE_CALC_OK) {
		grbl_sendf(	CLIENT_SERIAL, "[MSG:theta2 Error]\r\n");
		return status;
	}
		
	
    status = delta_calcAngleYZ(x0*cos120 - y0*sin120, y0*cos120+x0*sin120, z0, theta3);  // rotate coords to -120 deg

	if (status != KIN_ANGLE_CALC_OK) {
		grbl_sendf(	CLIENT_SERIAL, "[MSG:theta3 Error]\r\n");
	}
	
    return status;
}

 // helper functions, calculates angle theta1 (for YZ-pane)
 int delta_calcAngleYZ(float x0, float y0, float z0, float &theta) {
     float y1 = -0.5 * 0.57735 * f; // f/2 * tg 30
     y0 -= 0.5 * 0.57735    * e;    // shift center to edge
     // z = a + b*y
     float a = (x0*x0 + y0*y0 + z0*z0 +rf*rf - re*re - y1*y1)/(2*z0);
     float b = (y1-y0)/z0;
     // discriminant
     float d = -(a+b*y1)*(a+b*y1)+rf*(b*b*rf+rf); 
     if (d < 0) return -1; // non-existing point
     float yj = (y1 - a*b - sqrt(d))/(b*b + 1); // choosing outer point
     float zj = a + b*yj;
     theta = 180.0*atan(-zj/(y1 - yj))/pi + ((yj>y1)?180.0:0.0);
     return 0;
 }

void forward_kinematics(float *position)
{
	
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

	uint16_t tcoolthrs = settings.machine_int16[0]; // $80 setting
	uint16_t thigh = settings.machine_int16[1]; // $81 setting

	// setup the homing mode motor settings
	TRINAMIC_X.sgt(settings.stallguard[X_AXIS]);
	TRINAMIC_X.TCOOLTHRS(tcoolthrs) ; 
	TRINAMIC_X.THIGH(thigh);
	TRINAMIC_X.diag1_stall(1); // stallguard i/o is on diag1
	TRINAMIC_X.diag1_pushpull(0); // 0 = active low

	TRINAMIC_Y.sgt(settings.stallguard[Y_AXIS]);
	TRINAMIC_Y.TCOOLTHRS(tcoolthrs) ; //(0xFFFFF); // 20bit max
	TRINAMIC_Y.THIGH(thigh);
	TRINAMIC_Y.diag1_stall(1); // stallguard i/o is on diag1
	TRINAMIC_Y.diag1_pushpull(0); // 0 = active low

	TRINAMIC_Z.sgt(settings.stallguard[Z_AXIS]);
	TRINAMIC_Z.TCOOLTHRS(tcoolthrs) ; //(0xFFFFF); // 20bit max
	TRINAMIC_Z.THIGH(thigh);
	TRINAMIC_Z.diag1_stall(1); // stallguard i/o is on diag1
	TRINAMIC_Z.diag1_pushpull(0); // 0 = active low

	//grbl_sendf(CLIENT_SERIAL, "[MSG:Kinematics homing %d %d %d]\r\n", tcoolthrs, thigh, settings.stallguard[Z_AXIS]);

	if (spi_delta_homing(cycle_mask)) { // if successfully homes
		// return to the normal run settings
		TRINAMIC_X.TCOOLTHRS(NORMAL_TCOOLTHRS) ;
		TRINAMIC_X.THIGH(NORMAL_THIGH);
		TRINAMIC_X.diag1_stall(0); // stallguard i/o is not on diag1

		TRINAMIC_Y.TCOOLTHRS(NORMAL_TCOOLTHRS) ; //(0xFFFFF); // 20bit max
		TRINAMIC_Y.THIGH(NORMAL_THIGH);
		TRINAMIC_Y.diag1_stall(0); // stallguard i/o is not on diag1
		
		TRINAMIC_Z.TCOOLTHRS(NORMAL_TCOOLTHRS) ; //(0xFFFFF); // 20bit max
		TRINAMIC_Z.THIGH(NORMAL_THIGH);
		TRINAMIC_Z.diag1_stall(0); // stallguard i/o is not on diag1
		
		// set machine position to 0
		sys_position[X_AXIS] = 0;
		sys_position[Y_AXIS] = 0;
		sys_position[Z_AXIS] = 0;

		gc_sync_position();
		plan_sync_position();
		
	}
	else {
		set_stepper_disable(true); // disable motors to allow them to free wheel
	}

	return false; // false...we don't want to finish normal homing cycle
}

bool kinematics_pre_homing(uint8_t cycle_mask) {
	//grbl_sendf(CLIENT_SERIAL, "[MSG:Kinematics pre-homing %d]\r\n", cycle_mask);
	kinematics_homing(cycle_mask);
	return true; 
}

void kinematics_post_homing() {
	//grbl_send(CLIENT_SERIAL, "[MSG:Kinematics post-homing]\r\n");
}
 
	
bool spi_delta_homing(uint8_t cycle_mask)
{
	//grbl_sendf(CLIENT_SERIAL, "[MSG:spi_delta_homing %d]\r\n", cycle_mask);

	// =================== home X axis ======================
	if (cycle_mask == HOMING_CYCLE_ALL) {
		//grbl_sendf(CLIENT_SERIAL, "[MSG:spi_delta_homing %d]\r\n", cycle_mask);
		limits_go_home(HOMING_CYCLE_0);
		
		if (sys_rt_exec_alarm) {
			// if last homing failed quit
			//grbl_send(CLIENT_SERIAL, "[MSG:Kinematics homing failure]\r\n");
			return false;	
		}
	}
	
	gc_sync_position();
	plan_sync_position();
			
	return true;
}

int delta_calcForward(float theta1, float theta2, float theta3, float &x0, float &y0, float &z0) {
        

     float t = (f-e)*tan30/2;
     
 
     theta1 *= dtr;
     theta2 *= dtr;
     theta3 *= dtr;
 
     float y1 = -(t + rf*cos(theta1));
     float z1 = -rf*sin(theta1);
 
     float y2 = (t + rf*cos(theta2))*sin30;
     float x2 = y2*tan60;
     float z2 = -rf*sin(theta2);
 
     float y3 = (t + rf*cos(theta3))*sin30;
     float x3 = -y3*tan60;
     float z3 = -rf*sin(theta3);
 
     float dnm = (y2-y1)*x3-(y3-y1)*x2;
 
     float w1 = y1*y1 + z1*z1;
     float w2 = x2*x2 + y2*y2 + z2*z2;
     float w3 = x3*x3 + y3*y3 + z3*z3;
     
     // x = (a1*z + b1)/dnm
     float a1 = (z2-z1)*(y3-y1)-(z3-z1)*(y2-y1);
     float b1 = -((w2-w1)*(y3-y1)-(w3-w1)*(y2-y1))/2.0;
 
     // y = (a2*z + b2)/dnm;
     float a2 = -(z2-z1)*x3+(z3-z1)*x2;
     float b2 = ((w2-w1)*x3 - (w3-w1)*x2)/2.0;
 
     // a*z^2 + b*z + c = 0
     float a = a1*a1 + a2*a2 + dnm*dnm;
     float b = 2*(a1*b1 + a2*(b2-y1*dnm) - z1*dnm*dnm);
     float c = (b2-y1*dnm)*(b2-y1*dnm) + b1*b1 + dnm*dnm*(z1*z1 - re*re);
  
     // discriminant
     float d = b*b - (float)4.0*a*c;
     if (d < 0) return -1; // non-existing point
 
     z0 = -(float)0.5*(b+sqrt(d))/a;
     x0 = (a1*z0 + b1)/dnm;
     y0 = (a2*z0 + b2)/dnm;
     return 0;
 }
 
 float three_axis_dist(float *pt1, float *pt2)
 {
	 return sqrt(	((pt1[0] - pt2[0]) * (pt1[0] - pt2[0])) + 
					((pt1[1] - pt2[1]) * (pt1[1] - pt2[1])) +
					((pt1[2] - pt2[2]) * (pt1[2] - pt2[2])));
 }

#endif
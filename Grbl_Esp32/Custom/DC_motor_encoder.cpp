/*
	DCServo.cpp 
	Normal Flat CNC working with steper motor driver.
	I want Homing positions in 4 courners for 4 part mashining. 
	My stepper drivers got "I can't keep up" signalisation. 

	Part of Grbl_ESP32
	copyright (c) 2020 -	Bart Dring. This file was intended for use on the ESP32
  ...add your date and name here.
	CPU. Do not use this with Grbl for atMega328P
	Grbl_ESP32 is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	Grbl_ESP32 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
	=======================================================================
This is a template for user-defined C++ code functions.  Grbl can be
configured to call some optional functions, enabled by #define statements
in the machine definition .h file.  Implement the functions thus enabled
herein.  The possible functions are listed and described below.
To use this file, copy it to a name of your own choosing, and also copy
Machines/template.h to a similar name.
Example:
Machines/my_machine.h
Custom/my_machine.cpp
Edit machine.h to include your Machines/my_machine.h file
Edit Machines/my_machine.h according to the instructions therein.
Fill in the function definitions below for the functions that you have
enabled with USE_ defines in Machines/my_machine.h
===============================================================================
*/

#ifdef USE_MACHINE_INIT
/*
machine_init() is called when Grbl_ESP32 first starts. You can use it to do any
special things your machine needs at startup.

1.Setup the pins
2.Do nothing more
*/
void machine_init()
{
	grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DCServo cnc initialization");
//PIN INITIALIZATION
pinMode(X_STEP_PIN,OUTPUT);              
pinMode(X_DIRECTION_PIN,OUTPUT);        
pinMode(Y_STEP_PIN,OUTPUT);              
pinMode(Y_DIRECTION_PIN,OUTPUT);          
pinMode(Z_STEP_PIN,OUTPUT);             
pinMode(Z_DIRECTION_PIN,OUTPUT);        
pinMode(A_STEP_PIN,OUTPUT);              
pinMode(A_DIRECTION_PIN,OUTPUT);         

pinMode(X_HOMED_PIN,INPUT);            
pinMode(Y_HOMED_PIN,INPUT);           
pinMode(Z_HOMED_PIN,INPUT);           
pinMode(A_HOMED_PIN,INPUT); 
          
pinMode(X_LIMIT_PIN,INPUT);           
pinMode(X_two_LIMIT_PIN,INPUT);       

pinMode(Y_LIMIT_PIN,INPUT);             
pinMode(Y_two_LIMIT_PIN,INPUT);       

pinMode(Z_LIMIT_PIN,INPUT);         
pinMode(Z_two_LIMIT_PIN,INPUT);     

pinMode(A_LIMIT_PIN,INPUT);     
pinMode(A_two_LIMIT_PIN,INPUT);      

pinMode(SPINDLE_OUTPUT_PIN,OUTPUT);  //now im using SSR to run vacom and spindel

pinMode(COOLANT_MIST_PIN,OUTPUT);     
pinMode(COOLANT_FLOOD_PIN,OUTPUT);  
pinMode(PROBE_PIN,INPUT_PULLUP);  

}
#endif


#ifdef USE_MOTOR_TEST
/*DCServo got 2400PPR encoder (600 positions) so 2400 pulses will rotate it by 360*
1. Run motor X for 2400 steps back and fwd
2. Run motor Y for 2400 steps back and fwd
3. Run motor Z for 2400 steps back and fwd
4. Run motor A for 2400 steps back and fwd

In case it hits a wall or somth just break procedure. 
*/
void motor_test(){
	pinMode(X_DIRECTION_PIN,HIGH);
	for(int i=0;i<2400;i++){
		pinMode(X_STEP_PIN,HIGH);
		delay(10);
		pinMode(X_STEP_PIN,LOW);
		delay(10);
	}
	pinMode(X_DIRECTION_PIN,LOW);
	for(int i=0;i<2400;i++){
		pinMode(X_STEP_PIN,HIGH);
		delay(10);
		pinMode(X_STEP_PIN,LOW);
		delay(10);
	}
	
	pinMode(Y_DIRECTION_PIN,HIGH);
	for(int i=0;i<2400;i++){
		pinMode(Y_STEP_PIN,HIGH);
		delay(10);
		pinMode(Y_STEP_PIN,LOW);
		delay(10);
	}
	pinMode(Y_DIRECTION_PIN,LOW);
	for(int i=0;i<2400;i++){
		pinMode(Y_STEP_PIN,HIGH);
		delay(10);
		pinMode(Y_STEP_PIN,LOW);
		delay(10);
	}
	
	pinMode(Z_DIRECTION_PIN,HIGH);
	for(int i=0;i<2400;i++){
		pinMode(Z_STEP_PIN,HIGH);
		delay(10);
		pinMode(Z_STEP_PIN,LOW);
		delay(10);
	}
	pinMode(Z_DIRECTION_PIN,LOW);
	for(int i=0;i<2400;i++){
		pinMode(Z_STEP_PIN,HIGH);
		delay(10);
		pinMode(Z_STEP_PIN,LOW);
		delay(10);
	}
	
	pinMode(A_DIRECTION_PIN,HIGH);
	for(int i=0;i<2400;i++){
		pinMode(A_STEP_PIN,HIGH);
		delay(10);
		pinMode(A_STEP_PIN,LOW);
		delay(10);
	}
	pinMode(A_DIRECTION_PIN,LOW);
	for(int i=0;i<2400;i++){
		pinMode(A_STEP_PIN,HIGH);
		delay(10);
		pinMode(A_STEP_PIN,LOW);
		delay(10);
	}

}
#endif
	


#ifdef USE_CUSTOM_HOMING
/*
  user_defined_homing() is called at the begining of the normal Grbl_ESP32 homing
  sequence.  If user_defined_homing() returns true, the rest of normal Grbl_ESP32
  homing is skipped if it returns false, other normal homing continues.  For
  example, if you need to manually prep the machine for homing, you could implement
  user_defined_homing() to wait for some button to be pressed, then return true.
  
  It's prepared for using mostly with EASEL Online. It got botom left start position. I don't know how homing will work jet, but i think default function is OK.
*/
bool user_defined_homing()
{
  // True = done with homing, false = continue with normal Grbl_ESP32 homing
  
  return false;
}
#endif


#ifdef USE_CALIBRATION_PROCEDURE
/* My mashine got 2 proximity switches for every axis. 
	I want it to calibrate like an elevator. Then it should print out mesuring how much steps it takes to cover distance between switches. 
	Calibration algorithm:
1. Rise Z axis til hit proximity switch 
2. Move X and Y til hit proximity switches
3. Rotate A axis
3. Compute max feedrate of every axis. 

If during calibration I've get keepin up from homing pin, stop designed axis and then retract by few steps and try again. 

  */


void print_calibration(){
{ 
	unsigned int delay_betwen_steps_x = 1; //Variable used to count max feedrate
	unsigned int delay_betwen_steps_y = 1; //is rised everytime *_HOMED_PIN is rising by 1
	unsigned int delay_betwen_steps_z = 1; //i still need to find formula for feedrate from this xD
	unsigned int delay_betwen_steps_a = 1;
	
 grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Calibration!");
	grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Rising Z axis");
	while(digitalRead(
	
	grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Moving X axis");
	grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Moving Y axis");
	//grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Moving A axis");
	
}
#endif


#ifdef USE_KINEMATICS
/*
  Inverse Kinematics converts X,Y,Z cartesian coordinate to the steps
  on your "joint" motors.  It requires the following three functions:
*/

/*
  inverse_kinematics() looks at incoming move commands and modifies
  them before Grbl_ESP32 puts them in the motion planner.
  Grbl_ESP32 processes arcs by converting them into tiny little line segments.
  Kinematics in Grbl_ESP32 works the same way. Search for this function across
  Grbl_ESP32 for examples. You are basically converting cartesian X,Y,Z... targets to
    target = an N_AXIS array of target positions (where the move is supposed to go)
    pl_data = planner data (see the definition of this type to see what it is)
    position = an N_AXIS array of where the machine is starting from for this move
*/
void inverse_kinematics(float *target, plan_line_data_t *pl_data, float *position)
{
  // this simply moves to the target. Replace with your kinematics.
  mc_line(target, pl_data);
}

/*
  kinematics_pre_homing() is called before normal homing
  You can use it to do special homing or just to set stuff up
  cycle_mask is a bit mask of the axes being homed this time.
*/
bool kinematics_pre_homing(uint8_t cycle_mask))
{
  return false; // finish normal homing cycle
}

/*
  kinematics_post_homing() is called at the end of normal homing
*/
void kinematics_post_homing()
{
}
#endif

#ifdef USE_FWD_KINEMATIC
/*
  The status command uses forward_kinematics() to convert
  your motor positions to cartesian X,Y,Z... coordinates.
  Convert the N_AXIS array of motor positions to cartesian in your code.
*/
void forward_kinematics(float *position)
{
  // position[X_AXIS] =
  // position[Y_AXIS] =
}
#endif

#ifdef USE_TOOL_CHANGE
/*
  user_tool_change() is called when tool change gcode is received,
  to perform appropriate actions for your machine.
*/
void user_tool_change(uint8_t new_tool)
{
}
#endif

#if defined(MACRO_BUTTON_0_PIN) || defined(MACRO_BUTTON_1_PIN) || defined(MACRO_BUTTON_2_PIN)
/*
  options.  user_defined_macro() is called with the button number to
  perform whatever actions you choose.
*/
void user_defined_macro(uint8_t index)
{
}
#endif

#ifdef USE_M30
/*
  user_m30() is called when an M30 gcode signals the end of a gcode file.
*/
void user_m30()
{
}
#endif

#ifdef USE_MACHINE_TRINAMIC_INIT
/*
  machine_triaminic_setup() replaces the normal setup of trinamic
  drivers with your own code.  For example, you could setup StallGuard
*/
void machine_trinamic_setup()
{
}
#endif

#ifdef USE_MOTOR_TEST

// If you add any additional functions specific to your machine that
// require calls from common code, guard their calls in the common code with
// #ifdef USE_WHATEVER and add function prototypes (also guarded) to grbl.h

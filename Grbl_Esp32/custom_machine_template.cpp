/*
	custom_machine_template.cpp
	Part of Grbl_ESP32

	copyright (c) 2020 -	Bart Dring. This file was intended for use on the ESP32
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

This is a template of a custom machine file. All of these functions are called by
main Grbl_ESP when enabled via #define statements. The machine designer must fill 
in the contents of the functions. Almost all of them are optional. Remove any 
unused functions. See each function for more information

Make copies of custom_machine_template.cpp and custom_machine_template.h
and replace the file prefix with your machine's name

Example:
my_machine.h
my_machine.cpp

In cpu_map.h you need to create #include links to your new machine files. This is 
done using a CPU_MAP name so that you can switch to it in config.h like any of 
the other defined machine. See the template example at the bottom of cpu_map.h

Example
#ifdef CPU_MAP_MY_MACHINE
	#include my_machine.h
#endif

in config.h make sure CPU_MAP_MY_MACHINE is the only define cpu map. 

To keep your machine organized and cpu_map.h clean, put all of the stuff normally put in 
cpu_map.h in your my_machine.h file. There are sections in that template expaining it.

===============================================================================

*/

#ifdef CPU_MAP_CUSTOM_MACHINE // !!! Change this name to your machine map name
/*
This function is called if you have #define USE_MACHINE_INIT in your my_machine.h file
This function will be called when Grbl_ESP32 first starts. You can use it to do any
special things your machine needs at startup.
*/
void machine_init() {
	
}

/*
This function is called if you have #define USE_CUSTOM_HOMING in your my_machine.h file
This function gets called at the begining of the normal Grbl_ESP32 homing sequence. You 
Can skip the rest of normal Grbl_ESP32 homing by returning false. You return true if you 
want normal homing to continue. You might do this if you just need to prep the machine 
for homing.
*/
bool user_defined_homing() {
	return true; // True = done with homing, false = continue with normal Grbl_ESP32 homing
}


/*
Inverse Kinematics converts X,Y,Z cartesian coordinate to the steps on your "joint"
motors.

This function allows you to look at incoming move commands and modify them before
Grbl_ESP32 puts them in the motion planner.

Grbl_ESP32 processes arc by converting them into tiny little line segments. 
Kinematics in Grbl_ESP32 works the same way. Search for this function across Grbl_ESP32 
for examples. You are basically converting cartesian X,Y,Z... targets to 

target = an N_AXIS array of target positions (where the move is supposed to go)
pl_data = planner data (see the definition of this type to see what it is)
position = an N_AXIS array of where the machine is starting from for this move
*/
void inverse_kinematics(target, pl_data, position) {

	mc_line(target, pl_data); // this simply moves to the target Replace with your kinematics.
}

/*
Forward Kinematics converts your motor postions to X,Y,Z... cartesian information. 
This is used by the status command. 

Convert the N_AXIS array of motor positions to cartesian in your code. 

*/
void forward_kinematics(float *position) {

	// position[X_AXIS] = 
	// position[Y_AXIS] = 
}

/*
 This function is required if you have #define USE_KINEMATIC
 This function is called before normal homing
 You can use it to do special homing or just to set stuff up 
 
 cycle_mask = is a bit mask of the axes being homed this time.
 
*/
bool kinematics_pre_homing(cycle_mask)) {
	return false; // finish normal homing cycle
}



/* 
	This function is required if you have #define USE_KINEMATIC
	It is called at the end of normal homing
	
*/
void kinematics_post_homing() {
	
}

/*
	This function is called if #USE_TOOL_CHANGE is define and
	a gcode for a tool change is received

*/
void user_tool_change(uint8_t new_tool) {
	
}

/* 
	This will be called if any of the #define MACRO_BUTTON_0_PIN options
	are defined
*/
void user_defined_macro(uint8_t index)
{
}

/*
	This function is called if #define USE_M30 is defined and 
	an M30 gcode is received. M30 signals the end of a gcode file.
	
		
*/
void user_m30() {
}

/*
	Enable this function with #define USE_MACHINE_TRINAMIC_INIT
	This will replace the normal setup of trinamic drivers with your own
	This is where you could setup StallGaurd
	
*/
void machine_trinamic_setup() {
	
}


// feel free to add any additional functions specific to your machine


#endif
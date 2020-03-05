/*
	custom_machine_template.h
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

	====================================================================

	See custom_machine_templete.cpp for getting started creating custom
	machines
*/

// =============== MACHINE NAME ========================
// Change "MACHINE_CUSTOM", replacing "CUSTOM" with your chosen name

// #define MACHINE_NAME 		"MACHINE_CUSTOM"

// ================ config.h overrides ====================================
// If you want to make some changes to config.h, it is best to do it here
// if possible so all your changes are in your files.  Many of the #defines
// in config.h are only applied if the symbol is not already defined in
// a machine definition file.  Example:

// #define N_AXIS 4

// =============== MACHINE CONFIGURATION ========================
// Look at the other files in the Machines/ directory
// for all the things that can go here

// Assign CPU pins to machine functions, for example:
// #define X_STEP_PIN              GPIO_NUM_12
// #define X_DIRECTION_PIN         GPIO_NUM_26
// #define X_AXIS_LIMIT_PIN        GPIO_NUM_2

// #define SPINDLE_PWM_PIN         GPIO_NUM_17

// #define CONTROL_SAFETY_DOOR_PIN GPIO_NUM_35

// Set the number of bits to the number of limit switches
// #define LIMIT_MASK      	B111

// ============== Enable custom features =======================

// There are several customization functions that the code only
// calls if the symbols below are defined.  To use such a function,
// you must define the symbol herein, and define the function in
// your my_machine.cpp file.

// #define USE_MACHINE_INIT  // Enables use of custom machine_init()
// #define USE_CUSTOM_HOMING // Enables use of user_defined_homing()
// #define USE_KINEMATICS    // Enables use of inverse_kinematics(), kinematics_pre_homing(), and kinematics_post_homing()
// #define USE_FWD_KINEMATIC // Enables use of forward_kinematics()
// #define USE_TOOL_CHANGE   // Enables use off user_tool_change()
// #define USE_M30           // Enables use of user_m30()
// #define USE_TRIAMINIC     // Enables use of functions shown in grbl_triaminic.h
// #define USE_MACHINE_TRINAMIC_INIT  // Enables use of machine_triaminic_setup()

// ===================== $$ Defaults ==========================================
// defaults.h defines default values for the $$ settings that will be
// applied when you upload new firmware or if you reset them with $RST=$.
// You can override individual defaults by defining them here.  Examples:

// #define DEFAULT_SPINDLE_FREQ 2000.0
// #define DEFAULT_X_MAX_TRAVEL 100.0


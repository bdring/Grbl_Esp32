/*
	atari_1020.h
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

	This contains all the special features required to control an 
	Atari 1010 Pen Plotter
*/

#define SOLENOID_PWM_FREQ 5000
#define SOLENOID_PWM_RES_BITS 8

#define SOLENOID_PULSE_LEN_PULL 255
#define SOLENOID_PULL_DURATION 50 // in task counts...after this delay power will change to hold level see SOLENOID_TASK_FREQ
#define SOLENOID_PULSE_LEN_HOLD 40 // solenoid hold level ... typically a lower value to prevent overheating

#define SOLENOID_TASK_FREQ 50  // this is milliseconds

#define MAX_PEN_NUMBER			4
#define BUMPS_PER_PEN_CHANGE	3


#define ATARI_HOME_POS -10.0f // this amound to the left of the paper 0
#define ATARI_PAPER_WIDTH 100.0f //
#define ATARI_HOMING_ATTEMPTS 13

// tells grbl we have some special functions to call
#define USE_MACHINE_INIT
#define USE_CUSTOM_HOMING
#define USE_TOOL_CHANGE
#define ATARI_TOOL_CHANGE_Z 5.0
#define USE_M30 // use the user defined end of program

#ifndef atari_h
	#define atari_h

	void machine_init();
	void solenoid_disable();
	void solenoidSyncTask(void *pvParameters);
	void calc_solenoid(float penZ);
	bool user_defined_homing();
	void atari_home_task(void *pvParameters);
	void user_tool_change(uint8_t new_tool);
	void user_defined_macro(uint8_t index);
	void user_m30();
	void atari_next_pen();
	
#endif
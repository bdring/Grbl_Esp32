#pragma once

/*
	Motors.h
	Header file for Motor Classes
	Here is the hierarchy
		Motor
			Nullmotor
			StandardStepper
				TrinamicDriver
			Unipolar
			RC Servo

	These are for motors coordinated by Grbl_ESP32
	See motorClass.cpp for more details

	Part of Grbl_ESP32
	2020 -	Bart Dring

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

#include <cstdint>

enum class MotorType : int8_t {
    None = 0,
    StepStick,
    External,
    TMC2130,
    TMC5160,
    TMC2208,
    TMC2209,
    Unipolar,
    RCServo,
    Dynamixel,
    Solenoid,
};

enum class TrinamicMode : int8_t {
    None        = 0,  // not for machine defs!
    StealthChop = 1,
    CoolStep    = 2,
    StallGuard  = 3,
};

#include "../Grbl.h"

// These are used for setup and to talk to the motors as a group.
void    init_motors();
uint8_t get_next_trinamic_driver_index();
void    readSgTask(void* pvParameters);
void    motors_read_settings();

// The return value is a bitmask of axes that can home
uint8_t motors_set_homing_mode(uint8_t homing_mask, bool isHoming);
void    motors_set_disable(bool disable, uint8_t mask = B11111111);  // default is all axes
void    motors_step(uint8_t step_mask, uint8_t dir_mask);
void    motors_unstep();

void servoUpdateTask(void* pvParameters);

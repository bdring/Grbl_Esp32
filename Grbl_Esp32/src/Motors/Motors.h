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

#include "../Grbl.h"
#include <TMCStepper.h>  // https://github.com/teemuatlut/TMCStepper

extern rmt_item32_t  rmtItem[2];
extern rmt_config_t  rmtConfig;
extern rmt_channel_t sys_get_next_RMT_chan_num();

typedef enum {
    MOTOR,
    NULL_MOTOR,
    STANDARD_MOTOR,
    TRINAMIC_SPI_MOTOR,
    UNIPOLAR_MOTOR,
    RC_SERVO_MOTOR,
    SOLENOID,
    DYNAMIXEL2
} motor_class_id_t;

// These are used for setup and to talk to the motors as a group.
void    init_motors();
uint8_t get_next_trinamic_driver_index();
bool    motors_have_type_id(motor_class_id_t id);
void    readSgTask(void* pvParameters);
void    motors_read_settings();

// The return value is a bitmask of axes that can home
uint8_t motors_set_homing_mode(uint8_t homing_mask, bool isHoming);
void    motors_set_disable(bool disable);
void    motors_step(uint8_t step_mask, uint8_t dir_mask);
void    motors_unstep();

void servoUpdateTask(void* pvParameters);

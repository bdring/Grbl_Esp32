/*
	midtbot_uni.h
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

	The midTbot_Uni is a unipolar stepper version of the midTbot
*/

#define CPU_MAP_NAME "MIDTBOT_UNI"
	
#define USE_UNIPOLAR
#ifndef COREXY // maybe set in config.h
	//#define COREXY
#endif

#define X_UNIPOLAR
#define X_PIN_PHASE_0	GPIO_NUM_13
#define X_PIN_PHASE_1	GPIO_NUM_21
#define X_PIN_PHASE_2	GPIO_NUM_16
#define X_PIN_PHASE_3	GPIO_NUM_22

#define Y_UNIPOLAR
#define Y_PIN_PHASE_0	GPIO_NUM_25
#define Y_PIN_PHASE_1	GPIO_NUM_27
#define Y_PIN_PHASE_2	GPIO_NUM_26
#define Y_PIN_PHASE_3	GPIO_NUM_32

#define LIMIT_MASK 0  // no limit pins	

#define DEFAULT_X_STEPS_PER_MM 127.368
#define DEFAULT_Y_STEPS_PER_MM 127.368

#define DEFAULT_X_MAX_RATE 750.0 // mm/min
#define DEFAULT_Y_MAX_RATE 750.0 // mm/min

#define DEFAULT_X_ACCELERATION (50.0*60*60)
#define DEFAULT_Y_ACCELERATION (50.0*60*60)
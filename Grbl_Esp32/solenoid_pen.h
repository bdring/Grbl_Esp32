/*
  solenoid_pen.h
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

	Usage notes:
	This is designed to use a solenoid to lift a pen.
	When the current Z location is below zero the pen is down
	If the Z goes to zero or above the pen goes up.
	There are two power levels, the initial pull up strength, then the hold strength
	

	Note: There is a still a virtual Z axis that has a finite speed.
	If your gcode is commanding long travels in Z, there will be delays
	between solenoid states as the Z "travels" to the location that will
	change the state.

*/

#define SOLENOID_PWM_FREQ 5000
#define SOLENOID_PWM_RES_BITS 8

#define SOLENOID_TURNON_DELAY (SOLENOID_TIMER_INT_FREQ/2)
#define SOLENOID_PULSE_LEN_UP 255
#define SOLENOID_HOLD_DELAY (SOLENOID_TIMER_INT_FREQ/2) // in task counts...after this delay power will change to hold level
#define SOLENOID_PULSE_LEN_HOLD 80 // solenoid hold level ... typically a lower value to prevent overheating

#define SOLENOID_TIMER_INT_FREQ 50

#ifndef solenoid_h
#define solenoid_h

void solenoid_init();
void solenoid_disable();
void solenoidSyncTask(void *pvParameters);
void calc_solenoid(float penZ);

#endif
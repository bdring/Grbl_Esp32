/*
  servo.h
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

	To use this, uncomment #define USE_PEN_SERVO in config.h

	That should be the only change you need at the top level
	Everything occurs as a low priority task that syncs the servo with the
	current machine position.

*/

// ==== Begin: Things you are likely to change ====================
//#define SERVO_PEN_PIN 					GPIO_NUM_27   // FYI...you can disable the Z stepper pins (step & dir)

// the pulse lengths for the min and max travel .. (Note: Servo brands vary)
// If the servo goes backward from what you want, flip the values
// Note: this is not necessarily the servos limits (just the travel you want)
#define SERVO_MIN_PULSE_SEC 0.001 // min pulse in seconds
#define SERVO_MAX_PULSE_SEC 0.002 // max pulse in seconds

// Pulse repeat rate (PWM Frequency)
#define SERVO_PULSE_FREQ 50 // 50Hz ...This is a standard analog servo value. Digital ones can repeat faster

// the range of the servo is constrained
// values above or below these will be limited to the min or max
#define SERVO_PEN_RANGE_MIN_MM 0.0 // the minimum z position in mm
#define SERVO_PEN_RANGE_MAX_MM 5.0 // the minimum z position in mm
// ==== End: Things you are likely to change =======================

// Begin: Advanced settings

#define SERVO_TIMER_NUM 1
#define SERVO_TIMER_INT_FREQ 20 // Hz This is the task frequency
#define SERVO_PEN_CHANNEL_NUM 	5

#define SERVO_PULSE_RES_BITS 16 // bits of resolution of PWM (16 is max)
#define SERVO_PULSE_RES_COUNT 65535 // see above  TODO...do the math here 2^SERVO_PULSE_RES_BITS

// A way to reduce the turn on current
#define SERVO_TURNON_DELAY SERVO_TIMER_INT_FREQ*3  // Wait this many task counts to turn on servo

#define SERVO_TIME_PER_BIT  ((1.0 / (float)SERVO_PULSE_FREQ) / ((float)SERVO_PULSE_RES_COUNT) ) // seconds

#define SERVO_MIN_PULSE    (uint16_t)(SERVO_MIN_PULSE_SEC / SERVO_TIME_PER_BIT) // in timer counts
#define SERVO_MAX_PULSE    (uint16_t)(SERVO_MAX_PULSE_SEC / SERVO_TIME_PER_BIT) // in timer counts

#define SERVO_CAL_MIN 20.0 // Percent: the minimum allowable calibration value
#define SERVO_CAL_MAX 180.0 // Percent: the maximum allowable calibration value

#ifndef servo_h
#define servo_h

void servo_init();
void servo_disable();
bool validate_servo_settings(bool verbose);
void servoSyncTask(void *pvParameters);
void calc_pen_servo(float penZ);

#endif

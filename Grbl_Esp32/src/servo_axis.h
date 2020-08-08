#pragma once

/*
  solenoid_pen.h
  Part of Grbl_ESP32

	copyright (c) 2019 -	Bart Dring. This file was intended for use on the ESP32
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

	Servo Axis Class

	The Servo axis feature allows you to use a hobby servo on any axis.
	This is done using a repeating RTOS task. Grbl continues to calculate
	the position of the axis in real time. The task looks at the current position of
	the axis and calculates the required PWM value to go to that location. You define the travel
	of the servo in millimeters.

	Grbl still uses the acceleration and speed values you have in the settings, so it
	will coordinate servo axes with stepper motor axes. This assumes these values are within the
	capabilities of the servo

	Usage

	1. In config.h un-comment #define USE_SERVO_AXES

	2. In the machine definition file in Machines/, define servo pins and PWM channels like this ....
				#define SERVO_Y_PIN 					GPIO_NUM_14

				undefine any step and direction pins associated with that axis

	3. In servo_axis.cpp init_servos() function, configure servos like this ....
				X_Servo_Axis.set_range(0.0, 20.0);  // millimeter
				X_Servo_Axis.set_homing_type(SERVO_HOMING_OFF);
				X_Servo_Axis.set_disable_on_alarm(true);


	The positions can be calibrated using the settings. $10x (resolution) settings adjust the minimum
	position and $13x (max travel) settings adjust the maximum position. If the servo is traveling
	backwards from what you want, you can use the $3 direction setting to compensate.

*/

#include "Motors/RcServoSettings.h"

#define SERVO_HOMING_OFF 0     // servo is off during homing
#define SERVO_HOMING_TARGET 1  // servo is send to a location during homing

extern float my_location;

void init_servos();
void servosSyncTask(void* pvParameters);

class ServoAxis {
public:
    ServoAxis(uint8_t axis, uint8_t pin_num);  // constructor
    void init();
    void set_location();
    void disable();  // sets PWM to 0% duty cycle. Most servos can be manually moved in this state
    void set_range(float min, float max);
    void set_homing_type(uint8_t homing_type);
    void set_homing_position(float homing_position);
    void set_disable_on_alarm(bool disable_on_alarm);
    void set_disable_with_steppers(bool disable_with_steppers);
    void set_use_mpos(bool use_mpos);

private:
    int  _axis;         // these should be assign in constructor using Grbl X_AXIS type values
    int  _pin_num;      // The GPIO pin being used
    int  _channel_num;  // The PWM channel
    bool _showError;

    uint32_t _pwm_freq            = SERVO_PULSE_FREQ;
    uint32_t _pwm_resolution_bits = SERVO_PULSE_RES_BITS;
    float    _pulse_min           = SERVO_MIN_PULSE;             // in pwm counts
    float    _pulse_max           = SERVO_MAX_PULSE;             // in pwm counts
    float    _position_min        = SERVO_POSITION_MIN_DEFAULT;  // position in millimeters
    float    _position_max        = SERVO_POSITION_MAX_DEFAULT;  // position in millimeters

    uint8_t _homing_type           = SERVO_HOMING_OFF;
    float   _homing_position       = SERVO_POSITION_MAX_DEFAULT;
    bool    _disable_on_alarm      = true;
    bool    _disable_with_steppers = false;
    bool    _use_mpos              = true;

    bool _validate_cal_settings();
    void _write_pwm(uint32_t duty);
    bool _cal_is_valid();  // checks to see if calibration values are in acceptable range
};

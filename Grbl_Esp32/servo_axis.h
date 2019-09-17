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

	2. In a cpu_map.h section, define servo pins and PWM channels like this .... 
				#define SERVO_Y_PIN 					GPIO_NUM_14
				#define SERVO_Y_CHANNEL_NUM 	6
				
				undefine any step and direction pins associated with that axis
				
	3. In servo_axis.cpp init_servos() function, configure servos like this ....
				X_Servo_Axis.set_range(0.0, 20.0);  // millimeter
				X_Servo_Axis.set_homing_type(SERVO_HOMING_OFF);
				X_Servo_Axis.set_disable_on_alarm(true);
				

	The positions can be calibrated using the settings. $10x (resolution) settings adjust the minimum
	position and $13x (max travel) settings adjust the maximum position. If the servo is traveling 
	backwards from what you want, you can use the $3 direction setting to compensate.	

*/

#ifndef servo_axis_h
  #define servo_axis_h



// this is the pulse range of a the servo. Typical servos are 0.001 to 0.002 seconds
// some servos have a wider range. You can adjust this here or in the calibration feature
#define SERVO_MIN_PULSE_SEC 0.001 // min pulse in seconds
#define SERVO_MAX_PULSE_SEC 0.002 // max pulse in seconds

#define SERVO_POSITION_MIN_DEFAULT   0.0 // mm
#define SERVO_POSITION_MAX_DEFAULT   20.0 // mm

#define SERVO_PULSE_FREQ 50 // 50Hz ...This is a standard analog servo value. Digital ones can repeat faster

#define SERVO_PULSE_RES_BITS 16 // bits of resolution of PWM (16 is max)
#define SERVO_PULSE_RES_COUNT 65535 // see above  TODO...do the math here 2^SERVO_PULSE_RES_BITS

#define SERVO_TIME_PER_BIT  ((1.0 / (float)SERVO_PULSE_FREQ) / ((float)SERVO_PULSE_RES_COUNT) ) // seconds

#define SERVO_MIN_PULSE    (uint16_t)(SERVO_MIN_PULSE_SEC / SERVO_TIME_PER_BIT) // in timer counts
#define SERVO_MAX_PULSE    (uint16_t)(SERVO_MAX_PULSE_SEC / SERVO_TIME_PER_BIT) // in timer counts

#define SERVO_PULSE_RANGE (SERVO_MAX_PULSE-SERVO_MIN_PULSE)

#define SERVO_CAL_MIN 20.0 // Percent: the minimum allowable calibration value
#define SERVO_CAL_MAX 180.0 // Percent: the maximum allowable calibration value

#define SERVO_TIMER_INT_FREQ 20 // Hz This is the task frequency

#define SERVO_HOMING_OFF 0 // servo is off during homing
#define SERVO_HOMING_TARGET 1 // servo is send to a location during homing

extern float my_location;

void init_servos();
void servosSyncTask(void *pvParameters);


class ServoAxis{
	public:
		ServoAxis(uint8_t axis, uint8_t pin_num, uint8_t channel_num); // constructor   
		void init();
		void set_location();
		void disable(); // sets PWM to 0% duty cycle. Most servos can be manually moved in this state
		void set_range(float min, float max);
		void set_homing_type(uint8_t homing_type);
		void set_homing_position(float homing_position);
		void set_disable_on_alarm (bool disable_on_alarm);
		void set_disable_with_steppers(bool disable_with_steppers);
		void set_use_mpos(bool use_mpos);
    
	private:
		int _axis; // these should be assign in constructor using Grbl X_AXIS type values
		int _pin_num; // The GPIO pin being used
		int _channel_num; // The PWM channel
		bool _showError;
		
		uint32_t _pwm_freq = SERVO_PULSE_FREQ;
		uint32_t _pwm_resolution_bits = SERVO_PULSE_RES_BITS;
		float _pulse_min = SERVO_MIN_PULSE; // in pwm counts
		float _pulse_max = SERVO_MAX_PULSE; // in pwm counts
		float _position_min = SERVO_POSITION_MIN_DEFAULT; // position in millimeters
		float _position_max = SERVO_POSITION_MAX_DEFAULT; // position in millimeters
		
		
		uint8_t _homing_type = SERVO_HOMING_OFF;
		float _homing_position = SERVO_POSITION_MAX_DEFAULT;
		bool _disable_on_alarm = true;
		bool _disable_with_steppers = false;
		bool _use_mpos = true;
				
		bool _validate_cal_settings();
		void _write_pwm(uint32_t duty);
		bool _cal_is_valid(); // checks to see if calibration values are in acceptable range 
		
};

#endif

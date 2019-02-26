/*
  servo_axis.cpp
  Part of Grbl_ESP32

	copyright (c) 2018 -	Bart Dring. This file was intended for use on the ESP32
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
	
	See servo_axis.h for more details

*/

#include "grbl.h"

#ifdef USE_SERVO_AXES

#ifdef SERVO_X_PIN
	ServoAxis X_Servo_Axis(X_AXIS, SERVO_X_PIN, SERVO_X_CHANNEL_NUM);
#endif
#ifdef SERVO_Y_PIN
	ServoAxis Y_Servo_Axis(Y_AXIS, SERVO_Y_PIN, SERVO_Y_CHANNEL_NUM);
#endif
#ifdef SERVO_Z_PIN
	ServoAxis Z_Servo_Axis(Z_AXIS, SERVO_Z_PIN, SERVO_Z_CHANNEL_NUM);
#endif

void init_servos()
{
	#ifdef SERVO_X_PIN
		X_Servo_Axis.init();
		Y_Servo_Axis.set_range(SERVO_X_RANGE_MIN, SERVO_X_RANGE_MAX);
		X_Servo_Axis.set_homing_type(SERVO_HOMING_OFF);
		X_Servo_Axis.set_disable_on_alarm(true);
		X_Servo_Axis.set_disable_with_steppers(false);
	#endif
	#ifdef SERVO_Y_PIN
		Y_Servo_Axis.init();
		Y_Servo_Axis.set_range(SERVO_Y_RANGE_MIN, SERVO_Y_RANGE_MAX);
	#endif
	#ifdef SERVO_Z_PIN
		Z_Servo_Axis.init();		
		Z_Servo_Axis.set_range(SERVO_Z_RANGE_MIN, SERVO_Z_RANGE_MAX);
		Z_Servo_Axis.set_homing_type(SERVO_HOMING_TARGET);
		Z_Servo_Axis.set_homing_position(SERVO_Z_RANGE_MAX);
	#endif
  
  // setup a task that will calculate the determine and set the servo positions
  xTaskCreatePinnedToCore(  servosSyncTask,    // task
                              "servosSyncTask", // name for task
                              4096,   // size of task stack
                              NULL,   // parameters
                              1, // priority
                              &servosSyncTaskHandle,
                              0 // core
                         );
}


// this is the task
void servosSyncTask(void *pvParameters)
{ 
  TickType_t xLastWakeTime;
  const TickType_t xServoFrequency = SERVO_TIMER_INT_FREQ;  // in ticks (typically ms)
  uint16_t servo_delay_counter = 0;
  
  while(true) { // don't ever return from this or the task dies

    vTaskDelayUntil(&xLastWakeTime, xServoFrequency);
		#ifdef SERVO_X_PIN
			X_Servo_Axis.set_location();
			
		#endif
		#ifdef SERVO_Y_PIN
			Y_Servo_Axis.set_location();
		#endif
		#ifdef SERVO_Z_PIN
			Z_Servo_Axis.set_location();
		#endif
  }
} 

// =============================== Class Stuff ================================= //

ServoAxis::ServoAxis(uint8_t axis, uint8_t pin_num, uint8_t channel_num) // constructor
{
  _axis = axis;
	_pin_num = pin_num;
	_channel_num = channel_num;	
}

void ServoAxis::init()
{
	ledcSetup(_channel_num, _pwm_freq, _pwm_resolution_bits);
	ledcAttachPin(_pin_num, _channel_num);
  disable();
}
/*
void ServoAxis::set_location()
{
  // These are the pulse lengths for the minimum and maximum positions
  // Note: Some machines will have the physical max/min inverted with pulse length max/min due to invert setting $3=...
  float servo_pulse_min, servo_pulse_max;
  float min_pulse_cal, max_pulse_cal; // calibration values in percent 110% = 1.1
  uint32_t servo_pulse_len;
	float mpos, offset, wpos;
	
	// skip location if we are in alarm mode
	if (_disable_on_alarm && (sys.state == STATE_ALARM)) {
		disable();
		return;
	}
	
	// track the disable status of the steppers if desired.
	if (_disable_with_steppers && get_stepper_disable()) {
		disable();
		return;
	}
	
	
	if ( (_homing_type == SERVO_HOMING_TARGET) && (sys.state == STATE_HOMING) ) {
		wpos = _homing_position; // go to servos home position
	}
	else {
		mpos = system_convert_axis_steps_to_mpos(sys_position, _axis);  // get the axis machine position in mm
		offset = gc_state.coord_system[_axis]+gc_state.coord_offset[_axis]; // get the current axis work offset
		wpos = mpos - offset; // determine the current work position	
	}
  
	// get the calibration values
	if (_cal_is_valid(false)) { // if calibration settings are OK then apply them
		min_pulse_cal = (settings.steps_per_mm[_axis] / 100.0);
		max_pulse_cal = (settings.max_travel[_axis] / -100.0);
		if (bit_istrue(settings.dir_invert_mask,bit(_axis))) { // the offset needs to be backwards
			min_pulse_cal = 1.0 + (1.0 - min_pulse_cal);
			max_pulse_cal = 1.0 + (1.0 - max_pulse_cal);
		}			
	}
	else { // settings are not valid so don't apply any calibration
		min_pulse_cal = 1.0;
		max_pulse_cal = 1.0;	
	}
	
	
  if (bit_istrue(settings.dir_invert_mask,bit(_axis))) { // this allows the user to change the direction via settings		
		servo_pulse_min = SERVO_MAX_PULSE;
		servo_pulse_max = SERVO_MIN_PULSE;
	}
	else {
		servo_pulse_min = SERVO_MIN_PULSE;
		servo_pulse_max = SERVO_MAX_PULSE;
	}
	
	// apply the calibrations
	servo_pulse_min *= min_pulse_cal;
	servo_pulse_max *= max_pulse_cal;

  // determine the pulse length
  servo_pulse_len = (uint32_t)mapConstrain(wpos, 
        _position_min, _position_max, 
        servo_pulse_min, servo_pulse_max );
        
  _write_pwm(servo_pulse_len);
}
*/

void ServoAxis::set_location()
{
  // These are the pulse lengths for the minimum and maximum positions
  // Note: Some machines will have the physical max/min inverted with pulse length max/min due to invert setting $3=...
  float servo_pulse_min, servo_pulse_max;
  float min_pulse_cal, max_pulse_cal; // calibration values in percent 110% = 1.1
  uint32_t servo_pulse_len;
	float servo_pos, mpos, offset, wpos;
	
	// skip location if we are in alarm mode
	if (_disable_on_alarm && (sys.state == STATE_ALARM)) {
		disable();
		return;
	}
	
	// track the disable status of the steppers if desired.
	if (_disable_with_steppers && get_stepper_disable()) {
		disable();
		return;
	}
	
	
	if ( (_homing_type == SERVO_HOMING_TARGET) && (sys.state == STATE_HOMING) ) {
		servo_pos = _homing_position; // go to servos home position
	}
	else {
		mpos = system_convert_axis_steps_to_mpos(sys_position, _axis);  // get the axis machine position in mm
		if (_use_mpos) {
		servo_pos = mpos;
		
		}
		else {
			offset = gc_state.coord_system[_axis]+gc_state.coord_offset[_axis]; // get the current axis work offset
			servo_pos = mpos - offset; // determine the current work position	
		}
	}
  
	// get the calibration values
	if (_cal_is_valid(false)) { // if calibration settings are OK then apply them
		min_pulse_cal = (settings.steps_per_mm[_axis] / 100.0);
		max_pulse_cal = (settings.max_travel[_axis] / -100.0);
		if (bit_istrue(settings.dir_invert_mask,bit(_axis))) { // the offset needs to be backwards
			min_pulse_cal = 1.0 + (1.0 - min_pulse_cal);
			max_pulse_cal = 1.0 + (1.0 - max_pulse_cal);
		}			
	}
	else { // settings are not valid so don't apply any calibration
		min_pulse_cal = 1.0;
		max_pulse_cal = 1.0;	
	}
	
	
  if (bit_istrue(settings.dir_invert_mask,bit(_axis))) { // this allows the user to change the direction via settings		
		servo_pulse_min = SERVO_MAX_PULSE;
		servo_pulse_max = SERVO_MIN_PULSE;
	}
	else {
		servo_pulse_min = SERVO_MIN_PULSE;
		servo_pulse_max = SERVO_MAX_PULSE;
	}
	
	// apply the calibrations
	servo_pulse_min *= min_pulse_cal;
	servo_pulse_max *= max_pulse_cal;
	
  // determine the pulse length
  servo_pulse_len = (uint32_t)mapConstrain(servo_pos, 
        _position_min, _position_max, 
        servo_pulse_min, servo_pulse_max );
        
  _write_pwm(servo_pulse_len);
}

void ServoAxis::_write_pwm(uint32_t duty)
{
  if (ledcRead(_channel_num) != duty) { // only write if it is changing    
	  ledcWrite(_channel_num, duty);
  }
}

// sets the PWM to zero. This allows most servos to be manually moved
void ServoAxis::disable()
{
  _write_pwm(0);
}

// checks to see if calibration values are in an acceptable range
// vebose = true if you want an error sent to serial port
bool ServoAxis::_cal_is_valid(bool verbose)
{
  bool settingsOK = true;

	if ( (settings.steps_per_mm[_axis] < SERVO_CAL_MIN) || (settings.steps_per_mm[_axis] > SERVO_CAL_MAX) ) {
		if (verbose) {
			grbl_sendf(CLIENT_SERIAL, "[MSG:Servo cal ($10%d) Error: %4.4f s/b between %.2f and %.2f]\r\n", _axis, settings.steps_per_mm[_axis], SERVO_CAL_MIN, SERVO_CAL_MAX);
		}
		settingsOK = false;
	}

	// Note: Max travel is set positive via $$, but stored as a negative number
	if ( (settings.max_travel[_axis] < -SERVO_CAL_MAX) || (settings.max_travel[_axis] > -SERVO_CAL_MIN) ) {
		if (verbose) {
			grbl_sendf(CLIENT_SERIAL, "[MSG:Servo cal ($13%d) Error: %4.4f s/b between %.2f and %.2f]\r\n", _axis, -settings.max_travel[_axis], SERVO_CAL_MIN, SERVO_CAL_MAX);
		}
		settingsOK = false;
	}
	return settingsOK;
}

/*
		Use this to set the max and min position in mm of the servo
		This is used when mapping pulse length to the position
*/
void ServoAxis::set_range(float min, float max) {
  if (min < max) {
    _position_min = min;
    _position_max = max;
  }
  else {
    grbl_send(CLIENT_SERIAL, "[MSG:Error setting range. Min not smaller than max]\r\n");
  }
}

/*
		Sets the mode the servo will be in during homing
		See servo_axis.h for SERVO_HOMING_xxxxx types
*/
void ServoAxis::set_homing_type(uint8_t homing_type) 
{
	if (homing_type <= SERVO_HOMING_TARGET)
		_homing_type = homing_type;
}

/*
		Use this to set the homing position the servo will be commanded to go if
		the current homing mode is SERVO_HOMING_TARGET
*/
void ServoAxis::set_homing_position(float homing_position)
{
	_homing_position = homing_position;
}

/*
		Use this to set the disable on alarm feature. If true, then hobby servo PWM
		will be disable in Grbl alarm mode (like before homing). Typical hobby servo 
		can be moved by hand in this mode
*/
void ServoAxis::set_disable_on_alarm (bool disable_on_alarm)
{
	_disable_on_alarm = disable_on_alarm;
}

void ServoAxis::set_disable_with_steppers(bool disable_with_steppers) {
	_disable_with_steppers = disable_with_steppers;
}


#endif

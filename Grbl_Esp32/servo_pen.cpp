/*
  servo_pen.cpp
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

*/
#include "grbl.h"

#ifdef USE_PEN_SERVO

static TaskHandle_t servoSyncTaskHandle = 0;

// used to delay turn on
bool servo_pen_enable = false;

void servo_init()
{
	grbl_send(CLIENT_SERIAL, "[MSG:Servo Pen Mode]\r\n");  // startup message
	//validate_servo_settings(true); // display any calibration errors

	// Debug stuff
	//grbl_sendf(CLIENT_SERIAL, "[MSG:Servo max,min pulse times %.4f sec,%.4f sec]\r\n", SERVO_MAX_PULSE_SEC, SERVO_MIN_PULSE_SEC);
	//grbl_sendf(CLIENT_SERIAL, "[MSG:Servo max,min pulse counts %d,%d]\r\n", SERVO_MAX_PULSE, SERVO_MIN_PULSE);
	validate_servo_settings(true); // will print errors
	// debug stuff

	servo_pen_enable = false;  // start delay has not completed yet.

	// setup PWM channel
	ledcSetup(SERVO_PEN_CHANNEL_NUM, SERVO_PULSE_FREQ, SERVO_PULSE_RES_BITS);
	ledcAttachPin(SERVO_PEN_PIN, SERVO_PEN_CHANNEL_NUM);

	servo_disable(); // start it it off

	// setup a task that will calculate the determine and set the servo position
	xTaskCreatePinnedToCore(	servoSyncTask,    // task
	                            "servoSyncTask", // name for task
	                            4096,   // size of task stack
	                            NULL,   // parameters
	                            1, // priority
	                            &servoSyncTaskHandle,
	                            0 // core
	                       );
}

// turn off the PWM (0 duty) to prevent servo jitter when not in use.
void servo_disable()
{
	ledcWrite(SERVO_PEN_CHANNEL_NUM, 0);
}

// Grbl settings are used to calibrate the servo positions
// They work on a percentage, so a value of 100 (100%) applies no calibration
// Values outside a reasonable range can cause errors, so this function checks
// that they are within a reasonable range
bool validate_servo_settings(bool verbose) // make sure the settings are reasonable..otherwise reset the settings to default
{
	bool settingsOK = true;

	if ( (settings.steps_per_mm[Z_AXIS] < SERVO_CAL_MIN) || (settings.steps_per_mm[Z_AXIS] > SERVO_CAL_MAX) ) {
		if (verbose) {
			grbl_sendf(CLIENT_SERIAL, "[MSG:Servo cal ($102) Error: %4.4f s/b between %.2f and %.2f]\r\n", settings.steps_per_mm[Z_AXIS], SERVO_CAL_MIN, SERVO_CAL_MAX);
		}

		settingsOK = false;
	}

	// Note: Max travel is set positive via $$, but stored as a negative number
	if ( (settings.max_travel[Z_AXIS] < -SERVO_CAL_MAX) || (settings.max_travel[Z_AXIS] > -SERVO_CAL_MIN) ) {
		if (verbose) {
			grbl_sendf(CLIENT_SERIAL, "[MSG:Servo cal ($132) Error: %4.4f s/b between %.2f and %.2f]\r\n", -settings.max_travel[Z_AXIS], SERVO_CAL_MIN, SERVO_CAL_MAX);
		}

		settingsOK = false;
	}

	return settingsOK;
}

// this is the task
void servoSyncTask(void *pvParameters)
{
	//int32_t current_position[N_AXIS]; // copy of current location
	//float m_pos[N_AXIS];   // machine position in mm
	TickType_t xLastWakeTime;
	const TickType_t xServoFrequency = SERVO_TIMER_INT_FREQ;  // in ticks (typically ms)
	uint16_t servo_delay_counter = 0;
	
	float mpos_z, wpos_z;
	float z_offset;

	xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
	while(true) { // don't ever return from this or the task dies
		if (sys.state != STATE_ALARM) {	// don't move until alarm is cleared...typically homing
			if (!servo_pen_enable ) {
				servo_delay_counter++;
				servo_pen_enable = (servo_delay_counter > SERVO_TURNON_DELAY);
			} else {			
					mpos_z = system_convert_axis_steps_to_mpos(sys_position, Z_AXIS);  // get the machine Z in mm
					z_offset = gc_state.coord_system[Z_AXIS]+gc_state.coord_offset[Z_AXIS]; // get the current z work offset
					wpos_z = mpos_z - z_offset; // determine the current work Z			

					calc_pen_servo(wpos_z); // calculate kinematics and move the servos
			}
		}
		vTaskDelayUntil(&xLastWakeTime, xServoFrequency);
	}
}

// calculate and set the PWM value for the servo
void calc_pen_servo(float penZ)
{
	uint32_t servo_pen_pulse_len;
	float servo_pen_pulse_min, servo_pen_pulse_max;

	if (!servo_pen_enable) { // only proceed if startup delay as expired
		return;
	}

	if (validate_servo_settings(false)) { // if calibration settings are OK then apply them
		if (bit_istrue(settings.dir_invert_mask,bit(Z_AXIS))) { // this allows the user to change the direction via settings
			// Apply a calibration to the minimum position
			servo_pen_pulse_max = SERVO_MIN_PULSE * (settings.steps_per_mm[Z_AXIS] / 100.0);
			// Apply a calibration to the maximum position
			servo_pen_pulse_min = SERVO_MAX_PULSE * (settings.max_travel[Z_AXIS] / -100.0);
		}
		else {
			// Apply a calibration to the minimum position
			servo_pen_pulse_min = SERVO_MIN_PULSE * (settings.steps_per_mm[Z_AXIS] / 100.0);
			// Apply a calibration to the maximum position
			servo_pen_pulse_max = SERVO_MAX_PULSE * (settings.max_travel[Z_AXIS] / -100.0);
		}
		
	} else { // use the defaults
		if (bit_istrue(settings.dir_invert_mask,bit(Z_AXIS))) { // this allows the user to change the direction via settings
			servo_pen_pulse_min = SERVO_MAX_PULSE;
			servo_pen_pulse_max = SERVO_MIN_PULSE;
		}
		else {
			servo_pen_pulse_min = SERVO_MIN_PULSE;
			servo_pen_pulse_max = SERVO_MAX_PULSE;
		}
		
	}

	// determine the pulse length
	servo_pen_pulse_len = (uint32_t)mapConstrain(penZ, SERVO_PEN_RANGE_MIN_MM, SERVO_PEN_RANGE_MAX_MM, servo_pen_pulse_min, servo_pen_pulse_max );

	// skip setting value if it is unchanged
	if (ledcRead(SERVO_PEN_CHANNEL_NUM) == servo_pen_pulse_len)
		return;
  	
	// update the PWM value
	// ledcWrite appears to have issues with interrupts, so make this a critical section
	portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&myMutex);
	ledcWrite(SERVO_PEN_CHANNEL_NUM, servo_pen_pulse_len);
	portEXIT_CRITICAL(&myMutex);
}

#endif

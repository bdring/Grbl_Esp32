/*
  solenoid_pen.cpp
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

#ifdef USE_PEN_SOLENOID

static TaskHandle_t solenoidSyncTaskHandle = 0;

// used to delay turn on
bool solenoid_pen_enable; 
uint16_t solenoide_hold_count;

void solenoid_init()
{		
	grbl_send(CLIENT_SERIAL, "[MSG:Solenoid Mode]\r\n");  // startup message	
	//validate_servo_settings(true); // display any calibration errors 
		
	solenoid_pen_enable = false;  // start delay has not completed yet.	
	solenoide_hold_count = 0; // initialize
	
	// setup PWM channel
	ledcSetup(SOLENOID_CHANNEL_NUM, SOLENOID_PWM_FREQ, SOLENOID_PWM_RES_BITS);
	ledcAttachPin(SOLENOID_PEN_PIN, SOLENOID_CHANNEL_NUM);
	
  solenoid_disable(); // start it it off
	
	// setup a task that will calculate the determine and set the servo position		
	xTaskCreatePinnedToCore(	solenoidSyncTask,    // task
   													"solenoidSyncTask", // name for task
													4096,   // size of task stack
													NULL,   // parameters
													1, // priority
													&solenoidSyncTaskHandle,
													0 // core
													);													
}

// turn off the PWM (0 duty)
void solenoid_disable()
{
	ledcWrite(SOLENOID_CHANNEL_NUM, 0);
}

// this is the task
void solenoidSyncTask(void *pvParameters)
{		
	int32_t current_position[N_AXIS]; // copy of current location
	float m_pos[N_AXIS];   // machine position in mm
	TickType_t xLastWakeTime;
	const TickType_t xSolenoidFrequency = SOLENOID_TIMER_INT_FREQ;  // in ticks (typically ms)
	uint16_t solenoid_delay_counter = 0;

	xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
	while(true) { // don't ever return from this or the task dies
			if (!solenoid_pen_enable) {
				solenoid_delay_counter++;				
				solenoid_pen_enable = (solenoid_delay_counter > SOLENOID_TURNON_DELAY);
			}
			else {						
					memcpy(current_position,sys_position,sizeof(sys_position));  // get current position in step	
					system_convert_array_steps_to_mpos(m_pos,current_position); // convert to millimeters				
					calc_solenoid(m_pos[Z_AXIS]); // calculate kinematics and move the servos
			}			
			vTaskDelayUntil(&xLastWakeTime, xSolenoidFrequency);
    }
}

// calculate and set the PWM value for the servo
void calc_solenoid(float penZ)
{	
	uint32_t solenoid_pen_pulse_len;
	
	if (!solenoid_pen_enable) // only proceed if startup delay as expired 
		return;		
		
	if (penZ < 0 && (sys.state != STATE_ALARM)) {  // alarm also makes it go up
		solenoide_hold_count = 0; // reset this count
		solenoid_pen_pulse_len = 0; //
	}
	else {		
		if (solenoide_hold_count < SOLENOID_PULSE_LEN_HOLD) {
			solenoid_pen_pulse_len = SOLENOID_PULSE_LEN_UP;
			solenoide_hold_count++;
		}
		else {
			solenoid_pen_pulse_len = SOLENOID_PULSE_LEN_HOLD;
		}
		
	}
	
	// skip setting value if it is unchanged
	if (ledcRead(SOLENOID_CHANNEL_NUM) == solenoid_pen_pulse_len)
		return;
	
	// update the PWM value
	// ledcWrite appears to have issues with interrupts, so make this a critical section
	portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
	portENTER_CRITICAL(&myMutex);
		ledcWrite(SOLENOID_CHANNEL_NUM, solenoid_pen_pulse_len);		
	portEXIT_CRITICAL(&myMutex);	
}

#endif


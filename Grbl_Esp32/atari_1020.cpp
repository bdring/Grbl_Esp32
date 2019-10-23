/*
	atari_1020.cpp
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

	--------------------------------------------------------------
  
 	This contains all the special features required to control an 
	Atari 1010 Pen Plotter	
*/
#include "grbl.h"

#ifdef ATARI_1020

#define HOMING_PHASE_FULL_APPROACH	0 // move to right end
#define HOMING_PHASE_CHECK			1 // check reed switch
#define HOMING_PHASE_RETRACT 		2 // retract
#define HOMING_PHASE_SHORT_APPROACH	3 // retract

static TaskHandle_t solenoidSyncTaskHandle = 0;
static TaskHandle_t atariHomingTaskHandle = 0;
uint16_t solenoid_pull_count;
bool atari_homing = false;
uint8_t homing_phase = HOMING_PHASE_FULL_APPROACH;
uint8_t current_tool; 

void machine_init()
{				
	solenoid_pull_count = 0; // initialize
	
	grbl_send(CLIENT_SERIAL, "[MSG:Atari 1020 Solenoid]\r\n");
	
	// setup PWM channel
	ledcSetup(SOLENOID_CHANNEL_NUM, SOLENOID_PWM_FREQ, SOLENOID_PWM_RES_BITS);
	ledcAttachPin(SOLENOID_PEN_PIN, SOLENOID_CHANNEL_NUM);
		
	pinMode(SOLENOID_DIRECTION_PIN, OUTPUT);  // this sets the direction of the solenoid current	
	pinMode(REED_SW_PIN, INPUT_PULLUP); // external pullup required
	
	// setup a task that will calculate solenoid position		
	xTaskCreatePinnedToCore(	solenoidSyncTask,    // task
								"solenoidSyncTask", // name for task
								4096,   // size of task stack
								NULL,   // parameters
								1, // priority
								&solenoidSyncTaskHandle,
								0 // core
							);
	// setup a task that will do the custom homing sequence
	xTaskCreatePinnedToCore(	atari_home_task,    // task
								"atari_home_task", // name for task
								4096,   // size of task stack
								NULL,   // parameters
								1, // priority
								&atariHomingTaskHandle,
								0 // core
							);	
}

// this task tracks the Z position and sets the solenoid
void solenoidSyncTask(void *pvParameters)
{		
	int32_t current_position[N_AXIS]; // copy of current location
	float m_pos[N_AXIS];   // machine position in mm
	TickType_t xLastWakeTime;
	const TickType_t xSolenoidFrequency = SOLENOID_TASK_FREQ;  // in ticks (typically ms)

	xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
	while(true) { // don't ever return from this or the task dies
							
		memcpy(current_position,sys_position,sizeof(sys_position));  // get current position in step	
		system_convert_array_steps_to_mpos(m_pos,current_position); // convert to millimeters				
		calc_solenoid(m_pos[Z_AXIS]); // calculate kinematics and move the servos
						
		vTaskDelayUntil(&xLastWakeTime, xSolenoidFrequency);
    }	
}

// to do...have this return a true or false. This could be used by the normal homing feature to
// continue with regular homing after setup 
// return true if this completes homing

bool user_defined_homing() {
	// create and start a task to do the special homing	
	homing_phase = HOMING_PHASE_FULL_APPROACH;
	atari_homing = true;
	return true; // this does it...skip the rest of mc_homing_cycle(...)
}

/*
	Do a custom homing routine.
	
	A task is used because it needs to wait until until idle after each move.
	
	1) Do a full travel move to the right. OK to stall if the pen started closer
	2) Check for pen 1
	3) If fail Retract
	4) move to right end
	5) Check...
	....repeat up to 12 times to try to find pen one	
	
	TODO can the retract, move back be 1 phase rather than 2?

*/
void atari_home_task(void *pvParameters) {	
	uint8_t homing_attempt = 0; // how many times have we tried to home
	TickType_t xLastWakeTime;
	const TickType_t xHomingTaskFrequency = 100;  // in ticks (typically ms) .... need to make sure there is enough time to get out of idle	
	char gcode_line[20];	
	
	while(true) { // this task will only last as long as it is homing
		
		if (atari_homing) {
			// must be in idle or alarm state
			if (sys.state == STATE_IDLE) {
				switch(homing_phase) { 
					case HOMING_PHASE_FULL_APPROACH: // a full width move to insure it hits left end	
						inputBuffer.push("G90G0Z1\r"); // lift the pen					
						sprintf(gcode_line, "G91G0X%3.2f\r", -ATARI_PAPER_WIDTH + ATARI_HOME_POS - 3.0); // plus a little extra
						inputBuffer.push(gcode_line);
						homing_attempt = 1;
						homing_phase = HOMING_PHASE_CHECK;
					break;
					case HOMING_PHASE_CHECK: // check the limits switch 
						if (digitalRead(REED_SW_PIN) == 0) { // see if reed switch is grounded
							inputBuffer.push("G4P0.1\n"); // dramtic pause
							
							sys_position[X_AXIS] = ATARI_HOME_POS * settings.steps_per_mm[X_AXIS];
							sys_position[Y_AXIS] = 0.0;
							sys_position[Z_AXIS] = 1.0 * settings.steps_per_mm[Y_AXIS];
							
							gc_sync_position();
							plan_sync_position();
							
							sprintf(gcode_line, "G90G0X%3.2f\r", ATARI_PAPER_WIDTH); // alway return to right side to reduce home travel stalls											
							inputBuffer.push(gcode_line);
							
							current_tool = 1; // local copy for reference...until actual M6 change
							gc_state.tool = current_tool;
							atari_homing = false;  // done with homing sequence
						}
						else {
							homing_phase = HOMING_PHASE_RETRACT;
							homing_attempt++;
						}
					break;
					case HOMING_PHASE_RETRACT:						
						sprintf(gcode_line, "G0X%3.2f\r", -ATARI_HOME_POS);
						inputBuffer.push(gcode_line);
						sprintf(gcode_line, "G0X%3.2f\r", ATARI_HOME_POS);
						inputBuffer.push(gcode_line);
						homing_phase = HOMING_PHASE_CHECK;
					break;
					default:
						grbl_sendf(CLIENT_SERIAL, "[MSG:Homing phase error %d]\r\n", homing_phase);
						atari_homing = false;; // kills task
					break;
				}	
			
				if (homing_attempt > ATARI_HOMING_ATTEMPTS) { // try all positions plus 1
					grbl_send(CLIENT_SERIAL, "[MSG: Atari homing failed]\r\n");
					inputBuffer.push("G90\r");
					atari_homing = false;;
				}
			}
		}
		vTaskDelayUntil(&xLastWakeTime, xHomingTaskFrequency);
	}	
}


// calculate and set the PWM value for the servo
void calc_solenoid(float penZ)
{	
	bool isPenUp;
	static bool previousPenState = false;
	uint32_t solenoid_pen_pulse_len;  // duty cycle of solenoid		
		
	isPenUp = ( (penZ > 0) || (sys.state == STATE_ALARM) ); // is pen above Z0 or is there an alarm	
		
    // if the state has not change, we only count down to the pull time
	if (previousPenState == isPenUp) { // if state is unchanged		
		if (solenoid_pull_count > 0) {
			solenoid_pull_count--;
			solenoid_pen_pulse_len = SOLENOID_PULSE_LEN_PULL; // stay at full power while counting down
		}
		else {			
			solenoid_pen_pulse_len = SOLENOID_PULSE_LEN_HOLD; // pull in delay has expired so lower duty cycle			
		}
	}
	else { // pen direction has changed
		solenoid_pen_pulse_len = SOLENOID_PULSE_LEN_PULL; // go to full power
		solenoid_pull_count = SOLENOID_PULL_DURATION; // set the time to count down		
	}
	
	previousPenState = isPenUp; // save the prev state
	
	digitalWrite(SOLENOID_DIRECTION_PIN, isPenUp);
	
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


/*
	A tool (pen) change is done by bumping the carriage against the right edge 3 times per
	position change. Pen 1-4 is valid range.
*/
void user_tool_change(uint8_t new_tool) {
	uint8_t move_count;
	char gcode_line[20];		
	
	protocol_buffer_synchronize(); // wait for all previous moves to complete
	
	if ((new_tool < 1) || (new_tool > MAX_PEN_NUMBER)) {
		grbl_sendf(CLIENT_ALL, "[MSG: Requested Pen#%d is out of 1-4 range]\r\n", new_tool);
		return;
	}
	
	if (new_tool == current_tool)
		return;
	
	if (new_tool > current_tool) {
		move_count = BUMPS_PER_PEN_CHANGE * (new_tool - current_tool);
	}
	else {
		move_count = BUMPS_PER_PEN_CHANGE * ((MAX_PEN_NUMBER - current_tool) + new_tool);
	}
	sprintf(gcode_line, "G0Z%3.2f\r", ATARI_TOOL_CHANGE_Z); // go to tool change height
	inputBuffer.push(gcode_line);
	for (uint8_t i = 0; i < move_count; i++) {	
		sprintf(gcode_line, "G0X%3.2f\r", ATARI_HOME_POS); // 
		inputBuffer.push(gcode_line);		
		inputBuffer.push("G0X0\r");
	}
	
	current_tool = new_tool;
	
	grbl_sendf(CLIENT_ALL, "[MSG: Change to Pen#%d]\r\n", current_tool);
	
}

// move from current tool to next tool....
void atari_next_pen() {	
	if (current_tool < MAX_PEN_NUMBER) {
		gc_state.tool = current_tool + 1;
	}
	else {
		gc_state.tool = 1;
	}	
	user_tool_change(gc_state.tool);
}

// Polar coaster has macro buttons, this handles those button pushes.
void user_defined_macro(uint8_t index)
{
	char gcode_line[20];
	
	switch (index) {
		#ifdef MACRO_BUTTON_0_PIN
		case CONTROL_PIN_INDEX_MACRO_0:
			grbl_send(CLIENT_SERIAL, "[MSG: Pen Switch]\r\n");
			inputBuffer.push("$H\r");			
		break;
		#endif
		
		#ifdef MACRO_BUTTON_1_PIN
		case CONTROL_PIN_INDEX_MACRO_1:
			grbl_send(CLIENT_SERIAL, "[MSG: Color Switch]\r\n");
			atari_next_pen();
			sprintf(gcode_line, "G90G0X%3.2f\r", ATARI_PAPER_WIDTH); // alway return to right side to reduce home travel stalls											
			inputBuffer.push(gcode_line);
		break;
		#endif
		
		#ifdef MACRO_BUTTON_2_PIN
		case CONTROL_PIN_INDEX_MACRO_2:
			// feed out some paper and reset the Y 0
			grbl_send(CLIENT_SERIAL, "[MSG: Paper Switch]\r\n");		
			inputBuffer.push("G0Y-25\r");
			inputBuffer.push("G4P0.1\r"); // sync...forces wait for planner to clear
			sys_position[Y_AXIS] = 0.0; // reset the Y position
			gc_sync_position();
			plan_sync_position();
		break;
		#endif		
		
		default:
			grbl_sendf(CLIENT_SERIAL, "[MSG: Unknown Switch %d]\r\n", index);
		break;
	}
}

void user_m30() {
	char gcode_line[20];
	sprintf(gcode_line, "G90G0X%3.2f\r", ATARI_PAPER_WIDTH); // 
	inputBuffer.push(gcode_line);
}

#endif


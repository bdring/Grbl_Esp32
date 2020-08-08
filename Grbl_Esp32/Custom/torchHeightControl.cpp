/*
	torchHeightControl.cpp
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

 	This contains all the special features required for "Plasma Torch Height Control"
	This Torch Height Control Custom File Was Created By: William Curry
*/
// This file is enabled by defining: CUSTOM_CODE_FILENAME "Custom/torchHeightControl.cpp"
// in Machines/6_pack_thc.h, thus causing this file to be included
// from ../custom_code.cpp
static TaskHandle_t THCSyncTaskHandle = 0;
static TaskHandle_t THCVoltageTaskHandle = 0;
unsigned long THCCounter = 0; //For debugging only
unsigned long lastDebugPrintTimeMillis; //For debugging only, last time debug info was printed
unsigned long arcOnTime; //milliseconds at which plasma arc was turned on
bool thcRunning;
//int numberIters = 50;
int thcIterationMs = 4;
int directionDownPinState = 1;
int directionUpPinState = 0;
float thcPinVoltage;
float torchVoltage;
float torchVoltageFiltered; ///Current filtered value used for THC routine
float  voltageSetpoint;
int32_t voltageInt;
int currentDirectionState; ////get state of direction pin
bool directionDifference;
void thcStepZDown(){
		if (thc_debug_setting->get() && ((millis() - lastDebugPrintTimeMillis) > thc_debugprint_millis->get()) )
		{
            grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Setpoint = %4.1f THC Voltage = %4.1f Moving Z Down", voltageSetpoint, torchVoltageFiltered);
		    lastDebugPrintTimeMillis = millis();
		}
		//Get Current Direction Pin State
		currentDirectionState = digitalRead(Z_DIRECTION_PIN);
		if(directionDownPinState == currentDirectionState)
		{
			directionDifference = false;
		}
		else
		{
			directionDifference = true;
			//Set Z Direction Pin
			digitalWrite(Z_DIRECTION_PIN,directionDownPinState);  
            delay_ms(STEP_PULSE_DELAY); ///Delay setting the step pin high
		}
		for (int i = 1; i <= (thc_iter_freq -> get()); i++) {
		if(i>1){delay_ms(pulse_microseconds->get());} //if this isn't the first pass then wait to pulse again
        //Pulse Z Step Pin High
        digitalWrite(Z_STEP_PIN,1);
        delay_ms(pulse_microseconds->get()); //Leave Step pin high for 10ms
        //Pulse Z Step Pin Low
        digitalWrite(Z_STEP_PIN,0);  
		//Break for loop if plasma has been turned off
		uint8_t plasmaState = coolant_get_state(); //Using the coolant flood output to turn on the plasma cutter
		if(!plasmaState) {break;}
        //sys_position[Z_AXIS]--;
		}
		if(directionDifference)//if we wrote a different direction state than what the pin was at, then set it back
		{
			digitalWrite(Z_DIRECTION_PIN,!directionDownPinState);
		}
}

void thcStepZUp(){
		if (thc_debug_setting->get() && ((millis() - lastDebugPrintTimeMillis) > thc_debugprint_millis->get()) )
		{
            grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Setpoint = %4.1f THC Voltage = %4.1f Moving Z Up", voltageSetpoint, torchVoltageFiltered);
		    lastDebugPrintTimeMillis = millis();
		}
        //Get Current Direction Pin State
		currentDirectionState = digitalRead(Z_DIRECTION_PIN);
		if(directionUpPinState == currentDirectionState)
		{
			directionDifference = false;
		}
		else
		{
			directionDifference = true;
			//Set Z Direction Pin
			digitalWrite(Z_DIRECTION_PIN,directionUpPinState);
            delay_ms(STEP_PULSE_DELAY); ///Delay setting the step pin high  
		} 
		for (int i = 1; i <= (thc_iter_freq -> get()); i++) {
		if(i>1){delay_ms(pulse_microseconds->get());} //if this isn't the first pass then wait to pulse again
        //Pulse Z Step Pin
        digitalWrite(Z_STEP_PIN,1);
        //Pulse Z Step Pin High
        delay_ms(pulse_microseconds->get()); //Leave Step pin high for 4ms
        //Pulse Z Step Pin Low
        digitalWrite(Z_STEP_PIN,0); 
		//Break for loop if plasma has been turned off
		uint8_t plasmaState = coolant_get_state(); //Using the coolant flood output to turn on the plasma cutter
		if(!plasmaState) {break;}
        //sys_position[Z_AXIS]++;
		}
		if(directionDifference)//if we wrote a different direction state than what the pin was at, then set it back
		{
			digitalWrite(Z_DIRECTION_PIN,!directionUpPinState);
		}

}

void machine_init() {
    grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "Bill's THC Initialized");
    // setup a task that will do torch height control
    xTaskCreatePinnedToCore(THCSyncTask,   // task
                            "THCSyncTask", // name for task
                            4096,				// size of task stack
                            NULL,				// parameters
                            1,					// priority
                            &THCSyncTaskHandle,
                            0 // core
                           );
	// setup a task that will filter the torch voltage
    xTaskCreatePinnedToCore(THCVoltageTask,   // task
                            "THCVoltageTask", // name for task
                            4096,				// size of task stack
                            NULL,				// parameters
                            1,					// priority
                            &THCVoltageTaskHandle,
                            0 // core
                           );
}

// this task is the main THC loop
void THCSyncTask(void* pvParameters) {
    TickType_t xthcWakeTime;
    const TickType_t xTHCFrequency = thcIterationMs;//(thc_iter_freq -> get()); // in ticks (typically ms)
    xthcWakeTime = xTaskGetTickCount(); // Initialise the xthcWakeTime variable with the current time.
    while (true) { // don't ever return from this or the task dies
        //Get the state of the plasma cutter torch on relay
        uint8_t plasmaState = coolant_get_state(); //Using the coolant flood output to turn on the plasma cutter
		if(sys.suspend)
		{
			coolant_set_state(COOLANT_DISABLE);
		}
        if(plasmaState && (voltageSetpoint > 30) && !sys.suspend) ///Plasma Has Been Turned On and the Voltage Setpoint is greater than 30 volts Start The THC Routine
        {
            if((millis()- arcOnTime) > (thc_arc_delay_time->get()))
            {
				thcRunning = true;                
            }
            else
            {
				thcRunning = false;
            }
        }
        else
        {
            arcOnTime = millis();
			thcRunning = false;
            if (thc_debug_setting->get() && ((millis() - lastDebugPrintTimeMillis) > thc_debugprint_millis->get()) )
            {
                //grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Interation # %d", THCCounter);
                grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Voltage Filt = %4.1f", torchVoltageFiltered);
                grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Voltage Unfiltered = %4.1f", torchVoltage);
                grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Pin Voltage = %3.2f", thcPinVoltage);
                grbl_msg_sendf(CLIENT_ALL, MSG_LEVEL_INFO, "THC Voltage Setting = %4.1f", voltageSetpoint);
                lastDebugPrintTimeMillis = millis();
            }
        }
	    
		if((torchVoltageFiltered > voltageSetpoint) && thcRunning) //Voltage is too high and were running THC step Z down
        {
            thcStepZDown();
        }
        else if((torchVoltageFiltered < voltageSetpoint)&& thcRunning) //Voltage is too low and were running THC  step Z up
        {
            thcStepZUp();
        }
		
        THCCounter ++;
		voltageSetpoint = (thc_voltage_setting -> get());
		
        vTaskDelayUntil(&xthcWakeTime, thcIterationMs);//(thc_iter_freq -> get())+ 1);
    }
}

// this task is THC Voltage Filter Loop
void THCVoltageTask(void* pvParameters) {
    TickType_t xLastVoltageWakeTime;
    const TickType_t xTHCVoltageFrequency = 1; // in ticks (typically ms)
    xLastVoltageWakeTime = xTaskGetTickCount(); // Initialise the xLastVoltageWakeTime variable with the current time.
    while (true) {
        // don't ever return from this or the task dies
		voltageInt = analogRead(THC_VOLTAGE_PIN);
		thcPinVoltage = voltageInt * (3.3 / 4095); //0-3.3 volts at torch input pin
		torchVoltage =  (thcPinVoltage*(VOLTAGE_DIVIDER_R1+VOLTAGE_DIVIDER_R2))/VOLTAGE_DIVIDER_R2;//0-207 volts
        if(thcRunning) ///If the Main THC Loop is running Start filtering the voltage
        {
           torchVoltageFiltered = torchVoltageFiltered * (thc_voltage_filter_value -> get()) + torchVoltage * (1-(thc_voltage_filter_value -> get())); //Rough filter for voltage input
        }
        else
        {
            torchVoltageFiltered = torchVoltage;
        }
        vTaskDelayUntil(&xLastVoltageWakeTime, xTHCVoltageFrequency + 1);
    }
}
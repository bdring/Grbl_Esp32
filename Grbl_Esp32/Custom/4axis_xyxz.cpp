/*
	custom_code_template.cpp (4 axis machine)
	Part of Grbl_ESP32

		copyright (c) 2020 Jens Hauser. This file was intended for use on the ESP32
										CPU. Do not use this with Grbl for atMega328P

	
	//TODO describe this
	Tool change function called by G code "T[1..n] M06". Not triggered by G38.2 or ESP3D probe function :-)
	first Z probe before tool change. Only executed once, because then we´ll know the initial Z height.
	// second Z probe after tool change. Now we can compare

	//TODO parameterise this

	//vTaskDelay (0.5 / portTICK_RATE_MS); // 0.5 Sec.
*/

#define DEBUG 0; // do I want debug messages? yes/no

/*
   VARIABLES
*/
uint8_t AmountOfToolChanges; //  Each new tool increases this by 1. Before first tool, it´s 0.
uint8_t currenttoolNo, newtoolNo;
float firstZPos, newZPos, Zdiff;
static TaskHandle_t zProbeSyncTaskHandle = NULL;

// Finite state machine and sequence of steps
uint8_t tc_state; // tool change (tc) state machine
#define TOOLCHANGE_IDLE       0 // initial state. tool change switched off. set during machine_init()
#define TOOLCHANGE_INIT       1 // do some reporting at first. initialize G code for this procedure
#define TOOLCHANGE_START      2 // decide, if first or further tool changes and set next status appropriately
#define TOOLCHANGE_ZPROBE_1a  3 // Z probe #1. Send order to press the Z probe button
#define TOOLCHANGE_ZPROBE_1b  4 // Z probe #1. After button press
#define TOOLCHANGE_MANUAL     5 // Go to tool change position
#define TOOLCHANGE_ZPROBE_2  6 // Z probe #2. Send order to press the Z probe button
#define TOOLCHANGE_FINISH    99 // tool change finish. do some reporting, clean up, etc.

// declare functions
float getLastZProbePos();

#ifdef USE_MACHINE_INIT
/*
	machine_init() is called when Grbl_ESP32 first starts. You can use it to do any
	special things your machine needs at startup.

	Prerequisite: add "#define USE_MACHINE_INIT" to your machine.h file
*/
void machine_init()
{
	// We start with 0 tool changes
	AmountOfToolChanges=0;

	// unknown at the beginning, maybe there is no special tool loaded. But this will change, if the next tool is loaded
	currenttoolNo = 0;

	// Initialize state machine
	tc_state=TOOLCHANGE_IDLE;

	// TODO this task runs permanently. Alternative?
	xTaskCreatePinnedToCore(zProbeSyncTask,         // task
							"zProbeSyncTask",       // name for task
							4096,				    // size of task stack
							NULL,				    // parameters
							1,					    // priority
							&zProbeSyncTaskHandle,  // handle
							0                       // core
						   );
 }
#endif

// state machine
void zProbeSyncTask(void* pvParameters)
{
	TickType_t xLastWakeTime;

	const TickType_t xProbeFrequency = 100; // in ticks
	xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.

	//protocol_buffer_synchronize(); // wait for all previous moves to complete

	for ( ;; )
	{
		switch ( tc_state )
		{

			case TOOLCHANGE_INIT:
				// TODO set AmountOfToolChanges to 0 after job finish
				// Set amount of tool changes
				AmountOfToolChanges++; 
				
				#ifdef DEBUG
					grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_INIT. State=%d\r", tc_state);
					grbl_sendf (CLIENT_ALL, "This is the %d. tool change in this job\r", AmountOfToolChanges);
					grbl_sendf (CLIENT_ALL, "Old tool is #%d (0 means unknown), new tool is #%d\r", currenttoolNo, newtoolNo);
				#endif
				
				// Init. Safe start block. XY plane, mm mode, absolute addressing mode
//				inputBuffer.push("G90 G94\r");
//				inputBuffer.push("G17 G21\r");

				// Switch off spindle
				inputBuffer.push("M05\r");

				tc_state = TOOLCHANGE_START;
				break;
			
			case TOOLCHANGE_START:
				#ifdef DEBUG
					grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_START. State=%d\r", tc_state);
				#endif

				// Measure firstZPos only once. Then adjust G43.2 by comparing firstZPos and newZPos.
				if (AmountOfToolChanges == 1) // first time
					tc_state = TOOLCHANGE_ZPROBE_1a; // measure before manual tool change
				else
					tc_state = TOOLCHANGE_MANUAL; // measure after manual tool change
				break;

			// First Z Probe 
			case TOOLCHANGE_ZPROBE_1a:
				#ifdef DEBUG
					grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_1a. State=%d\r", tc_state);
				#endif

				// Place spindle directly above button in X/Y and at high Z
				inputBuffer.push("G53 G0 Z-5\r");
				inputBuffer.push("G53 G0 X-29 Y-410\r");

				// Z probe
				inputBuffer.push("G91 G38.2 Z-100 F500\r");

				tc_state = TOOLCHANGE_ZPROBE_1b;
				break;

			case TOOLCHANGE_ZPROBE_1b: // wait for button press
				#ifdef DEBUG
					grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_1b. State=%d\r", tc_state);
				#endif

				// wait until we hit Z probe button
				// TODO Error handling. What happens in case the button is not pressed?
				if ( probe_get_state() ) 
				{
					if (AmountOfToolChanges == 1)
						firstZPos = getLastZProbePos(); // save Z pos for comparison later

					// hit the probe
					grbl_sendf(CLIENT_ALL, "Button pressed first time. Z probe pos=%4.3f\r", firstZPos);

					inputBuffer.push("G53 G0 Z-5\r");

					tc_state = TOOLCHANGE_MANUAL;
				}
				break;

			// go to manual tool change position
			case TOOLCHANGE_MANUAL:
				#ifdef DEBUG
					grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_MANUAL. State=%d\r", tc_state);
				#endif

				if ( !probe_get_state() ) // button released now
				{
					// Go to tool change position
					inputBuffer.push("G53 G0 X-5 Y-210\r");

					// Hold
					inputBuffer.push("M0\r");

					// Place spindle directly above button in X/Y and a few mm above Z
					inputBuffer.push("G53 G0 Z-5\r");
					inputBuffer.push("G53 G0 X-29 Y-410\r");

					// Z probe, max. 50mm to press button, quick
					inputBuffer.push("G91 G38.2 Z-100 F500\r");
			
					tc_state = TOOLCHANGE_ZPROBE_2;
				}

				break;

			case TOOLCHANGE_ZPROBE_2: // wait for button press
				#ifdef DEBUG
					// grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_2. State=%d\r", tc_state);
				#endif

				// TODO Error handling. What happens in case the button is not pressed?
				if ( probe_get_state() )
				{
					newZPos = getLastZProbePos(); // save Z pos for later comparison to firstZPos

					// hit the probe
					#ifdef DEBUG
						grbl_sendf (CLIENT_ALL, "Button pressed second time. new Z probe pos=%4.3f\r", newZPos);
					#endif

					// calculate and send out G43.1 adjustment
					char gcode_line[20];
					sprintf(gcode_line, "G43.1 Z%4.3f\r", newZPos-firstZPos);
					inputBuffer.push(gcode_line);
					grbl_sendf (CLIENT_ALL, gcode_line);

					// go up
					inputBuffer.push("G53 G0 Z-5\r");

					tc_state = TOOLCHANGE_FINISH;
				}
				break;

			// That´s it
			case TOOLCHANGE_FINISH:
				#ifdef DEBUG
					grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_FINISH. State=%d\r", tc_state);
				#endif

				// button released, we lift up
				if (! probe_get_state() )
				{
					//vTaskDelay (1 / portTICK_RATE_MS); // 1 sec.
	
					grbl_send (CLIENT_ALL, "Tool change procedure finished.\r");
					grbl_send (CLIENT_ALL, "Go to current WCS origin after hold.\r");

					// go to current WCS origin. This could be G54, but also another one
					inputBuffer.push("G0 X0 Y0\r");
					inputBuffer.push("G0 Z0\r");

				    //protocol_buffer_synchronize(); // wait for all previous moves to complete
				}

				tc_state = TOOLCHANGE_IDLE;
				break;
		}

		vTaskDelayUntil(&xLastWakeTime, xProbeFrequency);
	}
}

#ifdef USE_TOOL_CHANGE
/*
	user_tool_change() is called when tool change gcode is received,
	to perform appropriate actions for your machine.

	Prerequisite: add "#define USE_TOOL_CHANGE" to your machine.h file
*/
void user_tool_change(uint8_t new_tool)
{
	// let´s start with the state machine

	newtoolNo = new_tool;
	tc_state = TOOLCHANGE_INIT;
	sleep(2500);

	while ( zProbeSyncTaskHandle != NULL )
    {
     vTaskDelete( xHandle );
    }

	//TODO
	// Nach Aufruf dieser Function wird gleich wieder zurcügkegeben in die aufrufende Function.
	// Ziel: Erst return, wenn wirklich beendet (RTOS!)
	return;
}
#endif

// return last Z probe machine position
float getLastZProbePos()
{
	int32_t lastZPosition[N_AXIS]; // copy of current location
	float m_pos[N_AXIS];   // machine position in mm
	char output[200];

	memcpy(lastZPosition, sys_probe_position, sizeof(sys_probe_position)); // get current position in step
	system_convert_array_steps_to_mpos(m_pos, lastZPosition); // convert to millimeters

	return m_pos[Z_AXIS];
}
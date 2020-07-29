/*
	custom_code_template.cpp (copy and use your machine name)
	Part of Grbl_ESP32
*/

#define DEBUG 0; // do I want debug messages? yes/no

/*
   VARIABLES
*/
uint8_t AmountOfToolChanges; //  Each new tool increases this by 1. Before first tool, it´s 0.
uint8_t current_toolNo, new_toolNo;
float firstZPos, newZPos, Zdiff;
static TaskHandle_t zProbeSyncTaskHandle = 0;

// Finite state machine.
uint8_t state; // final state machine

#define TOOLCHANGE_IDLE       0 // initial state. tool change switched off. set during machine_init()
#define TOOLCHANGE_INIT       1 // do some reporting at first. tool change number x and so on
#define TOOLCHANGE_START      2 // tool change function called by G code "T[1..n] M06". Not triggered by G38.2 or ESP3D probe function :-)

// first Z probe before tool change
#define TOOLCHANGE_ZPROBE_1a  3 // Z probe #1 (quick). Touch down and order to press the button
#define TOOLCHANGE_ZPROBE_1b  4 // Z probe #1 (quick). Wait for button press.

#define TOOLCHANGE_MANUAL     5 // go to tool change position

// second Z probe after tool change. Now we can compare
#define TOOLCHANGE_ZPROBE_2a  6 // Z probe #2 (slow ). Touch down and order to press the button
#define TOOLCHANGE_ZPROBE_2b  7 // Z probe #2 (slow ). Wait for button press.

#define TOOLCHANGE_FINISH    99 // tool change finish. do some reporting, clean up, etc.

// declare functions
float getCurrentZPos();
float getLastZProbePos();
float getG54Zvalue();
int8_t getState();
bool checkState(uint8_t checkState);
void setState(int8_t newState);
void setState(int8_t newState, uint8_t newtool);


#ifdef USE_MACHINE_INIT
/*
    machine_init() is called when Grbl_ESP32 first starts. You can use it to do any
    special things your machine needs at startup.

    Prerequisite: add "#define USE_MACHINE_INIT" to your machine.h file
*/
void machine_init()
{
    // We start with no tool changes yet
    AmountOfToolChanges=0;

    // unknown at the beginning. But this will change, if the first tool is loaded
    current_toolNo = 0;

    // Initialize state machine
    state=TOOLCHANGE_IDLE;

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

// return current state machine status
int8_t getState()
{
    return state;
}

// check, if given state is the active
bool checkState(uint8_t checkState)
{
    if (state == checkState) return 1;
    else return 0;
}

// setnew state machine status
void setState(int8_t newState)
{
    state = newState;
}

// setnew state machine status
void setState(int8_t newState, uint8_t newtool)
{
    setState(newState);
    new_toolNo = newtool;
}

// state machine
void zProbeSyncTask(void* pvParameters)
{
    TickType_t xLastWakeTime;

    const TickType_t xProbeFrequency = 100; // in ticks (typically ms)
    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.

    for( ;; )
    {
        switch ( getState() )
        {
            //while running

            case TOOLCHANGE_INIT:
                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_INIT. State=%d\r\n", getState());
                #else  
                    grbl_sendf (CLIENT_ALL, "Tool change procedure started.\r\n");
                #endif
    
                // TODO set AmountOfToolChanges to 0 after job finish
                AmountOfToolChanges++;             // Report tool change amount.

                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "This is the %d. tool change in this job\r\n", AmountOfToolChanges);
                    grbl_sendf (CLIENT_ALL, "Old tool is #%d (0 means unknown), new tool is #%d\r\n", current_toolNo, new_toolNo);
                #endif
                
                // Init. Safe start block. G54, XY plane, mm mode, relative addressing mode
                inputBuffer.push("G49\r\n");
                //inputBuffer.push("G54\r\n");
                inputBuffer.push("G53\r\n");
                inputBuffer.push("G17 G21 G90\r\n");
                inputBuffer.push("G0 F100\r\n");

                // Switch off spindle
                inputBuffer.push("M05\r\n");

                setState (TOOLCHANGE_START);
                break;
            
            case TOOLCHANGE_START:
                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_START. State=%d\r\n", getState());
                #endif

                // nur beim ersten Mal die Länge des Fräsers messen. Mit dem Wert alle Weiteren vergleichen und G43.2 berechnen.
                if (AmountOfToolChanges == 1)
                    setState(TOOLCHANGE_ZPROBE_1a);
                else
                    setState(TOOLCHANGE_MANUAL);
                break;

            // First Z Probe 
            case TOOLCHANGE_ZPROBE_1a:
                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_1a. State=%d\r\n", getState());
                #endif

                // Place spindle directly above button in X/Y and a few mm above Z
                inputBuffer.push("G53 G0 Z-5\r\n");
                inputBuffer.push("G53 G0 X-29 Y-410\r\n");

                // Z probe, max. 50mm to press button, quick
                inputBuffer.push("G91 G38.2 Z-110 F500\r\n");
                inputBuffer.push("G4 P1.0\r\n");

                setState(TOOLCHANGE_ZPROBE_1b);
                break;

            case TOOLCHANGE_ZPROBE_1b: // wait for button press
                #ifdef DEBUG
                    // grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_1b. State=%d\r\n", getState());
                #endif

                // TODO Error handling. What happens in case the button is not pressed?
                if ( probe_get_state() )
                {

                    // TODO neue Toollänge immer gegen das erste Tool, richtig?
                    if (AmountOfToolChanges == 1)
                        firstZPos = getLastZProbePos(); // save Z pos for comparison later

                    // hit the probe
                    #ifdef DEBUG
                        grbl_sendf(CLIENT_ALL, "Button pressed first time, quickly. Z probe pos=%4.3f\r\n", firstZPos);
                    #endif

                    inputBuffer.push("G91 G0 Z2\r\n");
                    vTaskDelay (xProbeFrequency * 20); // wait until button is released
                    setState (TOOLCHANGE_MANUAL);
                }
                break;

            // go to manual tool change position
            case TOOLCHANGE_MANUAL:
                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_MANUAL. State=%d\r\n", getState());
                #endif

                // Go to tool change position
                inputBuffer.push("G53 G0 Z-5\r\n");
                inputBuffer.push("G53 G0 X-5 Y-210\r\n");

                // Hold
                inputBuffer.push("M0\r\n");

                setState (TOOLCHANGE_ZPROBE_2a);
                break;

            case TOOLCHANGE_ZPROBE_2a:
                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_2a. State=%d\r\n", getState());
                #endif

                // Place spindle directly above button in X/Y and a few mm above Z
                inputBuffer.push("G53 G0 Z-5\r\n");
                inputBuffer.push("G53 G0 X-29 Y-410\r\n");

                // Z probe, max. 50mm to press button, quick
                inputBuffer.push("G91 G38.2 Z-110 F500\r\n");
                inputBuffer.push("G4 P2.0\r\n");

                setState(TOOLCHANGE_ZPROBE_2b);
                break;

            case TOOLCHANGE_ZPROBE_2b: // wait for button press
                #ifdef DEBUG
                    // grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_ZPROBE_2b. State=%d\r\n", getState());
                #endif

                // TODO Error handling. What happens in case the button is not pressed?
                if ( probe_get_state() )
                {
                    newZPos = getLastZProbePos(); // save Z pos for comparison later

                    // hit the probe
                    #ifdef DEBUG
                        grbl_sendf (CLIENT_ALL, "Button pressed second time, slowly. new Z probe pos=%4.3f\r\n", newZPos);
                     #endif

                    // send out G43.1 adjustment
                    char gcode_line[20];

                    sprintf(gcode_line, "G54 G43.1 Z%4.3f\r\n", newZPos-firstZPos);

                    inputBuffer.push(gcode_line);
                    grbl_sendf (CLIENT_ALL, gcode_line);

                    inputBuffer.push("G91 G0 Z2\r\n");
                    setState (TOOLCHANGE_FINISH);
                }
                break;

            // That´s it
            case TOOLCHANGE_FINISH:
                #ifdef DEBUG
                    grbl_sendf (CLIENT_ALL, "zProbeSyncTask. TOOLCHANGE_FINISH. State=%d\r\n", getState());
                #endif

                grbl_send (CLIENT_ALL, "Tool change procedure finished.\r\n");
                grbl_send (CLIENT_ALL, "Go to G54 after hold.\r\n");
                
                // Hold
                //inputBuffer.push("M0\r\n");

                // Clean up
                //inputBuffer.push("G90\r\n");

                // Go to G54
                // TODO G54 needs to be set before, otherwise bad things could happen
                inputBuffer.push("G53 G0 Z-5\r\n");

                inputBuffer.push("G54");
                inputBuffer.push("G90");
                inputBuffer.push("G0 X0 Y0\r\n");
                inputBuffer.push("G0 Z0\r\n");

                setState (TOOLCHANGE_IDLE);
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
    setState (TOOLCHANGE_INIT, new_tool); // let´s start with the state machine

    /*
        Prerequisites
        - First tool properly mounted in spindle
        - G54 is set. G54/Z touches the work piece surface
        - Reminder: set Autodesk Fusion 360 raw material origin to G54

        What happens with every call to this function

        + Switch off spindle
        + Move to tool change position                          (not for the first call)
        - Hold                                                  (not for the first call)

        - Manual tool change                                    (not for the first call)

        - Proceed by un-hold                                    (not for the first call)
        - Move to Z probe button
        - Quick and then slow Z probe for better precision
        - Save Z probe position to "NEW"
        - Calculate delta between "NEW" and "OLD"
        - Call G43.1 with that delta
        - Copy Z probe position "NEW" to "OLD"
    
        - Go to G54
        - Hold

        - Switch on spindle
        - Proceed with job
    */

    return;
}
#endif

// return current Z machine position
float getCurrentZPos()
{
    int32_t current_position[N_AXIS]; // copy of current location
    float m_pos[N_AXIS];   // machine position in mm

    memcpy(current_position, sys_position, sizeof(sys_position)); // get current position in step
    system_convert_array_steps_to_mpos(m_pos, sys_position); // convert to millimeters

    return m_pos[Z_AXIS];
}

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

// return the stored G54/Z value
float getG54Zvalue()
{
    float coord_data[N_AXIS];

    settings_read_coord_data(0, coord_data); //0=G54

    return coord_data[Z_AXIS];
}
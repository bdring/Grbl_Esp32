/*
	custom_code_template.cpp (copy and use your machine name)
	Part of Grbl_ESP32
*/

uint8_t AmountOfToolChanges; //  Each new tool increases this by 1. Before first tool, it´s 0.
float ZPosOld, ZPosNew;
static TaskHandle_t zProbeSyncTaskHandle = 0;


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
    float m_pos[N_AXIS];   // machine position in mm
    char output[200];

    //memcpy(current_position, sys_position, sizeof(sys_position)); // get current position in step
    system_convert_array_steps_to_mpos(m_pos, sys_probe_position); // convert to millimeters

    return m_pos[Z_AXIS];
}

// return the stored G54/Z value
float getG54Zvalue()
{
    float coord_data[N_AXIS];

    settings_read_coord_data(0, coord_data); //0=G54

    return coord_data[Z_AXIS];
}

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
    ZPosOld=0;

    xTaskCreatePinnedToCore(zProbeSyncTask,   // task
                            "zProbeSyncTask", // name for task
                            4096,				// size of task stack
                            NULL,				// parameters
                            1,					// priority
                            &zProbeSyncTaskHandle,
                            0 // core
                           );
 }
#endif

void zProbeSyncTask(void* pvParameters) {
    int32_t sys_POS_steps[N_AXIS], sys_PROBE_steps[N_AXIS];
    float sys_POS[N_AXIS], sys_PROBEPOS[N_AXIS];
    float m_pos[N_AXIS];			  // machine position in mm
    TickType_t xLastWakeTime;
    char temp[200];

    const TickType_t xProbeFrequency = ZPROBE_TASK_FREQ; // in ticks (typically ms)
    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.

    while (true) {
        // don't ever return from this or the task dies
       
        memcpy(sys_POS_steps, sys_position, sizeof(sys_position));
        memcpy(sys_PROBE_steps, sys_probe_position, sizeof(sys_probe_position));

        system_convert_array_steps_to_mpos(sys_POS, sys_POS_steps);
        system_convert_array_steps_to_mpos(sys_PROBEPOS, sys_PROBE_steps);

        //sprintf(temp, "lprobegestate=%d\r\n", probe_get_state());
        //grbl_send(CLIENT_ALL, temp);

        if ( (probe_get_state()==1) )
        {
            grbl_send(CLIENT_ALL, "Button pressed !");

            sprintf(temp, "syspos/Z=%4.3f, probepos/Z=%4.3f\r\n", sys_POS[Z_AXIS], sys_PROBEPOS[Z_AXIS]);

            //sprintf(temp, "last probe/Z =%4.3f\r\n", ZPosNew);
            grbl_send(CLIENT_ALL, temp);
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
    // Variables
    char temp[200];
    float value1, value2, diff;

    // Start of function
    // Increase AmountOfToolChanges every time by 1

    AmountOfToolChanges++;

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

    // Report tool change amount. During first call it´s 0.
    sprintf(temp, "Tool change #=%d\r\n", AmountOfToolChanges);
    grbl_send(CLIENT_ALL, temp);

    // Init. Safe start block. G54, XY plane, mm mode, relative addressing mode
    inputBuffer.push("G54\r\n");
    inputBuffer.push("G17 G21 G91\r\n");
    inputBuffer.push("G0 F100\r\n");
    protocol_buffer_synchronize();

    // Switch off spindle
    inputBuffer.push("M05\r\n");
    protocol_buffer_synchronize();

    // Go to tool change position
    inputBuffer.push("G53 G0 Z-5\r\n");
    inputBuffer.push("G53 G0 X-5 Y-210\r\n");
    protocol_buffer_synchronize();

    // Hold
    inputBuffer.push("M0\r\n");
    protocol_buffer_synchronize();

     /*
       do some reporting
    */
    //current Z pos. Should be, where the button is pressed
    sprintf(temp, "(1) z pos current at hold=%4.3f\r\n", getCurrentZPos());
    grbl_send(CLIENT_ALL, temp);

    // Place spindle directly above button in X/Y and a few mm above Z
    inputBuffer.push("G53 G0 X-29 Y-410\r\n");
    inputBuffer.push("G53 G0 Z-60 \r\n");
    protocol_buffer_synchronize();

    // Z probe, max. 50mm to press button
    inputBuffer.push("G38.2 Z-50 F250\r\n");
    protocol_buffer_synchronize();

    // Raise spindle a little bit for slower try to increase accuracy
    inputBuffer.push("G0 Z1.5\r\n");
    protocol_buffer_synchronize();

    // Z probe again
    inputBuffer.push("G38.2 Z-2 F30\r\n");
    inputBuffer.push("G4 P2.0\r\n");
    protocol_buffer_synchronize(); // wait for all previous moves to complete

    // Clean up
    inputBuffer.push("G90\r\n");
    protocol_buffer_synchronize(); // wait for all previous moves to complete

    /*
       do some reporting
    */
    //current Z pos. Should be, where the button is pressed
    sprintf(temp, "(2) z pos current=%4.3f\r\n", getCurrentZPos());
    grbl_send(CLIENT_ALL, temp);

    //Z probe position. Should be identical to getCurrentZPos() here
    sprintf(temp, "(3) z probe pos=%4.3f\r\n", getLastZProbePos());
    grbl_send(CLIENT_ALL, temp);
    
    //get G54/Z position, as manually configured by jogging. WORKS!
    sprintf(temp, "(4) z G54 pos=%4.3f\r\n", getG54Zvalue());
    grbl_send(CLIENT_ALL, temp);

    return;
}
#endif
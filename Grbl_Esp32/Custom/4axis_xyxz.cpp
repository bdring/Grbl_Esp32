/*
	custom_code_template.cpp (copy and use your machine name)
	Part of Grbl_ESP32
*/

uint8_t AmountOfToolChanges;

#ifdef USE_MACHINE_INIT
/*
machine_init() is called when Grbl_ESP32 first starts. You can use it to do any
special things your machine needs at startup.
*/
void machine_init()
{
    // First call means AmountOfToolChanges=0.
    AmountOfToolChanges=0;

    grbl_send(CLIENT_ALL, "machine_init");
}
#endif

#ifdef USE_TOOL_CHANGE
/*
  user_tool_change() is called when tool change gcode is received,
  to perform appropriate actions for your machine.
*/

float getCurrentZPos()
{
    int32_t current_position[N_AXIS]; // copy of current location
    float m_pos[N_AXIS];   // machine position in mm

    memcpy(current_position, sys_position, sizeof(sys_position)); // get current position in step
    system_convert_array_steps_to_mpos(m_pos, sys_position); // convert to millimeters

    return m_pos[Z_AXIS];
}

float getLastZProbePos()
{
    float m_pos[N_AXIS];   // machine position in mm
    char output[200];

    //memcpy(current_position, sys_position, sizeof(sys_position)); // get current position in step
    system_convert_array_steps_to_mpos(m_pos, sys_probe_position); // convert to millimeters

    return m_pos[Z_AXIS];
}

float getG54Zvalue()
{
    float coord_data[N_AXIS];

    settings_read_coord_data(0, coord_data); //0=G54

    return coord_data[Z_AXIS];
}

void user_tool_change(uint8_t new_tool)
{
    // Variables
    char temp[200];
    float value1, value2, diff;

    // Start of function
    // AmountOfToolChanges=AmountOfToolChanges+1 each time.

    AmountOfToolChanges++;

    sprintf(temp, "Tool change amount=%d\r\n", AmountOfToolChanges);
    grbl_send(CLIENT_ALL, temp);

    // Init. Safe start block. G54, XY plane, mm mode, relative addressing mode
    inputBuffer.push("G54\r\n");
    inputBuffer.push("G17 G21 G91\r\n");

    // Switch off spindle
    inputBuffer.push("M05\r\n");

    /*
    // Set speed
    inputBuffer.push("F100\r\n");

    // Place spindle directly above button in X/Y and a few mm above Z
    inputBuffer.push("G53 G0 Z-5 F500\r\n");
    inputBuffer.push("G53 G0 X-29 Y-410 F100\r\n");
    inputBuffer.push("G53 G0 Z-60 F100\r\n");

    // Z probe, max. 50mm to find button
    inputBuffer.push("G38.2 Z-50 F250\r\n");

    // Raise spindle a little bit
    inputBuffer.push("G0 Z1.5\r\n");

    // Z probe again
    inputBuffer.push("G38.2 Z-2 F30\r\n");
    */


    protocol_buffer_synchronize(); // wait for all previous moves to complete


    // Z Probe Pos.
    value1=getCurrentZPos();
    sprintf(temp, "z probe pos=%4.3f\r\n", value1);
    grbl_send(CLIENT_ALL, temp); // send the report


    // Raise spindle a little bit
    inputBuffer.push("G0 Z1.5\r\n");
    protocol_buffer_synchronize(); // wait for all previous moves to complete

    // Z Probe Pos.
    value2=getCurrentZPos();
    sprintf(temp, "z probe pos neu=%4.3f\r\n",  value2);
    grbl_send(CLIENT_ALL, temp); // send the report

    diff = value2 - value1;
    sprintf(temp, "diff=%4.3f mm\r\n",  diff);
    grbl_send(CLIENT_ALL, temp); // send the report

    

    //sprintf(temp, "z pos=%4.3f\r\n", getCurrentZPos());
    //grbl_send(CLIENT_ALL, temp); // send the report

    //sprintf(temp, "z G54=%4.3f\r\n", getG54Zvalue());
    //grbl_send(CLIENT_ALL, temp); // send the report


  
    

    // Clean up
    inputBuffer.push("G90\r\n");


    protocol_buffer_synchronize(); // wait for all previous moves to complete

    return;
}
#endif
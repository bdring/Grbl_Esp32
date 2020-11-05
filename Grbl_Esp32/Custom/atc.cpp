/*
 Testing ATC ideas


*/





void machine_init() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Machine Init");
}


/*

*/
void user_tool_change(uint8_t new_tool) {
    char gcode_line[20];

    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC use tool %d", new_tool);
    protocol_buffer_synchronize();  // wait for all previous moves to complete

    sprintf(gcode_line, "G53G0Z-1\r");  //
    gc_execute_line(gcode_line, CLIENT_INPUT);    

    sprintf(gcode_line, "G4P3.0");  //
    gc_execute_line(gcode_line, CLIENT_INPUT);

    sprintf(gcode_line, "G0G0Z0");  //
    gc_execute_line(gcode_line, CLIENT_INPUT);
}


// Polar coaster has macro buttons, this handles those button pushes.
void user_defined_macro(uint8_t index) {
    
}

void user_m30() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC M30");
}

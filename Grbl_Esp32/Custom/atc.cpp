/*
 Testing the readiness of Grbl for ATC

 When the M6 command is received, it will wait until all previous moves have completed, then
 inject normal gcode commands to complete the tool change and return to the point where the
 tool change occured.

 This uses a 5 position rack. The first position is a tool setter. The other 4 are 
 ATC collets. 
 
 To grab a tool you go above the tool, open the chuck, go down to the
 grab height,, then close the chuck.

 To release a tool you go to the grab height, open the chuck, drop the tool, raise
 to the top and close the checks

 The spindle must not be spinning when the chuck is open or the ATC seals will be destroyed. If 
 the spindle was on,it will turn as soon as the M6 command is received and set a spin down time. 
 It will do some moves to get to the tool.  If the spin down time is not down it will wait, 
 before activating the chuck. Same on spin up. It will wait before releasing control to the file. 

 Each tool will touch off on the tool setter. This saves the Z Mpos of that position
 for each tool.

 If you zero a tool on the work piece, all tools will use the delta determined by the 
 toolsetter to set the tool length offset.

*/

const int   TOOL_COUNT        = 4;
const int   TOOL_SETTER_INDEX = 0;
const float TOOL_GRAB_TIME    = 0.25;  // seconds. How long it takes to grab a tool
const float RACK_SAFE_DIST    = 25.0;  // how far in front of rack is safe to move in X

typedef struct {
    float mpos[MAX_N_AXIS];    // the pickup location in machine coords
    float offset[MAX_N_AXIS];  // TLO from the zero'd tool
} tool_t;

tool_t tool[TOOL_COUNT + 1];  // one setter, plus 4 tools

float top_of_z;                     // The highest Z position we can move around on
bool  tool_setter_probing = false;  // used to determine if current probe cycle is for the setter
int   zeroed_tool_index   = 0;      // Which tool was zero'd on the work piece

uint8_t current_tool = 0;

void go_above_tool(uint8_t tool_num);
void return_tool(uint8_t tool_num);
bool atc_tool_setter();
bool set_ATC_open(bool open);
void gc_exec_linef(bool sync_after, const char* format, ...);

void user_machine_init() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Machine Init");

    pinMode(ATC_RELEASE_PIN, OUTPUT);

    // the tool setter
    tool[TOOL_SETTER_INDEX].mpos[X_AXIS] = 5.0;
    tool[TOOL_SETTER_INDEX].mpos[Y_AXIS] = 130.0;
    tool[TOOL_SETTER_INDEX].mpos[Z_AXIS] = -25.0;  // Mpos before collet face triggers probe

    tool[1].mpos[X_AXIS] = 35.0;
    tool[1].mpos[Y_AXIS] = 130.0;
    tool[1].mpos[Z_AXIS] = -20.0;

    tool[2].mpos[X_AXIS] = 65.0;
    tool[2].mpos[Y_AXIS] = 130.0;
    tool[2].mpos[Z_AXIS] = -20.0;

    tool[3].mpos[X_AXIS] = 95.0;
    tool[3].mpos[Y_AXIS] = 130.0;
    tool[3].mpos[Z_AXIS] = -20.0;

    tool[4].mpos[X_AXIS] = 125.0;
    tool[4].mpos[Y_AXIS] = 130.0;
    tool[4].mpos[Z_AXIS] = -20.0;

    top_of_z = limitsMaxPosition(Z_AXIS) - homing_pulloff->get();
}

bool user_tool_change(uint8_t new_tool) {
    bool     spindle_was_on  = false;
    bool     was_incremental = false;      // started in G91 mode
    uint64_t spindle_spin_delay;           // used to make sure spindle has fully spun down and up.
    float    saved_mpos[MAX_N_AXIS] = {};  // the position before the tool change

    if (new_tool == current_tool)  // if no change, we are done
        return true;

    if (new_tool > TOOL_COUNT) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Tool out of range:%d", new_tool);
        return false;
    }

    protocol_buffer_synchronize();                                 // wait for all previous moves to complete
    system_convert_array_steps_to_mpos(saved_mpos, sys_position);  // save current position so we can return to it

    // see if we need to switch out of incremental mode
    if (gc_state.modal.distance == Distance::Incremental) {
        gc_exec_linef(false, "G90");
        was_incremental = true;
    }

    // is spindle on? Turn it off and determine when the spin down should be done.
    if (gc_state.modal.spindle != SpindleState::Disable) {
        spindle_was_on = true;
        gc_exec_linef(false, "M5");
    }
    // spindle could have been turned off in gcode before M6
    spindle_spin_delay = esp_timer_get_time() + (spindle_delay_spindown->get() * 1000.0);  // When will spindle spindown be done.

    return_tool(current_tool);

    if (new_tool == TOOL_SETTER_INDEX) {  // if changing to tool 0...we are done.
        current_tool = new_tool;
        return true;
    }

    go_above_tool(new_tool);

    uint64_t current_time = esp_timer_get_time();
    if (current_time < spindle_spin_delay) {
        vTaskDelay(spindle_spin_delay - current_time);
    }

    set_ATC_open(true);                                               // open ATC
    gc_exec_linef(true, "G53G0Z%0.3f", tool[new_tool].mpos[Z_AXIS]);  // drop down to tool
    set_ATC_open(false);                                              // Close ATC
    gc_exec_linef(true, "G4P%0.2f", TOOL_GRAB_TIME);                  // wait for grab to complete and settle
    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);                    // Go to top of Z travel
    // move in front of tool
    gc_exec_linef(false, "G53G0X%0.3fY%0.3f", tool[new_tool].mpos[X_AXIS], tool[new_tool].mpos[Y_AXIS] - RACK_SAFE_DIST);

    current_tool = new_tool;

    if (!atc_tool_setter()) {  // check the length of the tool
        return false;
    }

    // If the spindle was on before we started, we need to turn it back on.
    if (spindle_was_on) {
        gc_exec_linef(false, "M3");
        spindle_spin_delay = esp_timer_get_time() + (spindle_delay_spinup->get() * 1000.0);  // When will spindle spindown be done
    }

    gc_exec_linef(false, "G53G0X%0.3fY%0.3fZ%0.3f", saved_mpos[X_AXIS], saved_mpos[Y_AXIS], saved_mpos[Z_AXIS]);  // return to saved mpos

    // was was_incremental on? If so, return to that state
    if (was_incremental) {
        gc_exec_linef(false, "G91");
    }

    // Wait for spinup
    if (spindle_was_on) {
        uint64_t current_time = esp_timer_get_time();
        if (current_time < spindle_spin_delay) {
            vTaskDelay(spindle_spin_delay - current_time);
        }
    }

    return true;
}

void user_probe_notification() {
    float probe_position[MAX_N_AXIS];

    // https://linuxcnc.org/docs/2.6/html/gcode/gcode.html#sec:G43_1
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Probe complete. Setter:%d", tool_setter_probing);

    if (sys.state == State::Alarm) {
        return;  // probe failed
    }

    if (tool_setter_probing) {
        return;  // ignore these probes. They are handled elsewhere.
    }

    zeroed_tool_index = current_tool;
}
// ============= Local functions ==================

void go_above_tool(uint8_t tool_num) {
    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);  // Go to top of Z travel
    // move in front of tool
    gc_exec_linef(false, "G53G0X%0.3fY%0.3f", tool[tool_num].mpos[X_AXIS], tool[tool_num].mpos[Y_AXIS] - RACK_SAFE_DIST);
    gc_exec_linef(true, "G53G0Y%0.3f", tool[tool_num].mpos[Y_AXIS]);  // Move over tool
}

void return_tool(uint8_t tool_num) {
    if (tool_num == 0)
        return;

    go_above_tool(tool_num);
    gc_exec_linef(true, "G53G0Z%0.3f", tool[tool_num].mpos[Z_AXIS]);  // drop down to tool
    set_ATC_open(true);                                               // open ATC
    gc_exec_linef(false, "G4P0.5");                                   // wait
    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);                    // Go to top of Z travel
    set_ATC_open(false);                                              // close ATC
    gc_exec_linef(true, "G53G0X%0.3fY%0.3f", tool[tool_num].mpos[X_AXIS], tool[tool_num].mpos[Y_AXIS] - RACK_SAFE_DIST);  // move forward
}

bool atc_tool_setter() {
    float probe_to;  // Calculated work position
    float probe_position[MAX_N_AXIS];

    go_above_tool(0);

    float wco = gc_state.coord_system[Z_AXIS] + gc_state.coord_offset[Z_AXIS] + gc_state.tool_length_offset;
    probe_to  = tool[TOOL_SETTER_INDEX].mpos[Z_AXIS] - wco;

    // https://linuxcnc.org/docs/2.6/html/gcode/gcode.html#sec:G38-probe
    tool_setter_probing = true;
    gc_exec_linef(true, "G38.2F%0.3fZ%0.3f", 300.0, probe_to);  // probe
    tool_setter_probing = false;

    // Was probe successful?
    if (sys.state == State::Alarm) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Missing Tool?");
        return false;  // fail
    }

    system_convert_array_steps_to_mpos(probe_position, sys_probe_position);
    tool[current_tool].offset[Z_AXIS] = probe_position[Z_AXIS];  // Get the Z height ...

    if (zeroed_tool_index != 0) {
        float tlo = tool[zeroed_tool_index].offset[Z_AXIS] - tool[current_tool].offset[Z_AXIS];
        gc_exec_linef(false, "G43.1Z%0.3f", top_of_z);  // raise up
    }

    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);  // raise up
    // move forward
    gc_exec_linef(false, "G53G0X%0.3fY%0.3f", tool[TOOL_SETTER_INDEX].mpos[X_AXIS], tool[TOOL_SETTER_INDEX].mpos[Y_AXIS] - RACK_SAFE_DIST);

    return true;
}

bool set_ATC_open(bool open) {
    // todo lots of safety checks
    if (gc_state.modal.spindle != SpindleState::Disable) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Fail spindle on during change");
        return false;
    }
    digitalWrite(ATC_RELEASE_PIN, open);
    return true;
}

/*
    Format and send gcode line with optional synchronization
    sync_after: Forces all buffered lines to be completed for line send
    format: a printf style string

*/
void gc_exec_linef(bool sync_after, const char* format, ...) {
    char    loc_buf[100];
    char*   temp = loc_buf;
    va_list arg;
    va_list copy;
    va_start(arg, format);
    va_copy(copy, arg);
    size_t len = vsnprintf(NULL, 0, format, arg);
    va_end(copy);
    if (len >= sizeof(loc_buf)) {
        temp = new char[len + 1];
        if (temp == NULL) {
            return;
        }
    }
    len = vsnprintf(temp, len + 1, format, arg);

    gc_execute_line(temp, CLIENT_INPUT);
    //grbl_sendf(CLIENT_SERIAL, "[ATC GCode:%s]\r\n", temp);
    va_end(arg);
    if (temp != loc_buf) {
        delete[] temp;
    }
    if (sync_after) {
        protocol_buffer_synchronize();
    }
}

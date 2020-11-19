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

 TODO

 If no tool has been zero'd use the offset from tool #1

*/

const int   TOOL_COUNT     = 4;
const int   ETS_INDEX      = 0;     // electronic tool setter index
const float TOOL_GRAB_TIME = 0.25;  // seconds. How long it takes to grab a tool
const float RACK_SAFE_DIST = 25.0;  // how far in front of rack is safe to move in X
#ifndef ATC_MANUAL_CHANGE_TIME
#    define ATC_MANUAL_CHANGE_TIME 2000  // milliseconds ATC is open
#endif

#ifndef ATC_EMPTY_SAFE_HEIGHT
#    define ATC_EMPTY_SAFE_HEIGHT -50.0  // safe X travel over tools while empty
#endif

typedef struct {
    float mpos[MAX_N_AXIS];    // the pickup location in machine coords
    float offset[MAX_N_AXIS];  // TLO from the zero'd tool
} tool_t;

tool_t tool[TOOL_COUNT + 1];  // one ETS, plus 4 tools

float top_of_z;                     // The highest Z position we can move around on
bool  tool_setter_probing = false;  // used to determine if current probe cycle is for the setter
int   zeroed_tool_index   = 1;      // Which tool was zero'd on the work piece

uint8_t current_tool = 0;

void go_above_tool(uint8_t tool_num);
bool return_tool(uint8_t tool_num);
bool atc_ETS();
bool set_ATC_open(bool open);
void gc_exec_linef(bool sync_after, const char* format, ...);
bool atc_manual_change();

void user_machine_init() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Machine Init");

    pinMode(ATC_RELEASE_PIN, OUTPUT);

    // the tool setter
    tool[ETS_INDEX].mpos[X_AXIS] = 108;
    tool[ETS_INDEX].mpos[Y_AXIS] = 292.0;
    tool[ETS_INDEX].mpos[Z_AXIS] = -60.0;  // Mpos before collet face triggers probe

    tool[1].mpos[X_AXIS] = 151.0;
    tool[1].mpos[Y_AXIS] = 291.0;
    tool[1].mpos[Z_AXIS] = -86.0;

    tool[2].mpos[X_AXIS] = 186.0;
    tool[2].mpos[Y_AXIS] = 291.0;
    tool[2].mpos[Z_AXIS] = -86.0;

    tool[3].mpos[X_AXIS] = 221.0;
    tool[3].mpos[Y_AXIS] = 292.0;
    tool[3].mpos[Z_AXIS] = -86.0;

    tool[4].mpos[X_AXIS] = 256.0;
    tool[4].mpos[Y_AXIS] = 291.0;
    tool[4].mpos[Z_AXIS] = -86.0;

    top_of_z = limitsMaxPosition(Z_AXIS) - homing_pulloff->get();
}

bool user_tool_change(uint8_t new_tool) {
    bool     spindle_was_on  = false;
    bool     was_incremental = false;      // started in G91 mode
    uint64_t spindle_spin_delay;           // used to make sure spindle has fully spun down and up.
    float    saved_mpos[MAX_N_AXIS] = {};  // the position before the tool change

    if (new_tool == current_tool) {  // if no change, we are done
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC existing tool requested:%d", new_tool);
        return true;
    }

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
        spindle_spin_delay = esp_timer_get_time() + (spindle_delay_spindown->get() * 1000.0);  // When will spindle spindown be done.

        // optimize this
        uint64_t current_time = esp_timer_get_time();
        if (current_time < spindle_spin_delay) {
            vTaskDelay(spindle_spin_delay - current_time);
        }
    }

    // ============= Start of tool change ====================

    if (!return_tool(current_tool)) {  // does nothing if we have no tool
        gc_exec_linef(true, "G53 G0 X%0.3f Y%0.3f Z%0.3f", tool[new_tool].mpos[X_AXIS], tool[new_tool].mpos[Y_AXIS], top_of_z);
    }
    current_tool = 0;

    if (new_tool == 0) {  // if changing to tool 0...we are done.
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Changed to tool 0");
        gc_exec_linef(true, "G53 G0 Z%0.3f", top_of_z);
        gc_exec_linef(true, "G53 G0 Y%0.3f", tool[0].mpos[Y_AXIS] - RACK_SAFE_DIST);
        current_tool = new_tool;
        return true;
    }

    gc_exec_linef(true, "G53 G0 X%0.3f Y%0.3f", tool[new_tool].mpos[X_AXIS],
                  tool[new_tool].mpos[Y_AXIS]);  // Go over new tool

    //go_above_tool(new_tool);

    set_ATC_open(true);                                               // open ATC
    gc_exec_linef(true, "G53G0Z%0.3f", tool[new_tool].mpos[Z_AXIS]);  // drop down to tool
    set_ATC_open(false);                                              // Close ATC
    gc_exec_linef(true, "G4P%0.2f", TOOL_GRAB_TIME);                  // wait for grab to complete and settle
    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);                    // Go to top of Z travel

    current_tool = new_tool;

    if (!atc_ETS()) {  // check the length of the tool
        return false;
    }

    // If the spindle was on before we started, we need to turn it back on.
    if (spindle_was_on) {
        gc_exec_linef(false, "M3");
        spindle_spin_delay = esp_timer_get_time() + (spindle_delay_spinup->get() * 1000.0);  // When will spindle spindown be done
    }

    // return to saved mpos in XY
    gc_exec_linef(false, "G53G0X%0.3fY%0.3fZ%0.3f", saved_mpos[X_AXIS], saved_mpos[Y_AXIS], top_of_z);

    // return to saved mpos in Z if it is not outside of work area.
    float adjusted_z = saved_mpos[Z_AXIS] + gc_state.tool_length_offset;
    if (adjusted_z < limitsMaxPosition(Z_AXIS)) {
        gc_exec_linef(
            false, "G53G0X%0.3fY%0.3fZ%0.3f", saved_mpos[X_AXIS], saved_mpos[Y_AXIS], saved_mpos[Z_AXIS] + gc_state.tool_length_offset);
    }
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

    if (sys.state == State::Alarm) {
        return;  // probe failed
    }

    if (tool_setter_probing) {
        return;  // ignore these probes. They are handled elsewhere.
    }

    zeroed_tool_index = current_tool;
}

void user_defined_macro(uint8_t index) {
    switch (index) {
        case 0:
            atc_manual_change();
            break;
        default:
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Undefined macro number:%d", index);
            break;
    }
}

// ============= Local functions ==================$H

void go_above_tool(uint8_t tool_num) {
    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);  // Go to top of Z travel

    if (current_tool != 0) {
        // move in front of tool
        gc_exec_linef(false, "G53G0X%0.3fY%0.3f", tool[tool_num].mpos[X_AXIS], tool[tool_num].mpos[Y_AXIS] - RACK_SAFE_DIST);
    }

    gc_exec_linef(true, "G53G0X%0.3fY%0.3f", tool[tool_num].mpos[X_AXIS], tool[tool_num].mpos[Y_AXIS]);  // Move over tool
}

bool return_tool(uint8_t tool_num) {
    if (tool_num == 0) {
        return false;
    }

    go_above_tool(tool_num);
    gc_exec_linef(true, "G53G0Z%0.3f", tool[tool_num].mpos[Z_AXIS]);  // drop down to tool
    set_ATC_open(true);
    gc_exec_linef(true, "G53G0Z%0.3f", ATC_EMPTY_SAFE_HEIGHT);  // Go just above tools
    set_ATC_open(false);                                        // close ATC

    return true;
}

bool atc_ETS() {
    float probe_to;  // Calculated work position
    float probe_position[MAX_N_AXIS];

    if (current_tool == 1) {
        // we can go straight to the ATC
        gc_exec_linef(true, "G53G0X%0.3fY%0.3f", tool[ETS_INDEX].mpos[X_AXIS], tool[ETS_INDEX].mpos[Y_AXIS]);  // Move over tool
    } else {
        gc_exec_linef(false, "G91");
        // Arc out of current tool
        gc_exec_linef(false, "G2 X-%0.3f Y-%0.3f I-%0.3f F4000", RACK_SAFE_DIST, RACK_SAFE_DIST, RACK_SAFE_DIST);

        // Move it to arc start
        gc_exec_linef(
            false, "G53G0X%0.3fY%0.3f", tool[ETS_INDEX].mpos[X_AXIS] + RACK_SAFE_DIST, tool[ETS_INDEX].mpos[Y_AXIS] - RACK_SAFE_DIST);

        // arc in
        gc_exec_linef(false, "G2 X-%0.3f Y%0.3f J%0.3f F4000", RACK_SAFE_DIST, RACK_SAFE_DIST, RACK_SAFE_DIST);
        gc_exec_linef(false, "G90");
        // Move over tool
        gc_exec_linef(true, "G53G0X%0.3fY%0.3f", tool[ETS_INDEX].mpos[X_AXIS], tool[ETS_INDEX].mpos[Y_AXIS]);
    }

    float wco = gc_state.coord_system[Z_AXIS] + gc_state.coord_offset[Z_AXIS] + gc_state.tool_length_offset;
    probe_to  = tool[ETS_INDEX].mpos[Z_AXIS] - wco;

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
        float tlo = tool[current_tool].offset[Z_AXIS] - tool[zeroed_tool_index].offset[Z_AXIS];
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Tool No:%d TLO:%0.3f", current_tool, tlo);
        // https://linuxcnc.org/docs/2.6/html/gcode/gcode.html#sec:G43_1
        gc_exec_linef(false, "G43.1Z%0.3f", tlo);  // raise up
    }

    gc_exec_linef(false, "G53G0Z%0.3f", top_of_z);  // raise up
    // move forward
    gc_exec_linef(false, "G53G0X%0.3fY%0.3f", tool[ETS_INDEX].mpos[X_AXIS], tool[ETS_INDEX].mpos[Y_AXIS] - RACK_SAFE_DIST);

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

bool atc_manual_change() {
    // if (gc_state.modal.spindle != SpindleState::Disable) {
    //     grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Cannot use ATC with spindle on");
    // }

    if (sys.state != State::Idle) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC manual change only permitted in idle");
        return false;
    }

    if (!set_ATC_open(true)) {  // internally checks spindle state
        return false;
    }

    vTaskDelay(ATC_MANUAL_CHANGE_TIME);

    if (!set_ATC_open(false)) {
        return false;
    }

    return true;
}

/*
    Format and send gcode line with optional synchronization
    sync_after: Forces all buffered lines to be completed for line send
    format: a printf style string

*/
//void grbl_msg_sendf(uint8_t client, MsgLevel level, const char* format, ...);
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

/*
 Testing the readiness of Grbl for ATC

 This uses a 5 position rack. The first position is a tool setter. The other 4 are 
 ATC collets. 
 
 To grab a tool you go above the tool, open the chuck, go down to the
 grab height,, then close the chuck.

 To release a tool you go to the grab height, open the chuck, drop the tool, raise
 to the top and close the checks

 The spindle must not be spinning when the chuck is open.

 Each tool will touch off on the tool setter. This save the Z Mpos of that position
 for each tool.

 If you zero a tool on the work piece, all tools will use the delta determined by the 
 toolsetter to.

*/

const int   TOOL_COUNT     = 4;
const float TOOL_GRAB_TIME = 0.75;  // seconds. How long it takes to grab a tool

// temporary struct....should maybe be a class
typedef struct {
    float mpos[MAX_N_AXIS];    // the pickup location in machine coords
    float offset[MAX_N_AXIS];  // the offset from the zero'd tool
} tool_t;

tool_t tool[TOOL_COUNT + 1];  // one setter, plus 4 tools

void go_above_tool(uint8_t tool_num);
void return_tool(uint8_t tool_num);
void atc_tool_setter();
bool set_ATC_open(bool open);
void gc_exec_linef(const char* format, bool sync_after, ...);

uint8_t current_tool = 0;

void machine_init() {
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Machine Init");

    pinMode(ATC_RELEASE_PIN, OUTPUT);

    // the tool setter
    tool[0].mpos[X_AXIS] = 5.0;
    tool[0].mpos[Y_AXIS] = 130.0;
    tool[0].mpos[Z_AXIS] = -12.0;

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
}

bool user_tool_change(uint8_t new_tool) {
    bool     spindle_was_on = false;
    uint64_t spindle_spin_delay;  // milliseconds
    float    saved_mpos[MAX_N_AXIS] = {};

    if (new_tool == current_tool)
        return true;

    if (new_tool > TOOL_COUNT) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "ATC Tool out of range:%d", new_tool);
        return false;
    }

    protocol_buffer_synchronize();                                 // wait for all previous moves to complete
    system_convert_array_steps_to_mpos(saved_mpos, sys_position);  // save current position so we can return

    // is spindle on? Turn it off and determine when the spin down should be done.
    if (gc_state.modal.spindle != SpindleState::Disable) {
        spindle_was_on = true;
        gc_exec_linef("M5", false);
        spindle_spin_delay = esp_timer_get_time() + (spindle_delay_spindown->get() * 1000.0);  // When has spindle stopped
    }

    return_tool(current_tool);  // does nothing if current tool is 0

    // if changing to tool 0...we are done.
    if (new_tool == 0) {
        current_tool = new_tool;
        return true;
    }

    // TODO Check for G91...might not matter in G53

    go_above_tool(new_tool);

    // if spindle was on has the spindle down period completed? If not wait.
    if (spindle_was_on) {
        uint64_t current_time = esp_timer_get_time();
        if (current_time < spindle_spin_delay) {
            vTaskDelay(spindle_spin_delay - current_time);
        }
    }

    set_ATC_open(true);  // open ATC

    gc_exec_linef("G53G0Z%0.3f", true, tool[new_tool].mpos[Z_AXIS]);  // drop down to tool

    set_ATC_open(false);  // Close ATC

    gc_exec_linef("G4P%0.2f", true, TOOL_GRAB_TIME);  // wait for grab to complete and settle
    gc_exec_linef("G53G0Z-1", false);                 // raise to top of Z
    gc_exec_linef("G53G0X%0.3fY%0.3f", false, tool[new_tool].mpos[X_AXIS], tool[new_tool].mpos[Y_AXIS] - 20.0);  // move to X Y in front of tool

    current_tool = new_tool;

    atc_tool_setter();

    // was spindle on?
    if (spindle_was_on) {
        gc_exec_linef("M3", false);
    }

    gc_exec_linef("G53G0X%0.3fY%0.3fZ%0.3f", false, , saved_mpos[X_AXIS], saved_mpos[Y_AXIS], saved_mpos[Z_AXIS]);  // return to saved mpos

    // TODO wait for spinup

    return true;
}

// ============= Local functions ==================

void go_above_tool(uint8_t tool_num) {
    gc_exec_linef("G53G0Z-1", false);                                                                            // raise up
    gc_exec_linef("G53G0X%0.3fY%0.3f", false, tool[tool_num].mpos[X_AXIS], tool[tool_num].mpos[Y_AXIS] - 20.0);  // move in front of tool
    gc_exec_linef("G53G0Y%0.3f", true, tool[tool_num].mpos[Y_AXIS]);                                             // Move over tool
}

void return_tool(uint8_t tool_num) {
    if (tool_num == 0)
        return;

    go_above_tool(tool_num);

    gc_exec_linef("G53G0Z%0.3f", true, tool[tool_num].mpos[Z_AXIS]);  // drop down to tool
    set_ATC_open(true);                                               // open ATC
    gc_exec_linef("G4P0.5", false);                                   // wait
    gc_exec_linef("G53G0Z-1", true);                                  // raise up
    set_ATC_open(false);                                              // close ATC

    gc_exec_linef("G53G0X%0.3fY%0.3f", true, tool[tool_num].mpos[X_AXIS], tool[tool_num].mpos[Y_AXIS] - 20.0);  // move forward
}

void atc_tool_setter() {
    float print_position[MAX_N_AXIS];

    go_above_tool(0);

    // TODO Fast Probe...Slow Probe?

    gc_exec_linef("G38.2F%0.3fZ%0.3f", true, 300.0, -15);  // probe

    // TODO was probe successful?

    system_convert_array_steps_to_mpos(print_position, sys_probe_position);
    tool[current_tool].offset[Z_AXIS] = print_position[Z_AXIS];  // Get the Z height ...

    gc_exec_linef("G53G0Z-1", false);  // raise up

    // move forward
    gc_exec_linef("G53G0X%0.3fY%0.3f", false, tool[0].mpos[X_AXIS], tool[0].mpos[Y_AXIS] - 20.0);
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
Formats and sends a gcode line
char* format = a printf style string like "G0X%0.3fY0.3f"

*/
void gc_exec_linef(const char* format, bool sync_after, ...) {
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

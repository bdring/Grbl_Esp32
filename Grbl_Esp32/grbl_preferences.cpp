#include "grbl.h"



Preferences grbl_preferences;

void grbl_preferences_init() {
    grbl_preferences.begin(GRBL_PREFERENCES_NAMESPACE, false); // readonly = false
    spindle_read_prefs(grbl_preferences); // 
    grbl_preferences.end();

}

bool grbl_prefs_change(const char* line, int* cmd, String& cmd_params) {
    String buffer = line;
    String key = "";
    String value = "";   

    // grbl_preferences.begin(GRBL_PREFERENCES_NAMESPACE, false); // readonly = false

    // TODO probably need more error checking
    int close_bracket = buffer.indexOf("]", 2); // find the closing bracket
    if (close_bracket > -1) {
        key = buffer.substring(2, close_bracket);
        if (close_bracket < buffer.length()) {
                value = buffer.substring(close_bracket + 1);
        } else {
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "no value found");
            return false;
        }
    }

    switch (buffer[2]) {
        case 'S':
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Hey spindle please set key:%s to value:%s", key.c_str(), value.c_str());
            return true;
        break;
        default:
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Unknown setting prefix %c",  buffer[2]);
    }    
    // grbl_preferences.end();
    
    return false;
}
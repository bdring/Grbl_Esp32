#include "grbl.h"

#define GRBL_PREFERENCES_NAMESPACE "Grbl_ESP32"

Preferences grbL_preferences;

void grbl_preferences_init() {
    grbL_preferences.begin(GRBL_PREFERENCES_NAMESPACE, false); // readonly = false
    spindle_read_prefs(grbL_preferences); // 
    grbL_preferences.end();

}
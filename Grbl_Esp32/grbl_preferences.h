
#ifndef GRBL_PREFERENCES_H
#define  GRBL_PREFERENCES_H

#define GRBL_PREFERENCES_NAMESPACE "Grbl_ESP32"

#include "grbl.h"

extern Preferences grbl_preferences;

void grbl_preferences_init();
bool grbl_prefs_change(const char* line, int* cmd, String& cmd_params);

#endif
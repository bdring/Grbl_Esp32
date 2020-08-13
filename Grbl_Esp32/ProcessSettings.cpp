#include "grbl.h"
#include <map>

// WG Readable and writable as guest
// WU Readable and writable as user and admin
// WA Readable as user and admin, writable as admin

// If authentication is disabled, auth_level will be LEVEL_ADMIN
bool auth_failed(Word* w, const char* value, auth_t auth_level) {
    permissions_t permissions = w->getPermissions();
    switch (auth_level) {
        case LEVEL_ADMIN:  // Admin can do anything
            return false;  // Nothing is an Admin auth fail
        case LEVEL_GUEST:  // Guest can only access open settings
            return permissions != WG;  // Anything other than RG is Guest auth fail
        case LEVEL_USER:  // User is complicated...
            if (!value) {      // User can read anything
                return false;  // No read is a User auth fail
            }
            return permissions == WA;  // User cannot write WA
        default:
            return true;
    }
}

void show_setting(const char* name, const char* value, const char* description, ESPResponseStream* out) {
    grbl_sendf(out->client(), "$%s=%s", name, value);
    if (description) {
        grbl_sendf(out->client(), "    %s", description);
    }
    grbl_sendf(out->client(), "\r\n");
}

void settings_restore(uint8_t restore_flag) {
    #ifdef WIFI_OR_BLUETOOTH
        if (restore_flag & SETTINGS_RESTORE_WIFI_SETTINGS) {
            #ifdef ENABLE_WIFI
                    wifi_config.reset_settings();
            #endif
            #ifdef ENABLE_BLUETOOTH
                    bt_config.reset_settings();
            #endif
        }
    #endif
    if (restore_flag & SETTINGS_RESTORE_DEFAULTS) {
        bool restore_startup = restore_flag & SETTINGS_RESTORE_STARTUP_LINES;
        for (Setting* s = Setting::List; s; s = s->next()) {
            if (!s->getDescription()) {
                const char* name = s->getName();
                if (restore_startup) // all settings get restored
                    s->setDefault(); 
                else if ((strcmp(name, "Line0") != 0) && (strcmp(name, "Line1") != 0)) // non startup settings get restored
                    s->setDefault();
            }
        }
    }
    if (restore_flag & SETTINGS_RESTORE_PARAMETERS) {
        uint8_t idx;
        float coord_data[N_AXIS];
        memset(&coord_data, 0, sizeof(coord_data));
        for (idx = 0; idx <= SETTING_INDEX_NCOORD; idx++)  settings_write_coord_data(idx, coord_data);
    }
    if (restore_flag & SETTINGS_RESTORE_BUILD_INFO) {
        EEPROM.write(EEPROM_ADDR_BUILD_INFO, 0);
        EEPROM.write(EEPROM_ADDR_BUILD_INFO + 1, 0); // Checksum
        EEPROM.commit();
    }
}

// Get settings values from non volatile storage into memory
void load_settings()
{
    for (Setting *s = Setting::List; s; s = s->next()) {
        s->load();
    }
}

extern void make_settings();
extern void make_grbl_commands();
extern void make_web_settings();
void settings_init()
{
    EEPROM.begin(EEPROM_SIZE);
    make_settings();
    make_web_settings();
    make_grbl_commands();
    load_settings();
}

// TODO Settings - jog may need to be special-cased in the parser, since
// it is not really a setting and the entire line needs to be
// sent to gc_execute_line.  It is probably also more time-critical
// than actual settings, which change infrequently, so handling
// it early is probably prudent.
uint8_t jog_set(uint8_t *value, auth_t auth_level, ESPResponseStream* out) {
    // Execute only if in IDLE or JOG states.
    if (sys.state != STATE_IDLE && sys.state != STATE_JOG)  return STATUS_IDLE_ERROR;

    // restore the $J= prefix because gc_execute_line() expects it
#define MAXLINE 128
    char line[MAXLINE];
    strcpy(line, "$J=");
    strncat(line, (char *)value, MAXLINE-strlen("$J=")-1);

    return gc_execute_line(line, out->client()); // NOTE: $J= is ignored inside g-code parser and used to detect jog motions.
}

err_t show_grbl_help(const char* value, auth_t auth_level, ESPResponseStream* out) {
    report_grbl_help(out->client());
    return STATUS_OK;
}

err_t report_gcode(const char *value, auth_t auth_level, ESPResponseStream* out) {
    report_gcode_modes(out->client());
    return STATUS_OK;
}

void show_grbl_settings(ESPResponseStream* out, type_t type, bool wantAxis) {
    for (Setting *s = Setting::List; s; s = s->next()) {
        if (s->getType() == type && s->getGrblName()) {
            bool isAxis = s->getAxis() != NO_AXIS;
            // The following test could be expressed more succinctly with XOR,
            // but is arguably clearer when written out
            if ((wantAxis && isAxis) || (!wantAxis && !isAxis)) {
                show_setting(s->getGrblName(), s->getCompatibleValue(), NULL, out);
            }
        }
    }
}
err_t report_normal_settings(const char* value, auth_t auth_level, ESPResponseStream* out) {
    show_grbl_settings(out, GRBL, false);     // GRBL non-axis settings
    show_grbl_settings(out, GRBL, true);      // GRBL axis settings
    return STATUS_OK;
}
err_t report_extended_settings(const char* value, auth_t auth_level, ESPResponseStream* out) {
    show_grbl_settings(out, GRBL, false);     // GRBL non-axis settings
    show_grbl_settings(out, EXTENDED, false); // Extended non-axis settings
    show_grbl_settings(out, GRBL, true);      // GRBL axis settings
    show_grbl_settings(out, EXTENDED, true);  // Extended axis settings
    return STATUS_OK;
}
err_t list_grbl_names(const char* value, auth_t auth_level, ESPResponseStream* out)
{
    for (Setting *s = Setting::List; s; s = s->next()) {
        const char* gn = s->getGrblName();
        if (gn) {
            grbl_sendf(out->client(), "$%s => $%s\r\n", gn, s->getName());
        }
    }
    return STATUS_OK;
}
err_t list_settings(const char* value, auth_t auth_level, ESPResponseStream* out)
{
    for (Setting *s = Setting::List; s; s = s->next()) {
        const char *displayValue = auth_failed(s, value, auth_level)
                ? "<Authentication required>"
                : s->getStringValue();
        show_setting(s->getName(), displayValue, NULL, out);
    }
    return STATUS_OK;
}
err_t list_commands(const char* value, auth_t auth_level, ESPResponseStream* out)
{
    for (Command *cp = Command::List; cp; cp = cp->next()) {
        const char* name = cp->getName();
        const char* oldName = cp->getGrblName();
        if (oldName) {
            grbl_sendf(out->client(), "$%s or $%s", name, oldName);
        } else {
            grbl_sendf(out->client(), "$%s", name);
        }
        const char* description = cp->getDescription();
        if (description) {
            grbl_sendf(out->client(), " =%s", description);
        }
        grbl_sendf(out->client(), "\r\n");
    }
    return STATUS_OK;
}
err_t toggle_check_mode(const char* value, auth_t auth_level, ESPResponseStream* out) {
    // Perform reset when toggling off. Check g-code mode should only work if Grbl
    // is idle and ready, regardless of alarm locks. This is mainly to keep things
    // simple and consistent.
    if (sys.state == STATE_CHECK_MODE) {
        mc_reset();
        report_feedback_message(MESSAGE_DISABLED);
    } else {
        if (sys.state)  return (STATUS_IDLE_ERROR);  // Requires no alarm mode.
        sys.state = STATE_CHECK_MODE;
        report_feedback_message(MESSAGE_ENABLED);
    }
    return STATUS_OK;
}
err_t disable_alarm_lock(const char* value, auth_t auth_level, ESPResponseStream* out) {
    if (sys.state == STATE_ALARM) {
        // Block if safety door is ajar.
        if (system_check_safety_door_ajar())
            return (STATUS_CHECK_DOOR);
        report_feedback_message(MESSAGE_ALARM_UNLOCK);
        sys.state = STATE_IDLE;
        // Don't run startup script. Prevents stored moves in startup from causing accidents.
    } // Otherwise, no effect.
    return STATUS_OK;
}
err_t report_ngc(const char* value, auth_t auth_level, ESPResponseStream* out) {
    report_ngc_parameters(out->client());
    return STATUS_OK;
}
err_t home(int cycle) {
    if (homing_enable->get() == false)
        return (STATUS_SETTING_DISABLED);
    if (system_check_safety_door_ajar())
        return (STATUS_CHECK_DOOR); // Block if safety door is ajar.
    sys.state = STATE_HOMING;     // Set system state variable
    mc_homing_cycle(cycle);
    if (!sys.abort) {                         // Execute startup scripts after successful homing.
        sys.state = STATE_IDLE; // Set to IDLE when complete.
        st_go_idle();           // Set steppers to the settings idle state before returning.
        if (cycle == HOMING_CYCLE_ALL) {
            char line[128];
            system_execute_startup(line);
        }
    }
    return STATUS_OK;
}
err_t home_all(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_ALL);
}
err_t home_x(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_X);
}
err_t home_y(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_Y);
}
err_t home_z(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_Z);
}
err_t home_a(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_A);
}
err_t home_b(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_B);
}
err_t home_c(const char* value, auth_t auth_level, ESPResponseStream* out) {
    return home(HOMING_CYCLE_C);
}
err_t sleep_grbl(const char* value, auth_t auth_level, ESPResponseStream* out) {
    system_set_exec_state_flag(EXEC_SLEEP);
    return STATUS_OK;
}
err_t get_report_build_info(const char* value, auth_t auth_level, ESPResponseStream* out) {
    if (!value) {
        char line[128];
        settings_read_build_info(line);
        report_build_info(line, out->client());
        return STATUS_OK;
    }
    #ifdef ENABLE_BUILD_INFO_WRITE_COMMAND
        settings_store_build_info(value);
        return STATUS_OK;
    #else
        return STATUS_INVALID_STATEMENT;
    #endif
}
err_t report_startup_lines(const char* value, auth_t auth_level, ESPResponseStream* out) {
    report_startup_line(0, startup_line_0->get(), out->client());
    report_startup_line(1, startup_line_1->get(), out->client());
    return STATUS_OK;
}

std::map<const char*, uint8_t, cmp_str> restoreCommands = {
    #ifdef ENABLE_RESTORE_EEPROM_DEFAULT_SETTINGS
        { "$", SETTINGS_RESTORE_DEFAULTS },
        { "settings", SETTINGS_RESTORE_DEFAULTS },
    #endif
    #ifdef ENABLE_RESTORE_EEPROM_CLEAR_PARAMETERS
        { "#", SETTINGS_RESTORE_PARAMETERS },
        { "gcode", SETTINGS_RESTORE_PARAMETERS },
    #endif
    #ifdef ENABLE_RESTORE_EEPROM_WIPE_ALL
        { "*", SETTINGS_RESTORE_ALL },
        { "all", SETTINGS_RESTORE_ALL },
    #endif
    { "@", SETTINGS_RESTORE_WIFI_SETTINGS },
    { "wifi", SETTINGS_RESTORE_WIFI_SETTINGS },
};
err_t restore_settings(const char* value, auth_t auth_level, ESPResponseStream* out) {
    if (!value) {
        return STATUS_INVALID_STATEMENT;
    }
    auto it = restoreCommands.find(value);
    if (it == restoreCommands.end()) {
        return STATUS_INVALID_STATEMENT;
    }
    settings_restore(it->second);
    return STATUS_OK;
}

err_t showState(const char* value, auth_t auth_level, ESPResponseStream* out) {
    grbl_sendf(out->client(), "State 0x%x\r\n", sys.state);
    return STATUS_OK;
}
err_t doJog(const char* value, auth_t auth_level, ESPResponseStream* out) {
    // For jogging, you must give gc_execute_line() a line that
    // begins with $J=.  There are several ways we can get here,
    // including  $J, $J=xxx, [J]xxx.  For any form other than
    // $J without =, we reconstruct a $J= line for gc_execute_line().
    if (!value) {
        return STATUS_INVALID_STATEMENT;
    }
    char jogLine[LINE_BUFFER_SIZE];
    strcpy(jogLine, "$J=");
    strcat(jogLine, value);
    return gc_execute_line(jogLine, out->client());
}

std::map<uint8_t, const char*> ErrorCodes = {
    { STATUS_OK , "No error", },
    { STATUS_EXPECTED_COMMAND_LETTER , "Expected GCodecommand letter", },
    { STATUS_BAD_NUMBER_FORMAT , "Bad GCode number format", },
    { STATUS_INVALID_STATEMENT , "Invalid $ statement", },
    { STATUS_NEGATIVE_VALUE , "Negative value", },
    { STATUS_SETTING_DISABLED , "Setting disabled", },
    { STATUS_SETTING_STEP_PULSE_MIN , "Step pulse too short", },
    { STATUS_SETTING_READ_FAIL , "Failed to read settings", },
    { STATUS_IDLE_ERROR , "Command requires idle state", },
    { STATUS_SYSTEM_GC_LOCK , "GCode cannot be executed in lock or alarm state", },
    { STATUS_SOFT_LIMIT_ERROR , "Soft limit error", },
    { STATUS_OVERFLOW , "Line too long", },
    { STATUS_MAX_STEP_RATE_EXCEEDED , "Max step rate exceeded", },
    { STATUS_CHECK_DOOR , "Check door", },
    { STATUS_LINE_LENGTH_EXCEEDED , "Startup line too long", },
    { STATUS_TRAVEL_EXCEEDED , "Max travel exceeded during jog", },
    { STATUS_INVALID_JOG_COMMAND , "Invalid jog command", },
    { STATUS_SETTING_DISABLED_LASER , "Laser mode requires PWM output", },
    { STATUS_GCODE_UNSUPPORTED_COMMAND , "Unsupported GCode command", },
    { STATUS_GCODE_MODAL_GROUP_VIOLATION , "Gcode modal group violation", },
    { STATUS_GCODE_UNDEFINED_FEED_RATE , "Gcode undefined feed rate", },
    { STATUS_GCODE_COMMAND_VALUE_NOT_INTEGER , "Gcode command value not integer", },
    { STATUS_GCODE_AXIS_COMMAND_CONFLICT , "Gcode axis command conflict", },
    { STATUS_GCODE_WORD_REPEATED , "Gcode word repeated", },
    { STATUS_GCODE_NO_AXIS_WORDS , "Gcode no axis words", },
    { STATUS_GCODE_INVALID_LINE_NUMBER , "Gcode invalid line number", },
    { STATUS_GCODE_VALUE_WORD_MISSING , "Gcode value word missing", },
    { STATUS_GCODE_UNSUPPORTED_COORD_SYS , "Gcode unsupported coordinate system", },
    { STATUS_GCODE_G53_INVALID_MOTION_MODE , "Gcode G53 invalid motion mode", },
    { STATUS_GCODE_AXIS_WORDS_EXIST , "Gcode extra axis words", },
    { STATUS_GCODE_NO_AXIS_WORDS_IN_PLANE , "Gcode no axis words in plane", },
    { STATUS_GCODE_INVALID_TARGET , "Gcode invalid target", },
    { STATUS_GCODE_ARC_RADIUS_ERROR , "Gcode arc radius error", },
    { STATUS_GCODE_NO_OFFSETS_IN_PLANE , "Gcode no offsets in plane", },
    { STATUS_GCODE_UNUSED_WORDS , "Gcode unused words", },
    { STATUS_GCODE_G43_DYNAMIC_AXIS_ERROR , "Gcode G43 dynamic axis error", },
    { STATUS_GCODE_MAX_VALUE_EXCEEDED , "Gcode max value exceeded", },
    { STATUS_P_PARAM_MAX_EXCEEDED , "P param max exceeded", },
    { STATUS_SD_FAILED_MOUNT , "SD failed mount", },
    { STATUS_SD_FAILED_READ , "SD failed read", },
    { STATUS_SD_FAILED_OPEN_DIR , "SD failed to open directory", },
    { STATUS_SD_DIR_NOT_FOUND , "SD directory not found", },
    { STATUS_SD_FILE_EMPTY , "SD file empty", },
    { STATUS_SD_FILE_NOT_FOUND , "SD file not found", },
    { STATUS_SD_FAILED_OPEN_FILE , "SD failed to open file", },
    { STATUS_SD_FAILED_BUSY , "SD is busy", },
    { STATUS_SD_FAILED_DEL_DIR, "SD failed to delete directory", },
    { STATUS_SD_FAILED_DEL_FILE, "SD failed to delete file", },
    { STATUS_BT_FAIL_BEGIN , "Bluetooth failed to start", },
    { STATUS_WIFI_FAIL_BEGIN , "WiFi failed to start", },
    { STATUS_NUMBER_RANGE , "Number out of range for setting", },
    { STATUS_INVALID_VALUE , "Invalid value for setting", },
    { STATUS_MESSAGE_FAILED , "Failed to send message", },
    { STATUS_NVS_SET_FAILED , "Failed to store setting", },
    { STATUS_AUTHENTICATION_FAILED, "Authentication failed!", },
};

const char* errorString(err_t errorNumber) {
    auto it = ErrorCodes.find(errorNumber);
    return it == ErrorCodes.end() ? NULL : it->second;
}

err_t listErrorCodes(const char* value, auth_t auth_level, ESPResponseStream* out) {
    if (value) {
        char* endptr = NULL;
        uint8_t errorNumber = strtol(value, &endptr, 10);
        if (*endptr) {
            grbl_sendf(out->client(), "Malformed error number: %s\r\n", value);
            return STATUS_INVALID_VALUE;
        }
        const char* errorName = errorString(errorNumber);
        if (errorName) {
            grbl_sendf(out->client(), "%d: %s\r\n", errorNumber, errorName);
            return STATUS_OK;
        } else {
            grbl_sendf(out->client(), "Unknown error number: %d\r\n", errorNumber);
            return STATUS_INVALID_VALUE;
        }
    }

    for (auto it = ErrorCodes.begin();
         it != ErrorCodes.end();
         it++) {
        grbl_sendf(out->client(), "%d: %s\r\n", it->first, it->second);
    }
    return STATUS_OK;
}


// Commands use the same syntax as Settings, but instead of setting or
// displaying a persistent value, a command causes some action to occur.
// That action could be anything, from displaying a run-time parameter
// to performing some system state change.  Each command is responsible
// for decoding its own value string, if it needs one.
void make_grbl_commands() {
    new GrblCommand("",    "Help",  show_grbl_help, ANY_STATE);
    new GrblCommand("T",   "State", showState, ANY_STATE);
    new GrblCommand("J",   "Jog",   doJog, IDLE_OR_JOG);

    new GrblCommand("$",   "GrblSettings/List", report_normal_settings, NOT_CYCLE_OR_HOLD);
    new GrblCommand("+",   "ExtendedSettings/List", report_extended_settings, NOT_CYCLE_OR_HOLD);
    new GrblCommand("L",   "GrblNames/List", list_grbl_names, NOT_CYCLE_OR_HOLD);
    new GrblCommand("S",   "Settings/List",  list_settings, NOT_CYCLE_OR_HOLD);
    new GrblCommand("CMD", "Commands/List",  list_commands, NOT_CYCLE_OR_HOLD);
    new GrblCommand("E",   "ErrorCodes/List",listErrorCodes, ANY_STATE);
    new GrblCommand("G",   "GCode/Modes",    report_gcode, ANY_STATE);
    new GrblCommand("C",   "GCode/Check",    toggle_check_mode, ANY_STATE);
    new GrblCommand("X",   "Alarm/Disable",  disable_alarm_lock, ANY_STATE);
    new GrblCommand("NVX", "Settings/Erase", Setting::eraseNVS, IDLE_OR_ALARM, WA);
    new GrblCommand("V",   "Settings/Stats", Setting::report_nvs_stats, IDLE_OR_ALARM);
    new GrblCommand("#",   "GCode/Offsets",  report_ngc, IDLE_OR_ALARM);
    new GrblCommand("H",   "Home",           home_all, IDLE_OR_ALARM);
    #ifdef HOMING_SINGLE_AXIS_COMMANDS
        new GrblCommand("HX",  "Home/X", home_x, IDLE_OR_ALARM);
        new GrblCommand("HY",  "Home/Y", home_y, IDLE_OR_ALARM);
        new GrblCommand("HZ",  "Home/Z", home_z, IDLE_OR_ALARM);
        #if (N_AXIS > 3)
            new GrblCommand("HA",  "Home/A", home_a, IDLE_OR_ALARM);
        #endif
        #if (N_AXIS > 4)
            new GrblCommand("HB",  "Home/B", home_b, IDLE_OR_ALARM);
        #endif
        #if (N_AXIS > 5)
            new GrblCommand("HC",  "Home/C", home_c, IDLE_OR_ALARM);
        #endif
    #endif
    new GrblCommand("SLP", "System/Sleep", sleep_grbl, IDLE_OR_ALARM);
    new GrblCommand("I",   "Build/Info", get_report_build_info, IDLE_OR_ALARM);
    new GrblCommand("N",   "GCode/StartupLines", report_startup_lines, IDLE_OR_ALARM);
    new GrblCommand("RST", "Settings/Restore", restore_settings, IDLE_OR_ALARM, WA);
};

// normalize_key puts a key string into canonical form -
// without whitespace.
// start points to a null-terminated string.
// Returns the first substring that does not contain whitespace.
// Case is unchanged because comparisons are case-insensitive.
char *normalize_key(char *start) {
    char c;

    // In the usual case, this loop will exit on the very first test,
    // because the first character is likely to be non-white.
    // Null ('\0') is not considered to be a space character.
    while (isspace(c = *start) && c != '\0') {
        ++start;
    }

    // start now points to either a printable character or end of string
    if (c == '\0') {
        return start;
    }

    // Having found the beginning of the printable string,
    // we now scan forward until we find a space character.
    char *end;
    for (end = start; (c = *end) != '\0' && !isspace(c); end++) {
    }

    // end now points to either a whitespace character of end of string
    // In either case it is okay to place a null there
    *end = '\0';

    return start;
}

// This is the handler for all forms of settings commands,
// $..= and [..], with and without a value.
err_t do_command_or_setting(const char *key, char *value, auth_t auth_level, ESPResponseStream* out) {
    // If value is NULL, it means that there was no value string, i.e.
    // $key without =, or [key] with nothing following.
    // If value is not NULL, but the string is empty, that is the form
    // $key= with nothing following the = .  It is important to distinguish
    // those cases so that you can say "$N0=" to clear a startup line.

    // First search the settings list by text name.  If found, set a new
    // value if one is given, otherwise display the current value
    for (Setting *s = Setting::List; s; s = s->next()) {
        if (strcasecmp(s->getName(), key) == 0) {
            if (auth_failed(s, value, auth_level)) {
                return STATUS_AUTHENTICATION_FAILED;
            }
            if (value) {
                return s->setStringValue(value);
            } else {
                show_setting(s->getName(), s->getStringValue(), NULL, out);
                return STATUS_OK;
            }
        }
    }

    // Then search the setting list by compatible name.  If found, set a new
    // value if one is given, otherwise display the current value in compatible mode
    for (Setting *s = Setting::List; s; s = s->next()) {
        if (s->getGrblName() && strcasecmp(s->getGrblName(), key) == 0) {
            if (auth_failed(s, value, auth_level)) {
                return STATUS_AUTHENTICATION_FAILED;
            }
            if (value) {
                return s->setStringValue(value);
            } else {
                show_setting(s->getGrblName(), s->getCompatibleValue(), NULL, out);
                return STATUS_OK;
            }
        }
    }
    // If we did not find a setting, look for a command.  Commands
    // handle values internally; you cannot determine whether to set
    // or display solely based on the presence of a value.
    for (Command *cp = Command::List; cp; cp = cp->next()) {
        if (  (strcasecmp(cp->getName(), key) == 0)
           || (cp->getGrblName()
               && strcasecmp(cp->getGrblName(), key) == 0
              )
           ) {
            if (auth_failed(cp, value, auth_level)) {
                return STATUS_AUTHENTICATION_FAILED;
            }
            return cp->action(value, auth_level, out);
        }
    }

    // If we did not find an exact match and there is no value,
    // indicating a display operation, we allow partial matches
    // and display every possibility.  This only applies to the
    // text form of the name, not to the nnn and ESPnnn forms.
    err_t retval = STATUS_INVALID_STATEMENT;
    if (!value) {
        auto lcKey = String(key);
        // We allow the key string to begin with *, which we remove.
        // This lets us look at X axis settings with $*x.
        // $x by itself is the disable alarm lock command
        if (lcKey.startsWith("*")) {
            lcKey.remove(0,1);
        }
        lcKey.toLowerCase();
        bool found = false;
        for (Setting *s = Setting::List; s; s = s->next()) {
            auto lcTest = String(s->getName());
            lcTest.toLowerCase();

            if (lcTest.indexOf(lcKey) >= 0) {
                const char *displayValue = auth_failed(s, value, auth_level)
                        ? "<Authentication required>"
                        : s->getStringValue();
                show_setting(s->getName(), displayValue, NULL, out);
                found = true;
            }
        }
        if (found) {
            return STATUS_OK;
        }
    }
    return STATUS_INVALID_STATEMENT;
}

uint8_t system_execute_line(char* line, ESPResponseStream* out, auth_t auth_level) {
    remove_password(line, auth_level);

    char *value;
    if (*line++ == '[') { // [ESPxxx] form
        value = strrchr(line, ']');
        if (!value) {
            // Missing ] is an error in this form
            return STATUS_INVALID_STATEMENT;
        }
        // ']' was found; replace it with null and set value to the rest of the line.
        *value++ = '\0';
        // If the rest of the line is empty, replace value with NULL.
        if (*value == '\0') {
            value = NULL;
        }
    } else {
        // $xxx form
        value = strchr(line, '=');
        if (value) {
            // $xxx=yyy form.
            *value++ = '\0';
        }
    }

    char *key = normalize_key(line);

    // At this point there are three possibilities for value
    // NULL - $xxx without =
    // NULL - [ESPxxx] with nothing after ]
    // empty string - $xxx= with nothing after
    // non-empty string - [ESPxxx]yyy or $xxx=yyy
    return do_command_or_setting(key, value, auth_level, out);
}
uint8_t system_execute_line(char* line, uint8_t client, auth_t auth_level) {
    return system_execute_line(line, new ESPResponseStream(client, true), auth_level);
}

void system_execute_startup(char* line) {
    err_t status_code;
    char gcline[256];
    strncpy(gcline, startup_line_0->get(), 255);
    if (*gcline) {
        status_code = gc_execute_line(gcline, CLIENT_SERIAL);
        report_execute_startup_message(gcline, status_code, CLIENT_SERIAL);
    }
    strncpy(gcline, startup_line_1->get(), 255);
    if (*gcline) {
        status_code = gc_execute_line(gcline, CLIENT_SERIAL);
        report_execute_startup_message(gcline, status_code, CLIENT_SERIAL);
    }
}


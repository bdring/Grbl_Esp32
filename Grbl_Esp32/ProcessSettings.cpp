#include "grbl.h"

#ifdef NEW_SETTINGS
#include "SettingsDefinitions.h"
#include <map>


struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return strcmp(a, b) < 0;
   }
};

// The following table maps numbered settings to their settings
// objects, for backwards compatibility.  It is used if the
// line is of the form "$key=value".  If a key match is found,
// the object's setValueString method is invoked with the value
// as argument.  If no match is found, the list of named settings
// is searched, with the same behavior on a match.

// These are compatibility aliases for Classic GRBL
std::map<const char*, Setting*, cmp_str> numberedSettings = {
    #if XXX  // These cannot be initialized statically because the LHS of the -> is NULL then
    // The solution is to add a compat string to the Settings struct so the table goes away.
    { "0", pulse_microseconds },
    { "1", stepper_idle_lock_time },
    { "2", step_invert_mask },
    { "3", dir_invert_mask },
    { "4", step_enable_invert },
    { "5", limit_invert },
    { "6", probe_invert },
    { "10", status_mask },
    { "11", junction_deviation },
    { "12", arc_tolerance },
    { "13", report_inches },
    { "20", soft_limits },
    { "21", hard_limits },
    { "22", homing_enable },
    { "23", homing_dir_mask },
    { "24", homing_feed_rate },
    { "25", homing_seek_rate },
    { "26", homing_debounce },
    { "27", homing_pulloff },
    { "30", rpm_max },
    { "31", rpm_min },
    { "32", laser_mode },
    { "100", x_axis_settings->steps_per_mm },
    { "101", y_axis_settings->steps_per_mm },
    { "102", z_axis_settings->steps_per_mm },
    { "110", x_axis_settings->max_rate },
    { "111", y_axis_settings->max_rate },
    { "112", z_axis_settings->max_rate },
    { "120", x_axis_settings->acceleration },
    { "121", y_axis_settings->acceleration },
    { "122", z_axis_settings->acceleration },
    { "130", x_axis_settings->max_travel },
    { "131", y_axis_settings->max_travel },
    { "132", z_axis_settings->max_travel },
#endif
};

// FIXME - jog may need to be special-cased in the parser, since
// it is not really a setting and the entire line needs to be
// sent to gc_execute_line.  It is probably also more time-critical
// than actual settings, which change infrequently, so handling
// it early is probably prudent.
uint8_t jog_set(uint8_t *value, uint8_t client) {
    // Execute only if in IDLE or JOG states.
    if (sys.state != STATE_IDLE && sys.state != STATE_JOG)  return STATUS_IDLE_ERROR;

    // restore the $J= prefix because gc_execute_line() expects it
#define MAXLINE 128
    char line[MAXLINE];
    strcpy(line, "$J=");
    strncat(line, (char *)value, MAXLINE-strlen("$J=")-1);

    return gc_execute_line(line, client); // NOTE: $J= is ignored inside g-code parser and used to detect jog motions.
}

err_t report_gcode(uint8_t client) {
    report_gcode_modes(client);
    return STATUS_OK;
}
err_t report_normal_settings(uint8_t client) {
    report_grbl_settings(client, false);
    return STATUS_OK;
}
err_t report_extended_settings(uint8_t client) {
    report_grbl_settings(client, true);
    return STATUS_OK;
}
err_t toggle_check_mode(uint8_t client) {
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
err_t disable_alarm_lock(uint8_t client) {
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
err_t report_ngc(uint8_t client) {
    report_ngc_parameters(client);
    return STATUS_OK;
}
err_t home(uint8_t client, int cycle) {
    if (bit_isfalse(settings.flags, BITFLAG_HOMING_ENABLE))
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
err_t home_all(uint8_t client) {
    return home(client, HOMING_CYCLE_ALL);
}
err_t home_x(uint8_t client) {
    return home(client, HOMING_CYCLE_X);
}
err_t home_y(uint8_t client) {
    return home(client, HOMING_CYCLE_Y);
}
err_t home_z(uint8_t client) {
    return home(client, HOMING_CYCLE_Z);
}
err_t home_a(uint8_t client) {
    return home(client, HOMING_CYCLE_A);
}
err_t home_b(uint8_t client) {
    return home(client, HOMING_CYCLE_B);
}
err_t home_c(uint8_t client) {
    return home(client, HOMING_CYCLE_C);
}
err_t sleep_grbl(uint8_t client) {
    system_set_exec_state_flag(EXEC_SLEEP);
    return STATUS_OK;
}
err_t get_report_build_info(uint8_t client) {
    char line[128];
    settings_read_build_info(line);
    report_build_info(line, client);
    return STATUS_OK;
}
err_t report_startup_lines(uint8_t client) {
    report_startup_line(0, startup_line_0->get(), client);
    report_startup_line(1, startup_line_1->get(), client);
    return STATUS_OK;
}

// The following table is used if the line is of the form "$key\n"
// i.e. dollar commands without "="
// The key value is matched against the string and the corresponding
// function is called with no arguments.
// If there is no key match an error is reported
typedef err_t (*Command_t)(uint8_t);
std::map<const char*, Command_t, cmp_str> dollarCommands = {
    { "$", report_normal_settings },
    { "+", report_extended_settings },
    { "G", report_gcode },
    { "C", toggle_check_mode },
    { "X", disable_alarm_lock },
    { "#", report_ngc },
    { "H", home_all },
    { "HX", home_x },
    { "HY", home_y },
    { "HZ", home_z },
    { "HA", home_a },
    { "HB", home_b },
    { "HC", home_c },
    { "SLP", sleep_grbl },
    { "I", get_report_build_info },
    { "N", report_startup_lines },
};
// XXX See Store startup line [IDLE/ALARM]

// normalize_key puts a key string into canonical form -
// upper case without whitespace.
// start points to a null-terminated string.
// Returns the first substring that does not contain whitespace,
// converted to upper case.
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
    // we now scan forward, converting lower to upper case,
    // until we find a space character.
    char *end;
    for (end = start; (c = *end) != '\0' && !isspace(c); end++) {
        if (islower(c)) {
            *end = toupper(c);
        }
    }

    // end now points to either a whitespace character of end of string
    // In either case it is okay to place a null there
    *end = '\0';

    return start;
}

// This is for changing settings with $key=value .
// Lookup key in the numberedSettings map.  If found, execute
// the corresponding object's "set" method.  Otherwise fail.
err_t do_setting(const char *key, char *value) {
    // First search this numberedSettings array - aliases
    // for the underlying named settings.
    std::map<const char*, Setting*, cmp_str>::iterator it = numberedSettings.find(key);
    if (it != numberedSettings.end()) {
        return it->second->setStringValue(value);
    }

    // Then search the list of named settings.
    for (Setting *s = SettingsList; s; s = s->link) {
        if (strcmp(s->getName(), key) == 0) {
            return s->setStringValue(value);
        }
    }

    return STATUS_INVALID_STATEMENT;
}

// This is for bare commands like "$RST" - no equals sign.
// Lookup key in the dollarCommands map.  If found, execute
// the corresponding command.
// As an enhancement to Classic GRBL, if the key is not found
// in the commands map, look it up in the lists of settings
// and display the current value.
err_t do_command(const char *key, uint8_t client) {

    std::map<const char*, Command_t, cmp_str>::iterator i = dollarCommands.find(key);
    if (i != dollarCommands.end()) {
        return i->second(client);
    }

    // Enhancement - not in Classic GRBL:
    // If it is not a command, look up the key
    // as a setting and display the value.
    std::map<const char *, Setting*, cmp_str>::iterator it = numberedSettings.find(key);
    if (it != numberedSettings.end()) {
        Setting *s = it->second;
        grbl_sendf(client, "$%s=%s\n", key, s->getStringValue());
        return STATUS_OK;
    }

    for (Setting *s = SettingsList; s; s = s->next()) {
        //    if (s->getName().compare(k)) {
        if (strcmp(s->getName(), key) == 0) {
            grbl_sendf(client, "$%s=%s\n", key, s->getStringValue());
            return STATUS_OK;
        }
    }

    return STATUS_INVALID_STATEMENT;
}

uint8_t system_execute_line(char* line, uint8_t client) {
    err_t res;
    char *value = strchr(line, '=');
    char *key = normalize_key(line+1);

    if (value) {
        // Equals was found; replace it with null and skip it
        *value++ = '\0';
        res = do_setting(key, value);
    } else {
        // No equals, so it must be a command
        res = do_command(key, client);
    }
    return res;
}
void system_execute_startup(char* line) {
#ifndef XXX
    grbl_sendf(CLIENT_SERIAL, "Skipping system_execute_startup\r\n");
    delay(1000);
#else
    err_t status_code;
    const char *gcline = startup_line_0.get();
    if (*gcline) {
        status_code = gc_execute_line(gcline, CLIENT_SERIAL);
        report_execute_startup_message(gcline, status_code, CLIENT_SERIAL);
    }
    gcline = startup_line_1.get();
    if (*gcline) {
        status_code = gc_execute_line(gcline, CLIENT_SERIAL);
        report_execute_startup_message(gcline, status_code, CLIENT_SERIAL);
    }
#endif
}
void list_settings()
{
    for (Setting *s = SettingsList; s; s = s->link) {
        grbl_sendf(CLIENT_SERIAL, "%s=%s\r\n", s->getName(), s->getStringValue());
    }
}
#if 0

void list_numbered_settings()
{
    for (std::map<const char*, Setting*>::iterator it = numberedSettings.begin();
         it != numberedSettings.end();
         it++) {
        cout << it->first << ": " << it->second->getStringValue() << '\n';
    }
}
#endif
#if TEST_SETTINGS
int main()
{
    list_settings();
}
#endif
#endif

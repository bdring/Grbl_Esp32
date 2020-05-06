#include "grbl.h"

#ifdef NEW_SETTINGS
#include "SettingsDefinitions.h"
#include <map>

struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return strcasecmp(a, b) < 0;
   }
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
#ifdef NEW_SETTINGS
const char *map_grbl_value(const char *value) {
    if (strcmp(value, "Off") == 0) {
        return "0";
    }
    if (strcmp(value, "On") == 0) {
        return "1";
    }
    return value;
}
void show_grbl_settings(uint8_t client, group_t group, bool wantAxis) {
    for (Setting *s = SettingsList; s; s = s->next()) {
        if (s->getGroup() == group && s->getGrblName()) {
            bool isAxis = s->getAxis() != NO_AXIS;
            // The following test could be expressed more succinctly with XOR,
            // but is arguably clearer when written out
            if ((wantAxis && isAxis) || (!wantAxis && !isAxis)) {
                grbl_sendf(client, "$%s=%s\n", s->getGrblName(), map_grbl_value(s->getStringValue()));
            }
        }
    }
    
}
err_t report_normal_settings(uint8_t client) {
    show_grbl_settings(client, GRBL, false);     // GRBL non-axis settings
    show_grbl_settings(client, GRBL, true);      // GRBL axis settings
    return STATUS_OK;
}
err_t report_extended_settings(uint8_t client) {
    show_grbl_settings(client, GRBL, false);     // GRBL non-axis settings
    show_grbl_settings(client, EXTENDED, false); // Extended non-axis settings
    show_grbl_settings(client, GRBL, true);      // GRBL axis settings
    show_grbl_settings(client, EXTENDED, true);  // Extended axis settings
    return STATUS_OK;
}
err_t list_settings(uint8_t client)
{
    for (Setting *s = SettingsList; s; s = s->next()) {
        grbl_sendf(client, "$%s=%s\r\n", s->getName(), s->getStringValue());
    }
    return STATUS_OK;
}
#else
err_t report_normal_settings(uint8_t client) {
    report_grbl_settings(client, false);
    return STATUS_OK;
}
err_t report_extended_settings(uint8_t client) {
    report_grbl_settings(client, true);
    return STATUS_OK;
}
#endif
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
    { "S", list_settings },
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
// FIXME See Store startup line [IDLE/ALARM]

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

// This is for changing settings with $key=value .
// Lookup key in the Settings list, considering both
// the text name and the grbl compatible name, if any.
// If found, execute the object's "setStringValue" method.
// Otherwise fail.
err_t do_setting(const char *key, char *value) {
    // First search this numberedSettings array - aliases
    // for the underlying named settings.
    for (Setting *s = SettingsList; s; s = s->next()) {
        if ((strcasecmp(s->getName(), key) == 0)
        || (s->getGrblName() && (strcmp(s->getGrblName(), key) == 0))) {
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
    for (Setting *s = SettingsList; s; s = s->next()) {
        //    if (s->getName().compare(k)) {
        if ((strcasecmp(s->getName(), key) == 0)
        || (s->getGrblName() && (strcasecmp(s->getGrblName(), key) == 0))) {
            grbl_sendf(client, "$%s=%s\n", s->getName(), s->getStringValue());
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
    err_t status_code;
    const char *gcline = startup_line_0->get();
    if (*gcline) {
        status_code = gc_execute_line(gcline, CLIENT_SERIAL);
        report_execute_startup_message(gcline, status_code, CLIENT_SERIAL);
    }
    gcline = startup_line_1->get();
    if (*gcline) {
        status_code = gc_execute_line(gcline, CLIENT_SERIAL);
        report_execute_startup_message(gcline, status_code, CLIENT_SERIAL);
    }
}
#endif

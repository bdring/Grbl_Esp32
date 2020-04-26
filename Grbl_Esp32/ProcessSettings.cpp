#include "SettingsBaseClass.h"

// The following table maps numbered settings to their settings
// objects, for backwards compatibility.  It is used if the
// line is of the form "$key=value".  If a key match is found,
// the object's setValueString method is invoked with the value
// as argument.  If no match is found, the list of named settings
// is searched, with the same behavior on a match.

// FIXME - jog may need to be special-cased in the parser, since
// it is not really a setting and the entire line needs to be
// sent to gc_execute_line.  It is probably also more time-critical
// than actual settings, which change infrequently, so handling
// it early is probably prudent.
uint8_t jog_set(uint8_t *value, uint8_t client) {
  // Execute only if in IDLE or JOG states.
  if (sys.state != STATE_IDLE && sys.state != STATE_JOG)  return STATUS_IDLE_ERROR;

  // restore the $J= prefix because gc_execute_line() expects it
  char line[MAXLINE];
  strcpy(line, "$J=");
  strncat(line, (char *)value, MAXLINE-strlen("$J=")-1);

  return gc_execute_line(line, client); // NOTE: $J= is ignored inside g-code parser and used to detect jog motions.
}

// The following table is used if the line is of the form "$key\n"
// i.e. dollar commands without "="
// The key value is matched against the string and the corresponding
// function is called with no arguments.
// If there is no key match an error is reported
typedef err_t (*Command_t)(uint8_t);
std::map<const char*, Command_t> dollarCommands = {
    { "$", report_normal_settings },
    { "+", report_extended_settings },
    { "G", report_gcode_modes },
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
    { "SLP", sleep },
    { "I", report_build_info },
    { "N", report_startup_lines },
};


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
  map<const char*, Setting*>::iterator it = numberedSettings.find(key);
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

  map<const char*, Command_t>::iterator i = dollarCommands.find(key);
  if (i != dollarCommands.end()) {
    return i->second(client);
  }

  // Enhancement - not in Classic GRBL:
  // If it is not a command, look up the key
  // as a setting and display the value.

  map<const char *, Setting*>::iterator it = numberedSettings.find(key);
  if (it != numberedSettings.end()) {
    Setting *s = it->second;
    cout << key << "=" << s->getStringValue() << '\n';
    return STATUS_OK;
  }

  for (Setting *s = SettingsList; s; s = s->link) {
    //    if (s->getName().compare(k)) {
    if (strcmp(s->getName(), key) == 0) {
      cout << key << "=" << s->getStringValue() << '\n';
      return STATUS_OK;
    }
  }

  return STATUS_INVALID_STATEMENT;
}

uint8_t system_execute_line(char* line, uint8_t client) {
  err_t res;
  char *value = strchr(line, '=');

  if (value) {
    // Equals was found; replace it with null and skip it
    *value++ = '\0';
    res = do_setting(normalize_key(line), value);
  } else {
    // No equals, so it must be a command
    res = do_command(normalize_key(line), client);
  }
  // XXX report errors to client
}

void list_settings()
{
  for (Setting *s = SettingsList; s; s = s->link) {
    cout << s->getName() << " " << s->getStringValue() << '\n';
  }
}

void list_numbered_settings()
{
  for (map<const char*, Setting*>::iterator it = numberedSettings.begin();
       it != numberedSettings.end();
       it++) {
    cout << it->first << ": " << it->second->getStringValue() << '\n';
  }
}

#if TEST_SETTINGS
int main()
{
  list_settings();
}
#endif

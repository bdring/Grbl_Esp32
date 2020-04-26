#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <typeinfo>
#include <iterator>
#include "./defaults.h"
#include "./JSONencoder.h"
using namespace std;

#include "wmb_defs.h"


enum {
  BITFLAG_INVERT_ST_ENABLE = 1,
  BITFLAG_INVERT_LIMIT_PINS = 2,
  BITFLAG_INVERT_PROBE_PIN = 4,
  BITFLAG_REPORT_INCHES = 8,
  BITFLAG_SOFT_LIMIT_ENABLE = 16,
  BITFLAG_HARD_LIMIT_ENABLE = 32,
  BITFLAG_HOMING_ENABLE = 64,
  BITFLAG_LASER_MODE = 128,
};
enum {
  STATE_IDLE = 0,
  STATE_JOG,
};
#define MAXLINE 128
int client;

struct {
  int flags;
} settings;

struct {
  int state;
} sys;

#define MESSAGE_RESTORE_DEFAULTS "restoring defaults"
enum {
  SETTINGS_RESTORE_DEFAULTS,
  SETTINGS_RESTORE_PARAMETERS,
  SETTINGS_RESTORE_ALL,
  SETTINGS_RESTORE_WIFI_SETTINGS,
  SETTINGS_WIPE,
};

// SettingsList is a linked list of all settings,
// so common code can enumerate them.
class Setting;
Setting *SettingsList = NULL;

// This abstract class defines the generic interface that
// is used to set and get values for all settings independent
// of their underlying data type.  The values are always
// represented as human-readable strings.  This generic
// interface is used for managing settings via the user interface.
class Setting {
 private:
  const char* displayName;
  const char* webuiName;
 public:

  // Add each constructed setting to the linked list
  Setting *link;
  Setting(const char* name) {
    displayName = name;
    link = SettingsList;
    SettingsList = this;
  }
  const char* getName() { return displayName; };

  // load() reads the backing store to get the current
  // value of the setting.  This could be slow so it
  // should be done infrequently, typically once at startup.
  virtual void load() =0;

  // commit() puts the current value of the setting into
  // the backing store.
  //  virtual void commit() =0;

  virtual err_t setStringValue(string value) =0;
  virtual string getStringValue() =0;

  const char* getWebuiName() { return webuiName; };

  // The default implementation of addWebui() does nothing.
  // Derived classes may override it to do something.
  virtual void addWebui(JSONencoder *) { };
};

// This template class, derived from the generic abstract
// class, lets you store an individual setting as typed data.

template<typename T>
class TypedSetting : public Setting {
 public:
  TypedSetting<T>(const char *webName, const char* name, T defVal, T minVal, T maxVal, err_t (*function)(const char *) = NULL) :
  // TypedSetting<T>(const char *webName, const char* name, T defVal, T minVal, T maxVal) :
      Setting(name, webName),
      defaultValue(defVal),
      currentValue(defVal),
      minValue(minVal),
      maxValue(maxVal)
      //      checker(function)
  { }

  TypedSetting(const char* name, T defVal, T minVal, T maxVal, err_t (*function)(const char *) = NULL) :
      TypedSetting(NULL, name, defVal, minVal, maxVal, function)
  { }
  void load() {
    if (isfloat) {
      //p      storedValue = preferences.getFloat(displayName, defaultValue);
    } else {
      //p      storedValue = preferences.getInt(displayName, defaultValue);
    }  // Read value from backing store
    currentValue = storedValue;
  }
  void commit() {
    if (storedValue != currentValue) {
      if (storedValue == defaultValue) {
        //p        preferences.remove(displayName);
      } else {
        if (isfloat) {
          //p          preferences.putFloat(displayName, currentValue);
        } else {
          //p          preferences.putInt(displayName, currentValue);
        }
      }
    }
  }

  T get() {  return currentValue;  }
  err_t set(T value) { currentValue = value;  return STATUS_OK;  }
  err_t string_to_value(string s, T &value) {
    T convertedValue;
    try {
      convertedValue = stof(s);
    }
    catch (int e) {
      return STATUS_INVALID_VALUE;
    }
    if (convertedValue < minValue || convertedValue > maxValue) {
      return STATUS_NUMBER_RANGE;
    }
    value = convertedValue;
    return STATUS_OK;
  }
  string value_to_string (T value) {
    return to_string(value);
  }

  // Standard methods of the abstract class "Setting"
  err_t setStringValue(string s) {
    T newValue;
    err_t ret = string_to_value(s, newValue);
    if (ret) return ret;
    if (checker && (ret = checker())) return ret;
    currentValue = newValue;
    return ret;
  }
  const char* getStringValue() {
    return to_string(get()).c_str();
  }
  void addWebui(JSONencoder *j) {
    if (!isfloat && getWebuiName()) {
      j->begin_webui(getName(), getWebuiName(), "I", getStringValue(), minValue, maxValue);
      j->end_object();
    }
  }
};

typedef TypedSetting<float> FloatSetting;
typedef TypedSetting<int> IntSetting;

class AxisSettings {
 public:
  string name;
  FloatSetting steps_per_mm;
  FloatSetting max_rate;
  FloatSetting acceleration;
  FloatSetting max_travel;
  FloatSetting run_current;
  FloatSetting hold_current;
  IntSetting microsteps;
  IntSetting stallguard;
  AxisSettings(string axisName, float steps, float rate, float accel, float travel,
               float run, float hold, int usteps, int stall)
      : steps_per_mm((axisName+"_STEPS_PER_MM").c_str(), NULL, steps, 1.0, 50000.0,check_motor_settings)
      , max_rate((axisName+"_MAX_RATE").c_str(), NULL, rate, 1.0, 1000000.0, check_motor_settings)
    , acceleration((axisName+"_ACCELERATION").c_str(), NULL, accel, 1.0, 100000.0)
    , max_travel((axisName+"_MAX_TRAVEL").c_str(), NULL, travel, 1.0, 100000.0)   // Note! this values is entered as scaler but store negative
    , run_current((axisName+"_RUN_CURRENT").c_str(), NULL, run, 0.05, 20.0, settings_spi_driver_init)
    , hold_current((axisName+"_HOLD_CURRENT").c_str(), NULL, hold, 0.0, 100.0, settings_spi_driver_init)
    ,  microsteps((axisName+"_MICROSTEPS").c_str(), NULL, usteps, 1, 256, settings_spi_driver_init)
    , stallguard((axisName+"_STALLGUARD").c_str(), NULL, stall, 0, 100,  settings_spi_driver_init)
          { }
};

AxisSettings x_axis_settings = {
    "X",
    DEFAULT_X_STEPS_PER_MM,
    DEFAULT_X_MAX_RATE,
    DEFAULT_X_ACCELERATION,
    DEFAULT_X_MAX_TRAVEL,
    DEFAULT_X_CURRENT,
    DEFAULT_X_HOLD_CURRENT,
    DEFAULT_X_MICROSTEPS,
    DEFAULT_X_STALLGUARD
};

AxisSettings y_axis_settings = {
    "Y",
    DEFAULT_Y_STEPS_PER_MM,
    DEFAULT_Y_MAX_RATE,
    DEFAULT_Y_ACCELERATION,
    DEFAULT_Y_MAX_TRAVEL,
    DEFAULT_Y_CURRENT,
    DEFAULT_Y_HOLD_CURRENT,
    DEFAULT_Y_MICROSTEPS,
    DEFAULT_Y_STALLGUARD
};

AxisSettings z_axis_settings = {
    "Z",
    DEFAULT_Z_STEPS_PER_MM,
    DEFAULT_Z_MAX_RATE,
    DEFAULT_Z_ACCELERATION,
    DEFAULT_Z_MAX_TRAVEL,
    DEFAULT_Z_CURRENT,
    DEFAULT_Z_HOLD_CURRENT,
    DEFAULT_Z_MICROSTEPS,
    DEFAULT_Z_STALLGUARD
};

AxisSettings a_axis_settings = {
    "A",
    DEFAULT_A_STEPS_PER_MM,
    DEFAULT_A_MAX_RATE,
    DEFAULT_A_ACCELERATION,
    DEFAULT_A_MAX_TRAVEL,
    DEFAULT_A_CURRENT,
    DEFAULT_A_HOLD_CURRENT,
    DEFAULT_A_MICROSTEPS,
    DEFAULT_A_STALLGUARD
};

AxisSettings b_axis_settings = {
    "B",
    DEFAULT_B_STEPS_PER_MM,
    DEFAULT_B_MAX_RATE,
    DEFAULT_B_ACCELERATION,
    DEFAULT_B_MAX_TRAVEL,
    DEFAULT_B_CURRENT,
    DEFAULT_B_HOLD_CURRENT,
    DEFAULT_B_MICROSTEPS,
    DEFAULT_B_STALLGUARD
};

AxisSettings c_axis_settings = {
    "C",
    DEFAULT_C_STEPS_PER_MM,
    DEFAULT_C_MAX_RATE,
    DEFAULT_C_ACCELERATION,
    DEFAULT_C_MAX_TRAVEL,
    DEFAULT_C_CURRENT,
    DEFAULT_C_HOLD_CURRENT,
    DEFAULT_C_MICROSTEPS,
    DEFAULT_C_STALLGUARD
};

AxisSettings axis_settings[] = {
  x_axis_settings,
  y_axis_settings,
  z_axis_settings,
  a_axis_settings,
  b_axis_settings,
  c_axis_settings,
};

class StringSetting : public Setting {
 private:
  string currentValue;
  string storedValue;
  string defaultValue;

 public:
  StringSetting(const char* name, string defVal) :
    Setting(name),
      defaultValue(defVal),
      currentValue(defVal)
  { };
  void load() {
    //p    storedValue = preferences.getString(displayName, defaultValue);
    currentValue = storedValue;
  }
  void commit() {
    if (storedValue != currentValue) {
      if (storedValue == defaultValue) {
        //p        preferences.remove(displayName);
      } else {
        //p        preferences.putString(displayName, currentValue);
      }
    }
  }
  string get() { return currentValue;  }
  err_t set(string value) { currentValue = value;  return STATUS_OK;  }
  err_t string_to_value(string s, string &value) {
    value = s;
    return STATUS_OK;
  }
  string value_to_string (string value) { return value; }
  err_t setStringValue(string s) {
    currentValue = s;
    return STATUS_OK;
  }
  string getStringValue() { return currentValue; }
};
StringSetting startup_line_0("N0", "");
StringSetting startup_line_1("N1", "");
StringSetting build_info("I", "");

// Single-use class for the $RST= command
// There is no underlying stored value
class SettingsReset : public Setting {
 public:
  SettingsReset(const char* name) :
      Setting(name)
  { }
  void load() {
  }
  void commit() {
  }
  string get() {  return 0;  }
  err_t set(int value) {
    settings_restore(value);
    report_feedback_message(MESSAGE_RESTORE_DEFAULTS);
    mc_reset();  // Ensure settings are initialized correctly
    return STATUS_OK;
  }
  err_t string_to_value(string s, int &value) {
    switch (s[0]) {
      case '$': value = SETTINGS_RESTORE_DEFAULTS; break;
      case '#': value = SETTINGS_RESTORE_PARAMETERS; break;
      case '*': value = SETTINGS_RESTORE_ALL; break;
      case '@': value = SETTINGS_RESTORE_WIFI_SETTINGS; break;
      case '!': value = SETTINGS_WIPE; break;
      default: return STATUS_INVALID_VALUE;
    }
    return STATUS_OK;
  }
  err_t setStringValue(string s) {
    int value;
    err_t ret = string_to_value(s, value);
    return ret ? ret : set(value);
  }
  string value_to_string (int value) {
    switch(value) {
      case SETTINGS_RESTORE_DEFAULTS: return "$"; break;
      case SETTINGS_RESTORE_PARAMETERS: return "#"; break;
      case SETTINGS_RESTORE_ALL: return "*"; break;
      case SETTINGS_RESTORE_WIFI_SETTINGS: return "@"; break;
      case SETTINGS_WIPE: return "!"; break;
      default: return "";
    }
  }
  const char* getStringValue() { return ""; }
};

IntSetting pulse_microseconds("STEP_PULSE", DEFAULT_STEP_PULSE_MICROSECONDS, 3, 1000);
IntSetting stepper_idle_lock_time("STEPPER_IDLE_TIME", DEFAULT_STEPPER_IDLE_LOCK_TIME, 0, 255);

IntSetting step_invert_mask("STEP_INVERT_MASK", DEFAULT_STEPPING_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
IntSetting dir_invert_mask("DIR_INVERT_MASK", DEFAULT_DIRECTION_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
// XXX need to call st_generate_step_invert_masks()
IntSetting homing_dir_mask("HOMING_DIR_INVERT_MASK", DEFAULT_HOMING_DIR_MASK, 0, (1<<MAX_N_AXIS)-1);

class FlagSetting : public Setting {
 private:
  bool defaultValue;
  bool storedValue;
  bool currentValue;
  err_t (*checker)();
 public:
  FlagSetting(const char *webName, const char* name, bool defVal, err_t (*f)() = NULL) :
      Setting(name, webName),
      defaultValue(defVal),
      checker(f)
  { }
  FlagSetting(const char* name, bool defVal, err_t (*f)() = NULL) :
      FlagSetting(NULL, name, defVal, f)
  { }
  void load() {
    //p    storedValue = preferences.getBool(displayName, defaultValue);
    currentValue = storedValue;
  }
  void commit() {
    if (storedValue != currentValue) {
      if (storedValue == defaultValue) {
        //p        preferences.remove(displayName);
      } else {
        //p        preferences.putBool(displayName, currentValue);
      }
    }
  }
  bool get() {  return currentValue;  }
  err_t set(bool value) {
    currentValue = value;
    return STATUS_OK;
  }
  err_t string_to_value(string s, bool &value) {
    int n;
    try {
      n = stoi(s);
    }
    catch (int e) {
      return STATUS_INVALID_VALUE;
    }
    value = !!n;
    return STATUS_OK;
  }
  err_t setStringValue(string s) {
    bool value;
    err_t ret = string_to_value(s, value);
    if (checker && (ret = checker())) return ret;
    return ret ? ret : set(value);
  }
  const char* value_to_string (bool value) {
    return to_string(value).c_str();
  }
  string getStringValue() { return value_to_string(get()); }
};

FlagSetting step_enable_invert("STEP_ENABLE_INVERT", DEFAULT_INVERT_ST_ENABLE);
FlagSetting limit_invert("LIMIT_INVERT", DEFAULT_INVERT_LIMIT_PINS);
FlagSetting probe_invert("PROBE_INVERT", DEFAULT_INVERT_PROBE_PIN);
FlagSetting report_inches("REPORT_INCHES", DEFAULT_REPORT_INCHES);
err_t check_homing_enable() {
  return STATUS_OK;
}
FlagSetting soft_limits("SOFT_LIMITS", DEFAULT_SOFT_LIMIT_ENABLE, check_homing_enable);
// XXX need to check for HOMING_ENABLE
FlagSetting hard_limits("HARD_LIMITS", DEFAULT_HARD_LIMIT_ENABLE, limits_init);
// XXX need to call limits_init();
FlagSetting homing_enable("HOMING_ENABLE", DEFAULT_HOMING_ENABLE, also_soft_limit);
// XXX also need to clear, but not set, BITFLAG_SOFT_LIMIT_ENABLE
FlagSetting laser_mode("LASER_MODE", DEFAULT_LASER_MODE, my_spindle_init);
// XXX also need to call my_spindle->init();

IntSetting status_mask("STATUS_MASK", DEFAULT_STATUS_REPORT_MASK, 0, 2);
FloatSetting junction_deviation("JUNCTION_DEVIATION", DEFAULT_JUNCTION_DEVIATION, 0, 10);
FloatSetting arc_tolerance("ARC_TOLERANCE", DEFAULT_ARC_TOLERANCE, 0, 1);

FloatSetting homing_feed_rate("HOMING_FEED", DEFAULT_HOMING_FEED_RATE, 0, 10000);
FloatSetting homing_seek_rate("HOMING_SEEK", DEFAULT_HOMING_SEEK_RATE, 0, 10000);
FloatSetting homing_debounce("HOMING_DEBOUNCE", DEFAULT_HOMING_DEBOUNCE_DELAY, 0, 10000);
FloatSetting homing_pulloff("HOMING_PULLOFF", DEFAULT_HOMING_PULLOFF, 0, 10000);
FloatSetting spindle_pwm_freq("SPINDLE_PWM_FREQ", DEFAULT_SPINDLE_FREQ, 0, 10000);
FloatSetting rpm_max("RPM_MAX", DEFAULT_SPINDLE_RPM_MAX, 0, 10000);
FloatSetting rpm_min("RPM_MIN", DEFAULT_SPINDLE_RPM_MIN, 0, 10000);

FloatSetting spindle_pwm_off_value("SPINDLE_PWM_OFF_VALUE", DEFAULT_SPINDLE_OFF_VALUE, 0, 10000);
FloatSetting spindle_pwm_min_value("SPINDLE_PWM_MIN_VALUE", DEFAULT_SPINDLE_MIN_VALUE, 0, 10000);
FloatSetting spindle_pwm_max_value("SPINDLE_PWM_MAX_VALUE", DEFAULT_SPINDLE_MAX_VALUE, 0, 10000);
IntSetting spindle_pwm_bit_precision("SPINDLE_PWM_BIT_PRECISION", DEFAULT_SPINDLE_BIT_PRECISION, 0, 10000);

StringSetting spindle_type("SPINDLE_TYPE", DEFAULT_SPINDLE_TYPE);

// The following table maps numbered settings to their settings
// objects, for backwards compatibility.  It is used if the
// line is of the form "$key=value".  If a key match is found,
// the object's setValueString method is invoked with the value
// as argument.  If no match is found, the list of named settings
// is searched, with the same behavior on a match.

// These are compatibily aliases for Classic GRBL
std::map<const char*, Setting*> numberedSettings = {
    { "0", &pulse_microseconds },
    { "1", &stepper_idle_lock_time },
    { "2", &step_invert_mask },
    { "3", &dir_invert_mask },
    { "4", &step_enable_invert },
    { "5", &limit_invert },
    { "6", &probe_invert },
    { "10", &status_mask },
    { "11", &junction_deviation },
    { "12", &arc_tolerance },
    { "13", &report_inches },
    { "20", &soft_limits },
    { "21", &hard_limits },
    { "22", &homing_enable },
    { "23", &homing_dir_mask },
    { "24", &homing_feed_rate },
    { "25", &homing_seek_rate },
    { "26", &homing_debounce },
    { "27", &homing_pulloff },
    { "30", &rpm_max },
    { "31", &rpm_min },
    { "32", &laser_mode },
    { "100", &x_axis_settings.steps_per_mm },
    { "101", &y_axis_settings.steps_per_mm },
    { "102", &z_axis_settings.steps_per_mm },
    { "110", &x_axis_settings.max_rate },
    { "111", &y_axis_settings.max_rate },
    { "112", &z_axis_settings.max_rate },
    { "120", &x_axis_settings.acceleration },
    { "121", &y_axis_settings.acceleration },
    { "122", &z_axis_settings.acceleration },
    { "130", &x_axis_settings.max_travel },
    { "131", &y_axis_settings.max_travel },
    { "132", &z_axis_settings.max_travel },
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
err_t do_setting(const char *key, char *value, uint8_t client) {

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
  char *value = strchr(line, '=');

  if (value) {
    // Equals was found; replace it with null and skip it
    *value++ = '\0';
    do_setting(normalize_key(line), value, client);
  } else {
    // No equals, so it must be a command
    do_command(normalize_key(line), client);
  }
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

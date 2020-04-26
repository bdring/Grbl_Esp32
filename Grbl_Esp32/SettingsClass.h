#pragma once
#include "SettingsBaseClass.h"

// This template class, derived from the generic abstract
// class, lets you store an individual setting as typed data.

template<typename T>
class TypedSetting : public Setting {
 private:
  T defaultValue;
  T currentValue;
  T storedValue;
  T minValue;
  T maxValue;
  const bool isfloat = typeid(T) == typeid(float);
  err_t (*checker)();

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
  err_t string_to_value(const char* s, T &value) {
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
  const char* value_to_string (T value) {
    return to_string(value);
  }

  // Standard methods of the abstract class "Setting"
  err_t setStringValue(const char* s) {
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

class StringSetting : public Setting {
 private:
  const char* currentValue;
  const char* storedValue;
  const char* defaultValue;
  int minLength;
  int maxLength;
  err_t (*checker)(const char *);

 public:
  StringSetting(const char *webName, const char* name, const char* defVal, int min, int max, err_t (*function)(const char *)) :
    Setting(name, webName),
    defaultValue(defVal),
    currentValue(defVal),
    minLength(min),
    maxLength(max),
    checker(function)
  { };
  StringSetting(const char* name, const char* defVal, err_t (*function)(const char *) = NULL) :
      StringSetting(NULL, name, defVal, 0, 0, function)
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
  const char* get() { return currentValue;  }
  err_t set(const char* value) { currentValue = value;  return STATUS_OK;  }
  err_t string_to_value(const char* s, const char* &value) {
    if (checker && checker(value)) {
      return STATUS_INVALID_VALUE;
    }
    value = s;
    return STATUS_OK;
  }
  const char* value_to_string (const char* value) { return value; }
  err_t setStringValue(const char* s) {
    if (checker && checker(s)) {
      return STATUS_INVALID_VALUE;
    }
    currentValue = s;
    return STATUS_OK;
  }
  const char* getStringValue() { return currentValue; }
  void addWebui(JSONencoder *j) {
    if (!getWebuiName()) {
      return;
    }
    j->begin_webui(
        getName(), getWebuiName(), "S", getStringValue(), minLength, maxLength);
    j->end_object();
  }
};

class AxisSettings {
 public:
  const char* name;
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

// Single-use class for the $RST= command
// There is no underlying stored value
class SettingsReset : public Setting {
 public:
  SettingsReset(const char* name) :
      Setting(name, NULL)
  { }
  void load() {
  }
  void commit() {
  }
  const char* get() {  return 0;  }
  err_t set(int value) {
    settings_restore(value);
    report_feedback_message(MESSAGE_RESTORE_DEFAULTS);
    mc_reset();  // Ensure settings are initialized correctly
    return STATUS_OK;
  }
  err_t string_to_value(const char* s, int &value) {
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
  err_t setStringValue(const char* s) {
    int value;
    err_t ret = string_to_value(s, value);
    return ret ? ret : set(value);
  }
  const char* value_to_string (int value) {
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
  err_t string_to_value(const char* s, bool &value) {
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
  err_t setStringValue(const char* s) {
    bool value;
    err_t ret = string_to_value(s, value);
    if (checker && (ret = checker())) return ret;
    return ret ? ret : set(value);
  }
  const char* value_to_string (bool value) {
    return to_string(value).c_str();
  }
  const char* getStringValue() { return value_to_string(get()); }
};

class EnumSetting : public Setting {
 private:
  int8_t defaultValue;
  int8_t storedValue;
  int8_t currentValue;
  std::map<const char *, int8_t>options;
 public:
  EnumSetting(const char *webName, const char* name, int8_t defVal, std::map<const char *, int8_t>opts) :
      Setting(name, webName),
      defaultValue(defVal),
      options(opts)
  { }
  EnumSetting(const char* name, int8_t defVal, std::map<const char *, int8_t>opts) :
      EnumSetting(NULL, name, defVal, opts)
  { }
  void load() {
    //p    storedValue = preferences.getBool(displayName, defaultValue);
    int8_t storedValue;
    currentValue = nvs_get_i8(nvs_handle, getName(), &storedValue) ? defaultValue : storedValue;
  }
  void commit() {
    if (storedValue != currentValue) {
      if (storedValue == defaultValue) {
        nvs_erase_key(nvs_handle, getName());
      } else {
        nvs_set_i8(nvs_handle, getName(), currentValue);
        // do_commit() or something
      }
    }
  }
  int8_t get() {  return currentValue;  }
  err_t set(int8_t value) {
    currentValue = value;
    return STATUS_OK;
  }
  err_t string_to_value(const char* s, int8_t &value) {
    map<const char *, int8_t>::iterator it = options.find(s);
    if (it == options.end())
      return STATUS_INVALID_VALUE;
    value = it->second;
    return STATUS_OK;
  }
  err_t setStringValue(const char* s) {
    map<const char *, int8_t>::iterator it = options.find(s);
    return (it == options.end()) ? STATUS_INVALID_VALUE : set(it->second);
  }
  const char* value_to_string (int8_t value) {
    for (map<const char*, int8_t>::iterator it = options.begin();
         it != options.end();
         it++) {
      if (it->second == value) {
        return it->first;
      }
      return "???";
    }
  }
  const char* getStringValue() { return value_to_string(get()); }
  void addWebui(JSONencoder *j) {
    if (!getWebuiName()) {
      return;
    }
    j->begin_webui(getName(), getWebuiName(), "B", getStringValue());
    j->begin_array("O");
    for (map<const char*, int8_t>::iterator it = options.begin();
         it != options.end();
         it++) {
        j->begin_object();
        j->member(it->first, it->second);
        j->end_object();
      }
    j->end_array();
    j->end_object();
  }
};

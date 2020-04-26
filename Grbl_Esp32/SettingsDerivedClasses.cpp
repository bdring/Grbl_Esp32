#include "SettingsDerivedClasses.h"
#include "JSONencoder.h"
#include <limits>
#include <string>
#include <cstring>
#include "wmb_defs.h"

int nvs_handle;

IntSetting::IntSetting(const char *webName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, err_t (*checker)(const char *) = NULL)
      : Setting(webName, name, checker)
      , defaultValue(defVal)
      , currentValue(defVal)
      , minValue(minVal)
      , maxValue(maxVal)
  { }

void IntSetting::load() {
    esp_err_t err = nvs_get_i32(nvs_handle, getName(), &storedValue);
    if (err) {
        storedValue = std::numeric_limits<int32_t>::min();
        currentValue = defaultValue;
    } else {
        currentValue = storedValue;
    }
    // Read value from backing store
    //p  int32_t storedValue = preferences.getInt(displayName, defaultValue);
    currentValue = storedValue;
}

void IntSetting::commit() {
    if (storedValue != currentValue) {
        if (currentValue == defaultValue) {
            nvs_erase_key(nvs_handle, getName());
        } else {
            nvs_set_i32(nvs_handle, getName(), currentValue);
            storedValue = currentValue;
        }
    }
}

err_t IntSetting::setStringValue(const char* s) {
    if (err_t err = check(s)) {
      return err;
    }
    int32_t convertedValue;
    try {
      convertedValue = stof(s);
    }
    catch (int e) {
      return STATUS_INVALID_VALUE;
    }
    if (convertedValue < minValue || convertedValue > maxValue) {
      return STATUS_NUMBER_RANGE;
    }
    currentValue = convertedValue;
    return STATUS_OK;
  }

const char* IntSetting::getStringValue() {
    return to_string(get()).c_str();
}

void IntSetting::addWebui(JSONencoder *j) {
    if (getWebuiName()) {
        j->begin_webui(getName(), getWebuiName(), "I", getStringValue(), minValue, maxValue);
        j->end_object();
    }
}

FloatSetting::FloatSetting(const char *webName, const char* name, float defVal, float minVal, float maxVal, err_t (*checker)(const char *) = NULL)
    : Setting(webName, name, checker)
    , defaultValue(defVal)
    , currentValue(defVal)
    , minValue(minVal)
    , maxValue(maxVal)
{ }

void FloatSetting::load() {
    union {
        int32_t ival;
        float   fval;
    } v;
    currentValue = nvs_get_i32(nvs_handle, getName(), &v.ival) ? defaultValue : v.fval;
}

void FloatSetting::commit() {
    if (storedValue != currentValue) {
        if (currentValue == defaultValue) {
            nvs_erase_key(nvs_handle, getName());
        } else {
            union {
                int32_t ival;
                float   fval;
            } v;
            v.fval = currentValue;
            nvs_set_i32(nvs_handle, getName(), v.ival);
            storedValue = currentValue;
        }
    }
}

err_t FloatSetting::setStringValue(const char* s) {
    if (err_t err = check(s)) {
      return err;
    }

    float convertedValue;
    try {
        convertedValue = stof(s);
    }
    catch (int e) {
        return STATUS_INVALID_VALUE;
    }
    if (convertedValue < minValue || convertedValue > maxValue) {
        return STATUS_NUMBER_RANGE;
    }
    currentValue = convertedValue;
    return STATUS_OK;
}

const char* FloatSetting::getStringValue() {
    return to_string(currentValue).c_str();
}

StringSetting::StringSetting(const char *webName, const char* name, const char* defVal, int min, int max, err_t (*checker)(const char *))
    : Setting(webName, name, checker)
    , defaultValue(defVal)
    , currentValue(defVal)
    , minLength(min)
    , maxLength(max)
{ };

void StringSetting::load() {
    size_t len = 0;
    esp_err_t err = nvs_get_str(nvs_handle, getName(), NULL, &len);
    if(err) {
        storedValue = defaultValue;
        currentValue = defaultValue;
        return;
    }
    char buf[len];
    err = nvs_get_str(nvs_handle, getName(), buf, &len);
    if (err) {
        storedValue = defaultValue;
        currentValue = defaultValue;
        return;
    }
    storedValue = std::string(buf);
    currentValue = storedValue;
}

void StringSetting::commit() {
    if (storedValue != currentValue) {
        if (currentValue == defaultValue) {
            nvs_erase_key(nvs_handle, getName());
            storedValue = defaultValue;
        } else {
            nvs_set_str(nvs_handle, getName(), currentValue.c_str());
            storedValue = currentValue;
        }
    }
}

err_t StringSetting::setStringValue(const char* s) {
    if (minLength && maxLength && (std::strlen(s) < minLength || std::strlen(s) > maxLength)) {
        return STATUS_INVALID_VALUE;
    }
    if (err_t err = check(s)) {
        return err;
    }
    currentValue = s;
    return STATUS_OK;
}

const char* StringSetting::getStringValue() { return currentValue.c_str(); }

void StringSetting::addWebui(JSONencoder *j) {
    if (!getWebuiName()) {
        return;
    }
    j->begin_webui(
        getName(), getWebuiName(), "S", getStringValue(), minLength, maxLength);
    j->end_object();
}

EnumSetting::EnumSetting(const char *webName, const char* name, int8_t defVal, std::map<const char *, int8_t>opts)
    // No checker function because enumerations have an exact set of value
    : Setting(webName, name, NULL)
    , defaultValue(defVal)
    , options(opts)
{ }

void EnumSetting::load() {
    esp_err_t err = nvs_get_i8(nvs_handle, getName(), &storedValue);
    if (err) {
        storedValue = -1;
        currentValue = defaultValue;
    } else {
        currentValue = storedValue;
    }
}
void EnumSetting::commit() {
    if (storedValue != currentValue) {
        if (storedValue == defaultValue) {
            nvs_erase_key(nvs_handle, getName());
        } else {
            nvs_set_i8(nvs_handle, getName(), currentValue);
            storedValue = currentValue;
        }
    }
}
err_t EnumSetting::setStringValue(const char* s) {
    map<const char *, int8_t>::iterator it = options.find(s);
    if (it == options.end()) {
        return STATUS_INVALID_VALUE;
    }
    currentValue = it->second;
    return STATUS_OK;
}

const char* EnumSetting::getStringValue() {
    for (map<const char*, int8_t>::iterator it = options.begin();
         it != options.end();
         it++) {
        if (it->second == currentValue) {
            return it->first;
        }
        return "???";
    }
}

void EnumSetting::addWebui(JSONencoder *j) {
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

FlagSetting::FlagSetting(const char *webName, const char* name, bool defVal, err_t (*checker)(const char *) = NULL) :
    Setting(webName, name, checker),
    defaultValue(defVal)
{ }

void FlagSetting::load() {
    int8_t storedValue;
    esp_err_t err = nvs_get_i8(nvs_handle, getName(), &storedValue);
    if (err) {
        storedValue = -1;
        currentValue = defaultValue;
    } else {
        currentValue = !!storedValue;
    }
}
void FlagSetting::commit() {
    // storedValue is -1, 0, or 1
    // currentValue is 0 or 1
    if (storedValue != (int8_t)currentValue) {
        if (currentValue == defaultValue) {
            nvs_erase_key(nvs_handle, getName());
        } else {
            nvs_set_i8(nvs_handle, getName(), currentValue);
            storedValue = currentValue;
        }
    }
}
err_t FlagSetting::setStringValue(const char* s) {
    if (err_t err = check(s)) {
        return err;
    }
    int n;
    try {
        n = stoi(s);
    }
    catch (int e) {
        return STATUS_INVALID_VALUE;
    }
    currentValue = !!n;
    return STATUS_OK;
}
const char* FlagSetting::getStringValue() {
    return to_string(currentValue).c_str();
}

AxisSettings::AxisSettings(string axisName, float steps, float rate, float accel, float travel,
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

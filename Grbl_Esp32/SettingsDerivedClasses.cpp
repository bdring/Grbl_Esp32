#include "grbl.h"

#include "SettingsDerivedClasses.h"
#include "JSONencoder.h"
#include <string>
#include <map>
#include "nvs.h"

Setting::Setting(const char* webuiName, group_t group, const char* grblName, const char* displayName, bool (*checker)(const char *))
        : _webuiName(webuiName)
        , _group(group)
        , _axis(NO_AXIS)
        , _grblName(grblName)
        , _displayName(displayName)
        , _checker(checker)
{
    link = SettingsList;
    SettingsList = this;
}

nvs_handle _handle = 0;

IntSetting::IntSetting(const char *webName, group_t group, const char* grblName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(const char *) = NULL)
      : Setting(webName, group, grblName, name, checker)
      , _defaultValue(defVal)
      , _currentValue(defVal)
      , _minValue(minVal)
      , _maxValue(maxVal)
{ }

void IntSetting::load() {
    esp_err_t err = nvs_get_i32(_handle, getName(), &_storedValue);
    if (err) {
        _storedValue = std::numeric_limits<int32_t>::min();
        _currentValue = _defaultValue;
    } else {
        _currentValue = _storedValue;
    }
}

void IntSetting::commit() {
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i32(_handle, getName(), _currentValue);
            _storedValue = _currentValue;
        }
    }
}

err_t IntSetting::setStringValue(const char* s) {
    if (check(s)) {
        return STATUS_INVALID_VALUE;
    }
    int32_t convertedValue;
    try {
        convertedValue = std::stoi(s);
    }
    catch (int e) {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    if (convertedValue < _minValue || convertedValue > _maxValue) {
        return STATUS_NUMBER_RANGE;
    }
    _currentValue = convertedValue;
    return STATUS_OK;
}

const char* IntSetting::getStringValue() {
    return std::to_string(get()).c_str();
}

void IntSetting::addWebui(JSONencoder *j) {
    if (getWebuiName()) {
        j->begin_webui(getName(), getWebuiName(), "I", getStringValue(), _minValue, _maxValue);
        j->end_object();
    }
}

FloatSetting::FloatSetting(const char *webName, group_t group, const char* grblName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(const char *) = NULL)
    : Setting(webName, group, grblName, name, checker)
    , _defaultValue(defVal)
    , _currentValue(defVal)
    , _minValue(minVal)
    , _maxValue(maxVal)
{ }

void FloatSetting::load() {
    union {
        int32_t ival;
        float   fval;
    } v;
    _currentValue = nvs_get_i32(_handle, getName(), &v.ival) ? _defaultValue : v.fval;
}

void FloatSetting::commit() {
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            union {
                int32_t ival;
                float   fval;
            } v;
            v.fval = _currentValue;
            nvs_set_i32(_handle, getName(), v.ival);
            _storedValue = _currentValue;
        }
    }
}

err_t FloatSetting::setStringValue(const char* s) {
    if (check(s)) {
      return STATUS_INVALID_VALUE;
    }

    float convertedValue;
    try {
        convertedValue = std::stof(s);
    }
    catch (int e) {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    if (convertedValue < _minValue || convertedValue > _maxValue) {
        return STATUS_NUMBER_RANGE;
    }
    _currentValue = convertedValue;
    return STATUS_OK;
}

const char* FloatSetting::getStringValue() {
    static char strval[32];
    (void)sprintf(strval, "%.3f", get());
#if 0
    // With the goal of representing both large and small floating point
    // numbers compactly while showing clearly that the are floating point,
    // remove trailing zeros leaving at least one post-decimal digit.
    // The loop is guaranteed to terminate because the string contains
    // a decimal point which is not a '0'.
    for (char *p = strval + strlen(strval) - 1; *p == '0'; --p) {
        if (*(p-1) != '.' && *(p-1) != ',') {
            *p = '\0';
        }
    }
#endif
    return strval;
}

StringSetting::StringSetting(const char *webName, group_t group, const char* grblName, const char* name, const char* defVal, int min, int max, bool (*checker)(const char *))
    : Setting(webName, group, grblName, name, checker)
{
    _defaultValue = defVal;
    _currentValue = defVal;
    _minLength = min;
    _maxLength = max;
 };

void StringSetting::load() {
    size_t len = 0;
    esp_err_t err = nvs_get_str(_handle, getName(), NULL, &len);
    if(err) {
        _storedValue = _defaultValue;
        _currentValue = _defaultValue;
        return;
    }
    char buf[len];
    err = nvs_get_str(_handle, getName(), buf, &len);
    if (err) {
        _storedValue = _defaultValue;
        _currentValue = _defaultValue;
        return;
    }
    _storedValue = std::string(buf);
    _currentValue = _storedValue;
}

void StringSetting::commit() {
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
            _storedValue = _defaultValue;
        } else {
            nvs_set_str(_handle, getName(), _currentValue.c_str());
            _storedValue = _currentValue;
        }
    }
}

err_t StringSetting::setStringValue(const char* s) {
    if (_minLength && _maxLength && (strlen(s) < _minLength || strlen(s) > _maxLength)) {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    if (check(s)) {
        return STATUS_INVALID_VALUE;
    }
    _currentValue = s;
    return STATUS_OK;
}

const char* StringSetting::getStringValue() {
    // If the string is a password do not display it 
    if (_checker && _checker == WiFiConfig::isPasswordValid) {
        return "******";
    }
     return _currentValue.c_str();
}

void StringSetting::addWebui(JSONencoder *j) {
    if (!getWebuiName()) {
        return;
    }
    j->begin_webui(
        getName(), getWebuiName(), "S", getStringValue(), _minLength, _maxLength);
    j->end_object();
}

EnumSetting::EnumSetting(const char *webName, group_t group, const char* grblName, const char* name, int8_t defVal, std::map<const char *, int8_t>opts)
    // No checker function because enumerations have an exact set of value
    : Setting(webName, group, grblName, name, NULL)
    , _defaultValue(defVal)
    , _options(opts)
{ }

void EnumSetting::load() {
    esp_err_t err = nvs_get_i8(_handle, getName(), &_storedValue);
    if (err) {
        _storedValue = -1;
        _currentValue = _defaultValue;
    } else {
        _currentValue = _storedValue;
    }
}
void EnumSetting::commit() {
    if (_storedValue != _currentValue) {
        if (_storedValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i8(_handle, getName(), _currentValue);
            _storedValue = _currentValue;
        }
    }
}
err_t EnumSetting::setStringValue(const char* s) {
    std::map<const char *, int8_t>::iterator it = _options.find(s);
    if (it == _options.end()) {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    _currentValue = it->second;
    return STATUS_OK;
}

const char* EnumSetting::getStringValue() {
    for (std::map<const char*, int8_t>::iterator it = _options.begin();
         it != _options.end();
         it++) {
        if (it->second == _currentValue) {
            return it->first;
        }
    }
    return "???";
}

void EnumSetting::addWebui(JSONencoder *j) {
    if (!getWebuiName()) {
      return;
    }
    j->begin_webui(getName(), getWebuiName(), "B", getStringValue());
    j->begin_array("O");
    for (std::map<const char*, int8_t>::iterator it = _options.begin();
         it != _options.end();
         it++) {
        j->begin_object();
        j->member(it->first, it->second);
        j->end_object();
      }
    j->end_array();
    j->end_object();
}

FlagSetting::FlagSetting(const char *webName, group_t group, const char * grblName, const char* name, bool defVal, bool (*checker)(const char *) = NULL) :
    Setting(webName, group, grblName, name, checker),
    _defaultValue(defVal)
{ }

void FlagSetting::load() {
    esp_err_t err = nvs_get_i8(_handle, getName(), &_storedValue);
    if (err) {
        _storedValue = -1;  // Neither well-formed false (0) nor true (1)
        _currentValue = _defaultValue;
    } else {
        _currentValue = !!_storedValue;
    }
}
void FlagSetting::commit() {
    // _storedValue is -1, 0, or 1
    // _currentValue is 0 or 1
    if (_storedValue != (int8_t)_currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i8(_handle, getName(), _currentValue);
            _storedValue = _currentValue;
        }
    }
}
err_t FlagSetting::setStringValue(const char* s) {
    _currentValue = (strcasecmp(s, "on") == 0)
    || (strcasecmp(s, "true") == 0)
    || (strcasecmp(s, "enabled") == 0)
    || (strcasecmp(s, "yes") == 0)
    || (strcasecmp(s, "1") == 0);
     return STATUS_OK;
}
const char* FlagSetting::getStringValue() {
    return get() ? "On" : "Off";
}

#include <WiFi.h>

IPaddrSetting::IPaddrSetting(const char *webName, group_t group, const char * grblName, const char* name, uint32_t defVal, bool (*checker)(const char *) = NULL)
      : Setting(webName, group, grblName, name, checker) // There are no GRBL IP settings.
      , _defaultValue(defVal)
      , _currentValue(defVal)
{ }

IPaddrSetting::IPaddrSetting(const char *webName, group_t group, const char * grblName, const char* name, const char *defVal, bool (*checker)(const char *) = NULL)
      : Setting(webName, group, grblName, name, checker)
{
    IPAddress ipaddr;
    if (ipaddr.fromString(defVal)) {
        _defaultValue = ipaddr;
        _currentValue = _defaultValue;
    } else {
        throw std::runtime_error("Bad IPaddr default");
    }
}

void IPaddrSetting::load() {
    esp_err_t err = nvs_get_i32(_handle, getName(), (int32_t *)&_storedValue);
    if (err) {
        _storedValue = 0x000000ff;  // Unreasonable value for any IP thing
        _currentValue = _defaultValue;
    } else {
        _currentValue = _storedValue;
    }
    _currentValue = _storedValue;
}

void IPaddrSetting::commit() {
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i32(_handle, getName(), (int32_t)_currentValue);
            _storedValue = _currentValue;
        }
    }
}

err_t IPaddrSetting::setStringValue(const char* s) {
    if (check(s)) {
        return STATUS_INVALID_VALUE;
    }
    uint32_t convertedValue;
    IPAddress ipaddr;
    if (ipaddr.fromString(s)) {
        _currentValue = ipaddr;
        return STATUS_OK;
    }
    return STATUS_INVALID_VALUE;
}

const char* IPaddrSetting::getStringValue() {
    static String s;
    IPAddress ipaddr(get());
    s = ipaddr.toString();
    return s.c_str();
}

void IPaddrSetting::addWebui(JSONencoder *j) {
    if (getWebuiName()) {
        j->begin_webui(getName(), getWebuiName(), "A", getStringValue());
        j->end_object();
    }
}

#if 0
// Construct e.g. X_MAX_RATE from axisName "X" and tail "_MAX_RATE"
// in dynamically allocated memory that will not be freed.
static const char *makename(const char *axisName, const char *tail) {
    char *retval = (char *)malloc(strlen(axisName) + strlen(tail) + 1);
    strcpy(retval, axisName);
    return strcat(retval, tail);
}

// Generates a string like "122" from axisNum 2 and base 120
static const char *makeGrblName(int axisNum, int base) {
    // To omit A,B,C axes:
    // if (axisNum > 2) return NULL;
    char buf[4];
    snprintf(buf, 4, "%d", axisNum + base);
    char *retval = (char *)malloc(strlen(buf));
    return strcpy(retval, buf);
}

AxisSettings::AxisSettings(int axisNum, const char *axisName, float steps, float rate, float accel, float travel,
                           float run, float hold, int usteps, int stall)
{
    name = axisName;
    // Creating these in reverse order makes them display in the order people expect
    stallguard   = new IntSetting(EXTENDED, makeGrblName(axisNum, 170), makename(axisName, "StallGuard"), stall, 0, 100);
    stallguard->setAxis(axisNum);
    microsteps   = new IntSetting(EXTENDED, makeGrblName(axisNum, 160), makename(axisName, "Microsteps"), usteps, 1, 256);
    microsteps->setAxis(axisNum);
    hold_current = new FloatSetting(EXTENDED, makeGrblName(axisNum, 150), makename(axisName, "HoldCurrent"), hold, 0.0, 100.0);
    hold_current->setAxis(axisNum);
    run_current  = new FloatSetting(EXTENDED, makeGrblName(axisNum, 140), makename(axisName, "RunCurrent"), run, 0.05, 20.0);
    run_current->setAxis(axisNum);
    max_travel   = new FloatSetting(GRBL, makeGrblName(axisNum, 130), makename(axisName, "MaxTravel"), travel, 1.0, 100000.0);   // Note! this values is entered as scaler but store negative
    max_travel->setAxis(axisNum);
    acceleration = new FloatSetting(GRBL, makeGrblName(axisNum, 120), makename(axisName, "Acceleration"), accel, 1.0, 100000.0);
    acceleration->setAxis(axisNum);
    max_rate     = new FloatSetting(GRBL, makeGrblName(axisNum, 110), makename(axisName, "MaxRate"), rate, 1.0, 1000000.0);
    max_rate->setAxis(axisNum);
    steps_per_mm = new FloatSetting(GRBL, makeGrblName(axisNum, 100), makename(axisName, "StepsPerMm"), steps, 1.0, 50000.0);
    steps_per_mm->setAxis(axisNum);
 }
#endif
 AxisSettings::AxisSettings(const char *axisName) :
    name(axisName)
{}

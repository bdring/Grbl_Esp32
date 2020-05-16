#include "grbl.h"

#include "SettingsClass.h"
#include "JSONencoder.h"
#include <string>
#include <map>
#include "nvs.h"

Command::Command(const char* webuiName, group_t group, const char* grblName, const char* displayName)
    : _webuiName(webuiName)
    , _group(group)
    , _grblName(grblName)
    , _displayName(displayName)
{
    link = CommandsList;
    CommandsList = this;
}

Setting::Setting(const char* webuiName, group_t group, const char* grblName, const char* displayName, bool (*checker)(const char *))
    : Command(webuiName, group, grblName, displayName)
    , _checker(checker)
{ }

err_t Setting::action(char* value, ESPResponseStream* out) {
    if (value && *value) {
        return setStringValue(value);
    } else {
        if (out) {
            out->println(getStringValue());
        }
        return STATUS_OK;
    }
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

void IntSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, getName());
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
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i32(_handle, getName(), _currentValue);
            _storedValue = _currentValue;
        }
    }
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

void FloatSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, getName());
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

void StringSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, getName());
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
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
            _storedValue = _defaultValue;
        } else {
            nvs_set_str(_handle, getName(), _currentValue.c_str());
            _storedValue = _currentValue;
        }
    }
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

typedef std::map<const char *, int8_t, cmp_str> enum_opt_t;

EnumSetting::EnumSetting(const char *webName, group_t group, const char* grblName, const char* name, int8_t defVal, enum_opt_t *opts)
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

void EnumSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, getName());
    }
}

err_t EnumSetting::setStringValue(const char* s) {
    enum_opt_t::iterator it = _options->find(s);
    if (it == _options->end()) {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    _currentValue = it->second;
    if (_storedValue != _currentValue) {
        if (_storedValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i8(_handle, getName(), _currentValue);
            _storedValue = _currentValue;
        }
    }
    return STATUS_OK;
}

const char* EnumSetting::getStringValue() {
    for (enum_opt_t::iterator it = _options->begin();
         it != _options->end();
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
    for (enum_opt_t::iterator it = _options->begin();
         it != _options->end();
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
void FlagSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, getName());
    }
}

err_t FlagSetting::setStringValue(const char* s) {
    _currentValue = (strcasecmp(s, "on") == 0)
    || (strcasecmp(s, "true") == 0)
    || (strcasecmp(s, "enabled") == 0)
    || (strcasecmp(s, "yes") == 0)
    || (strcasecmp(s, "1") == 0);
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

void IPaddrSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, getName());
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
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, getName());
        } else {
            nvs_set_i32(_handle, getName(), (int32_t)_currentValue);
            _storedValue = _currentValue;
        }
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

 AxisSettings::AxisSettings(const char *axisName) :
    name(axisName)
{}

err_t GrblCommand::action(char* value, ESPResponseStream* out) {
  return _action((const char*)value, out->client());
};

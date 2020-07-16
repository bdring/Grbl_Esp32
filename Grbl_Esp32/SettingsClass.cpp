#include "grbl.h"
#include "JSONencoder.h"
#include <map>
#include "nvs.h"

Word::Word(type_t type, permissions_t permissions, const char* description, const char* grblName, const char* fullName)
    : _description(description)
    , _grblName(grblName)
    , _fullName(fullName)
    , _type(type)
    , _permissions(permissions)
{}

Command* Command::List = NULL;

Command::Command(const char* description, type_t type, permissions_t permissions, const char* grblName, const char* fullName)
    : Word(type, permissions, description, grblName, fullName)
{
    link = List;
    List = this;
}

Setting* Setting::List = NULL;

Setting::Setting(const char* description, type_t type, permissions_t permissions, const char* grblName, const char* fullName, bool (*checker)(char *))
    : Word(type, permissions, description, grblName, fullName)
    , _checker(checker)
{
    link = List;
    List = this;

    // NVS keys are limited to 15 characters, so if the setting name is longer
    // than that, we derive a 15-character name from a hash function
    size_t len = strlen(fullName);
    if (len <= 15) {
        _keyName = _fullName;
    } else {
        // This is Donald Knuth's hash function from Vol 3, chapter 6.4
        char *hashName = (char *)malloc(16);
        uint32_t hash = len;
        for (const char *s = fullName; *s; s++) {
            hash = ((hash << 5) ^ (hash >> 27)) ^ (*s);
        }
        sprintf(hashName, "%.7s%08x", fullName, hash);
        _keyName = hashName;
    }
}

err_t Setting::check(char *s) {
    if (sys.state != STATE_IDLE && !(sys.state & STATE_ALARM)) {
        return STATUS_IDLE_ERROR;
    }
    if (!_checker) {
        return STATUS_OK;
    }
    return _checker(s) ? STATUS_OK : STATUS_INVALID_VALUE;
}

nvs_handle Setting::_handle = 0;

void Setting::init() {
    if (!_handle) {
        if (esp_err_t err = nvs_open("Grbl_ESP32", NVS_READWRITE, &_handle)) {
            grbl_sendf(CLIENT_SERIAL, "nvs_open failed with error %d\r\n", err);
        }
    }
}

IntSetting::IntSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(char *) = NULL)
    : Setting(description, type, permissions, grblName, name, checker)
    , _defaultValue(defVal)
    , _currentValue(defVal)
    , _minValue(minVal)
    , _maxValue(maxVal)
{ }

void IntSetting::load() {
    esp_err_t err = nvs_get_i32(_handle, _keyName, &_storedValue);
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
        nvs_erase_key(_handle, _keyName);
    }
}

err_t IntSetting::setStringValue(char* s) {
    s = trim(s);
    if (err_t err = check(s)) {
        return err;
    }
    char* endptr;
    int32_t convertedValue = strtol(s, &endptr, 10);
    if (endptr == s || *endptr != '\0') {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    if (convertedValue < _minValue || convertedValue > _maxValue) {
        return STATUS_NUMBER_RANGE;
    }
    _currentValue = convertedValue;
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
        } else {
            if (nvs_set_i32(_handle, _keyName, _currentValue)) {
                return STATUS_NVS_SET_FAILED;
            }
            _storedValue = _currentValue;
        }
    }
    return STATUS_OK;
}

const char* IntSetting::getStringValue() {
    static char strval[32];
    sprintf(strval, "%d", get());
    return strval;
}

void IntSetting::addWebui(JSONencoder *j) {
    if (getDescription()) {
        j->begin_webui(getName(), getDescription(), "I", getStringValue(), _minValue, _maxValue);
        j->end_object();
    }
}

AxisMaskSetting::AxisMaskSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, int32_t defVal, bool (*checker)(char *) = NULL)
    : Setting(description, type, permissions, grblName, name, checker)
    , _defaultValue(defVal)
    , _currentValue(defVal)
{ }

void AxisMaskSetting::load() {
    esp_err_t err = nvs_get_i32(_handle, _keyName, &_storedValue);
    if (err) {
        _storedValue = -1;
        _currentValue = _defaultValue;
    } else {
        _currentValue = _storedValue;
    }
}

void AxisMaskSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, _keyName);
    }
}

err_t AxisMaskSetting::setStringValue(char* s) {
    s = trim(s);
    if (err_t err = check(s)) {
        return err;
    }
    int32_t convertedValue;
    char* endptr;
    if (*s == '\0') {
        convertedValue = 0;
    } else {
        convertedValue = strtol(s, &endptr, 10);
        if (endptr == s || *endptr != '\0') {
            // Try to convert as an axis list
            convertedValue = 0;
            auto axisNames = String("XYZABC");
            while (*s) {
                int index = axisNames.indexOf(toupper(*s++));
                if (index < 0) {
                    return STATUS_BAD_NUMBER_FORMAT;
                }
                convertedValue |= bit(index);
            }
        }
    }
    _currentValue = convertedValue;
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
        } else {
            if (nvs_set_i32(_handle, _keyName, _currentValue)) {
                return STATUS_NVS_SET_FAILED;
            }
            _storedValue = _currentValue;
        }
    }
    return STATUS_OK;
}

const char* AxisMaskSetting::getCompatibleValue() {
    static char strval[32];
    sprintf(strval, "%d", get());
    return strval;
}

const char* AxisMaskSetting::getStringValue() {
    static char strval[32];
    char *s = strval;
    uint32_t mask = get();
    for (int i = 0; i < MAX_N_AXIS; i++) {
        if (mask & bit(i)) {
            *s++ = "XYZABC"[i];
        }
    }
    *s = '\0';
    return strval;
}

void AxisMaskSetting::addWebui(JSONencoder *j) {
    if (getDescription()) {
        j->begin_webui(getName(), getDescription(), "I", getStringValue(), 0, (1<<MAX_N_AXIS)-1);
        j->end_object();
    }
}

FloatSetting::FloatSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(char *) = NULL)
    : Setting(description, type, permissions, grblName, name, checker)
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
    if (nvs_get_i32(_handle, _keyName, &v.ival)) {
        _currentValue = _defaultValue;
    } else {
        _currentValue = v.fval;
    }
}

void FloatSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, _keyName);
    }
}

err_t FloatSetting::setStringValue(char* s) {
    s = trim(s);
    if (err_t err = check(s)) {
        return err;
    }

    float convertedValue;
    uint8_t len = strlen(s);
    uint8_t retlen = 0;
    if (!read_float(s, &retlen, &convertedValue)
        || retlen != len)
    {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    if (convertedValue < _minValue || convertedValue > _maxValue) {
        return STATUS_NUMBER_RANGE;
    }
    _currentValue = convertedValue;
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
        } else {
            union {
                int32_t ival;
                float   fval;
            } v;
            v.fval = _currentValue;
            if (nvs_set_i32(_handle, _keyName, v.ival)) {
                return STATUS_NVS_SET_FAILED;
            }
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

StringSetting::StringSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, const char* defVal, int min, int max, bool (*checker)(char *))
    : Setting(description, type, permissions, grblName, name, checker)
{
    _defaultValue = defVal;
    _currentValue = defVal;
    _minLength = min;
    _maxLength = max;
 };

void StringSetting::load() {
    size_t len = 0;
    esp_err_t err = nvs_get_str(_handle, _keyName, NULL, &len);
    if(err) {
        _storedValue = _defaultValue;
        _currentValue = _defaultValue;
        return;
    }
    char buf[len];
    err = nvs_get_str(_handle, _keyName, buf, &len);
    if (err) {
        _storedValue = _defaultValue;
        _currentValue = _defaultValue;
        return;
    }
    _storedValue = String(buf);
    _currentValue = _storedValue;
}

void StringSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, _keyName);
    }
}

err_t StringSetting::setStringValue(char* s) {
    if (_minLength && _maxLength && (strlen(s) < _minLength || strlen(s) > _maxLength)) {
        return STATUS_BAD_NUMBER_FORMAT;
    }
    if (err_t err = check(s)) {
        return err;
    }
   _currentValue = s;
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
            _storedValue = _defaultValue;
        } else {
            if (nvs_set_str(_handle, _keyName, _currentValue.c_str())) {
                return STATUS_NVS_SET_FAILED;
            }
            _storedValue = _currentValue;
        }
    }
    return STATUS_OK;
}

const char* StringSetting::getStringValue() {
    // If the string is a password do not display it
    if (_checker &&
        (
       #ifdef ENABLE_WIFI
         _checker == (bool (*)(char *))WiFiConfig::isPasswordValid
         ||
       #endif
         _checker == (bool (*)(char *))COMMANDS::isLocalPasswordValid
         )) {
        return "******";
    }
    return _currentValue.c_str();
}

void StringSetting::addWebui(JSONencoder *j) {
    if (!getDescription()) {
        return;
    }
    j->begin_webui(
        getName(), getDescription(), "S", getStringValue(), _minLength, _maxLength);
    j->end_object();
}

typedef std::map<const char *, int8_t, cmp_str> enum_opt_t;

EnumSetting::EnumSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, int8_t defVal, enum_opt_t *opts)
    // No checker function because enumerations have an exact set of value
    : Setting(description, type, permissions, grblName, name, NULL)
    , _defaultValue(defVal)
    , _options(opts)
{ }

void EnumSetting::load() {
    esp_err_t err = nvs_get_i8(_handle, _keyName, &_storedValue);
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
        nvs_erase_key(_handle, _keyName);
    }
}

// For enumerations, we allow the value to be set
// either with the string name or the numeric value.
// This is necessary for WebUI, which uses the number
// for setting.
err_t EnumSetting::setStringValue(char* s) {
    s = trim(s);
    enum_opt_t::iterator it = _options->find(s);
    if (it == _options->end()) {
        // If we don't find the value in keys, look for it in the numeric values

        // Disallow empty string
        if (!s || !*s) {
            return STATUS_BAD_NUMBER_FORMAT;
        }
        char *endptr;
        uint8_t num = strtol(s, &endptr, 10);
        // Disallow non-numeric characters in string
        if (*endptr) {
            return STATUS_BAD_NUMBER_FORMAT;
        }
        for (it = _options->begin(); it != _options->end(); it++) {
            if (it->second == num) {
                break;
            }
        }
        if (it == _options->end()) {
            return STATUS_BAD_NUMBER_FORMAT;
        }
    }
    _currentValue = it->second;
    if (_storedValue != _currentValue) {
        if (_storedValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
        } else {
            if (nvs_set_i8(_handle, _keyName, _currentValue)) {
                return STATUS_NVS_SET_FAILED;
            }
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
    if (!getDescription()) {
      return;
    }
    j->begin_webui(getName(), getDescription(), "B", String(get()).c_str());
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

FlagSetting::FlagSetting(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* name, bool defVal, bool (*checker)(char *) = NULL) :
    Setting(description, type, permissions, grblName, name, checker),
    _defaultValue(defVal)
{ }

void FlagSetting::load() {
    esp_err_t err = nvs_get_i8(_handle, _keyName, &_storedValue);
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
        nvs_erase_key(_handle, _keyName);
    }
}

err_t FlagSetting::setStringValue(char* s) {
    s = trim(s);
    _currentValue = (strcasecmp(s, "on") == 0)
    || (strcasecmp(s, "true") == 0)
    || (strcasecmp(s, "enabled") == 0)
    || (strcasecmp(s, "yes") == 0)
    || (strcasecmp(s, "1") == 0);
    // _storedValue is -1, 0, or 1
    // _currentValue is 0 or 1
    if (_storedValue != (int8_t)_currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
        } else {
            if (nvs_set_i8(_handle, _keyName, _currentValue)) {
                return STATUS_NVS_SET_FAILED;
            }
            _storedValue = _currentValue;
        }
    }
    return STATUS_OK;
}
const char* FlagSetting::getStringValue() {
    return get() ? "On" : "Off";
}
const char* FlagSetting::getCompatibleValue() {
    return get() ? "1" : "0";
}


#include <WiFi.h>

IPaddrSetting::IPaddrSetting(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* name, uint32_t defVal, bool (*checker)(char *) = NULL)
      : Setting(description, type, permissions, grblName, name, checker) // There are no GRBL IP settings.
      , _defaultValue(defVal)
      , _currentValue(defVal)
{ }

IPaddrSetting::IPaddrSetting(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* name, const char *defVal, bool (*checker)(char *) = NULL)
    : Setting(description, type, permissions, grblName, name, checker)
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
    esp_err_t err = nvs_get_i32(_handle, _keyName, (int32_t *)&_storedValue);
    if (err) {
        _storedValue = 0x000000ff;  // Unreasonable value for any IP thing
        _currentValue = _defaultValue;
    } else {
        _currentValue = _storedValue;
    }
}

void IPaddrSetting::setDefault() {
    _currentValue = _defaultValue;
    if (_storedValue != _currentValue) {
        nvs_erase_key(_handle, _keyName);
    }
}

err_t IPaddrSetting::setStringValue(char* s) {
    s = trim(s);
    if (err_t err = check(s)) {
        return err;
    }
    IPAddress ipaddr;
    if (!ipaddr.fromString(s)) {
        return STATUS_INVALID_VALUE;
    }
    _currentValue = ipaddr;
    if (_storedValue != _currentValue) {
        if (_currentValue == _defaultValue) {
            nvs_erase_key(_handle, _keyName);
        } else {
            if (nvs_set_i32(_handle, _keyName, (int32_t)_currentValue)) {
                return STATUS_NVS_SET_FAILED;
            }
            _storedValue = _currentValue;
        }
    }
    return STATUS_OK;
}

const char* IPaddrSetting::getStringValue() {
    static String s;
    IPAddress ipaddr(get());
    s = ipaddr.toString();
    return s.c_str();
}

void IPaddrSetting::addWebui(JSONencoder *j) {
    if (getDescription()) {
        j->begin_webui(getName(), getDescription(), "A", getStringValue());
        j->end_object();
    }
}

 AxisSettings::AxisSettings(const char *axisName) :
    name(axisName)
{}

err_t GrblCommand::action(char* value, auth_t auth_type, ESPResponseStream* out) {
    if (sys.state & _disallowedStates) {
        return STATUS_IDLE_ERROR;
    }
    return _action((const char*)value, auth_type, out);
};

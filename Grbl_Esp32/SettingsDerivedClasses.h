#pragma once
#include "SettingsBaseClass.h"
#include <nvs.h>

extern nvs_handle _handle;

class IntSetting : public Setting {
private:
    int32_t _defaultValue;
    int32_t _currentValue;
    int32_t _storedValue;
    int32_t _minValue;
    int32_t _maxValue;

public:
    IntSetting(const char *webName, group_t group, const char* grblName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(const char *));

    IntSetting(group_t group, const char* grblName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(const char *) = NULL)
        : IntSetting(NULL, group, grblName, name, defVal, minVal, maxVal, checker)
    { }

    int32_t get() {  return _currentValue;  }

    void load();
    void setDefault();
    err_t setStringValue(const char *);
    const char *getStringValue();
    const char *getStringDefault();
    void addWebui(JSONencoder *);
};

class FloatSetting : public Setting {
private:
    float _defaultValue;
    float _currentValue;
    float _storedValue;
    float _minValue;
    float _maxValue;
public:
    FloatSetting(const char *webName, group_t group, const char* grblName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(const char *));

    FloatSetting(group_t group, const char* grblName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(const char *) = NULL)
        : FloatSetting(NULL, group, grblName, name, defVal, minVal, maxVal, checker)
    { }

    float get() {  return _currentValue;  }
    void load();
    void setDefault();
    err_t setStringValue(const char *);
    const char *getStringValue();
};

#define MAX_SETTING_STRING 256
class StringSetting : public Setting {
private:
    std::string _defaultValue;
    std::string _currentValue;
    std::string _storedValue;
    int _minLength;
    int _maxLength;
    void _setStoredValue(const char *s);
public:
    StringSetting(const char *webName, group_t group, const char* grblName, const char* name, const char* defVal, int min, int max, bool (*checker)(const char *));

    StringSetting(group_t group, const char* grblName, const char* name, const char* defVal, bool (*checker)(const char *) = NULL)
        : StringSetting(NULL, group, grblName, name, defVal, 0, 0, checker)
    { };

    const char* get() { return _currentValue.c_str();  }
    void load();
    void setDefault();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
};
struct cmp_str
{
   bool operator()(char const *a, char const *b) const
   {
      return strcasecmp(a, b) < 0;
   }
};
typedef std::map<const char *, int8_t, cmp_str> enum_opt_t;

class EnumSetting : public Setting {
private:
    int8_t _defaultValue;
    int8_t _storedValue;
    int8_t _currentValue;
    std::map<const char *, int8_t, cmp_str>* _options;
public:
    EnumSetting(const char *webName, group_t group, const char* grblName, const char* name, int8_t defVal, enum_opt_t* opts);

    EnumSetting(group_t group, const char* grblName, const char* name, int8_t defVal, enum_opt_t* opts) :
        EnumSetting(NULL, group, grblName, name, defVal, opts)
    { }

    int8_t get() { return _currentValue;  }
    void load();
    void setDefault();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
};

class FlagSetting : public Setting {
private:
    bool _defaultValue;
    int8_t _storedValue;
    bool _currentValue;
public:
    FlagSetting(const char *webName, group_t group, const char* grblName, const char* name, bool defVal, bool (*checker)(const char *));
    FlagSetting(group_t group, const char* grblName, const char* name, bool defVal, bool (*checker)(const char *) = NULL)
        : FlagSetting(NULL, group, grblName, name, defVal, checker)
    { }

    bool get() {  return _currentValue;  }
    void load();
    void setDefault();
    err_t setStringValue(const char *);
    const char *getStringValue();
    // void addWebui(JSONencoder *);
};

class IPaddrSetting : public Setting {
private:
    uint32_t _defaultValue;
    uint32_t _currentValue;
    uint32_t _storedValue;

public:
    IPaddrSetting(const char *webName, group_t group, const char * grblName, const char* name, uint32_t defVal, bool (*checker)(const char *));
    IPaddrSetting(const char *webName, group_t group, const char * grblName, const char* name, const char *defVal, bool (*checker)(const char *));

    uint32_t get() {  return _currentValue;  }

    void load();
    void setDefault();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
};

class AxisSettings {
public:
    const char* name;
    FloatSetting *steps_per_mm;
    FloatSetting *max_rate;
    FloatSetting *acceleration;
    FloatSetting *max_travel;
    FloatSetting *run_current;
    FloatSetting *hold_current;
    IntSetting *microsteps;
    IntSetting *stallguard;

    AxisSettings(const char *axisName);
};

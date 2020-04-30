#include "SettingsBaseClass.h"

class IntSetting : public Setting {
private:
    int32_t defaultValue;
    int32_t currentValue;
    int32_t storedValue;
    int32_t minValue;
    int32_t maxValue;

public:
    IntSetting(const char *webName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(const char *));

    IntSetting(const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(const char *) = NULL)
        : IntSetting(NULL, name, defVal, minVal, maxVal, checker)
    { }

    int32_t get() {  return currentValue;  }

    void load();
    void commit();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
};

class FloatSetting : public Setting {
private:
    float defaultValue;
    float currentValue;
    float storedValue;
    float minValue;
    float maxValue;
public:
    FloatSetting(const char *webName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(const char *));

    FloatSetting(const char* name, float defVal, float minVal, float maxVal, bool (*checker)(const char *) = NULL)
        : FloatSetting(NULL, name, defVal, minVal, maxVal, checker)
    { }

    float get() {  return currentValue;  }
    void load();
    void commit();
    err_t setStringValue(const char *);
    const char *getStringValue();
};

#define MAX_SETTING_STRING 256
class StringSetting : public Setting {
private:
    std::string defaultValue;
    std::string currentValue;
    std::string storedValue;
    int minLength;
    int maxLength;
    void setStoredValue(const char *s);
public:
    StringSetting(const char *webName, const char* name, const char* defVal, int min, int max, bool (*checker)(const char *));

    StringSetting(const char* name, const char* defVal, bool (*checker)(const char *) = NULL)
        : StringSetting(NULL, name, defVal, 0, 0, checker)
    { };

    const char* get() { return currentValue.c_str();  }
    void load();
    void commit();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
};

class EnumSetting : public Setting {
private:
    int8_t defaultValue;
    int8_t storedValue;
    int8_t currentValue;
    std::map<const char *, int8_t>options;
public:
    EnumSetting(const char *webName, const char* name, int8_t defVal, std::map<const char *, int8_t>opts);

    EnumSetting(const char* name, int8_t defVal, std::map<const char *, int8_t>opts) :
        EnumSetting(NULL, name, defVal, opts)
    { }

    int8_t get() { return currentValue;  }
    void load();
    void commit();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
};

class FlagSetting : public Setting {
private:
    bool defaultValue;
    int8_t storedValue;
    bool currentValue;
public:
    FlagSetting(const char *webName, const char* name, bool defVal, bool (*checker)(const char *));
    FlagSetting(const char* name, bool defVal, bool (*checker)(const char *) = NULL)
        : FlagSetting(NULL, name, defVal, checker)
    { }

    bool get() {  return currentValue;  }
    void load();
    void commit();
    err_t setStringValue(const char *);
    const char *getStringValue();
    // void addWebui(JSONencoder *);
};

class IPaddrSetting : public Setting {
private:
    uint32_t defaultValue;
    uint32_t currentValue;
    uint32_t storedValue;

public:
    IPaddrSetting(const char *webName, const char* name, uint32_t defVal, bool (*checker)(const char *));

    IPaddrSetting(const char *webName, const char* name, const char *defVal, bool (*checker)(const char *));

    uint32_t get() {  return currentValue;  }

    void load();
    void commit();
    err_t setStringValue(const char *);
    const char *getStringValue();
    void addWebui(JSONencoder *);
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
               float run, float hold, int usteps, int stall);
};

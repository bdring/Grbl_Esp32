#pragma once
#include "JSONencoder.h"
#include <map>
#include <nvs.h>
#include "espresponse.h"

// Command::List is a linked list of all settings,
// so common code can enumerate them.
class Command;
// extern Command *CommandsList;

// This abstract class defines the generic interface that
// is used to set and get values for all settings independent
// of their underlying data type.  The values are always
// represented as human-readable strings.  This generic
// interface is used for managing settings via the user interface.

// Derived classes implement these generic functions for different
// kinds of data.  Code that accesses settings should use only these
// generic functions and should not use derived classes directly.

enum {
    NO_AXIS = 255,
};
typedef enum : uint8_t {
    GRBL = 1,  // Classic GRBL settings like $100
    EXTENDED,  // Settings added by early versions of Grbl_Esp32
    WEBSET,    // Settings for ESP3D_WebUI, stored in NVS
    GRBLCMD,   // Non-persistent GRBL commands like $H
    WEBCMD,    // ESP3D_WebUI commands that are not directly settings
} type_t;
typedef enum : uint8_t {
    WG,  // Readable and writable as guest
    WU,  // Readable and writable as user and admin
    WA,  // Readable as user and admin, writable as admin
} permissions_t;
typedef uint8_t axis_t;

class Word {
protected:
    const char*  _description;
    const char*  _grblName;
    const char*  _fullName;
    type_t       _type;
    permissions_t _permissions;
public:
    Word(type_t type, permissions_t permissions, const char *description, const char * grblName, const char* fullName);
    type_t getType() { return _type; }
    permissions_t getPermissions() { return _permissions; }
    const char* getName() { return _fullName; }
    const char* getGrblName() { return _grblName; }
    const char* getDescription() { return _description; }
};

class Command : public Word {
protected:
    Command *link;  // linked list of setting objects
public:
    static Command* List;
    Command* next() { return link; }

    ~Command() {}
    Command(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* fullName);

    // The default implementation of addWebui() does nothing.
    // Derived classes may override it to do something.
    virtual void addWebui(JSONencoder *) {};

    virtual err_t action(char* value, auth_t auth_level, ESPResponseStream* out) =0;
};

class Setting : public Word {
private:
protected:
    static nvs_handle _handle;
    // group_t _group;
    axis_t _axis = NO_AXIS;
    Setting *link;  // linked list of setting objects

    bool (*_checker)(char *);
    const char* _keyName;
public:
    static void init();
    static Setting* List;
    Setting* next() { return link; }

    err_t check(char *s);

    static err_t report_nvs_stats(const char* value, auth_t auth_level, ESPResponseStream* out) {
        nvs_stats_t stats;
        if (err_t err = nvs_get_stats(NULL, &stats))
            return err;
        grbl_sendf(out->client(), "[MSG: NVS Used: %d Free: %d Total: %d]\r\n",
                   stats.used_entries, stats.free_entries, stats.total_entries);
#if 0  // The SDK we use does not have this yet
        nvs_iterator_t it = nvs_entry_find(NULL, NULL, NVS_TYPE_ANY);
        while (it != NULL) {
            nvs_entry_info_t info;
            nvs_entry_info(it, &info);
            it = nvs_entry_next(it);
            grbl_sendf(out->client(), "namespace %s key '%s', type '%d' \n", info.namespace_name, info.key, info.type);
        }
#endif
        return STATUS_OK;
    }

    static err_t eraseNVS(const char* value, auth_t auth_level, ESPResponseStream* out) {
        nvs_erase_all(_handle);
        //        return STATUS_OK;
        return 0;
    }

    ~Setting() {}
    // Setting(const char *description, group_t group, const char * grblName, const char* fullName, bool (*checker)(char *));
    Setting(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* fullName, bool (*checker)(char *));
    axis_t getAxis() { return _axis; }
    void setAxis(axis_t axis) { _axis = axis; }

    // load() reads the backing store to get the current
    // value of the setting.  This could be slow so it
    // should be done infrequently, typically once at startup.
    virtual void load() {};
    virtual void setDefault() {};

    // The default implementation of addWebui() does nothing.
    // Derived classes may override it to do something.
    virtual void addWebui(JSONencoder *) {};

    virtual err_t setStringValue(char* value) =0;
    err_t setStringValue(String s) {  return setStringValue(s.c_str());  }
    virtual const char* getStringValue() =0;
    virtual const char* getCompatibleValue() { return getStringValue(); }
};

class IntSetting : public Setting {
private:
    int32_t _defaultValue;
    int32_t _currentValue;
    int32_t _storedValue;
    int32_t _minValue;
    int32_t _maxValue;

public:
    IntSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(char *));

    IntSetting(type_t type, permissions_t permissions, const char* grblName, const char* name, int32_t defVal, int32_t minVal, int32_t maxVal, bool (*checker)(char *) = NULL)
            : IntSetting(NULL, type, permissions, grblName, name, defVal, minVal, maxVal, checker)
    { }

    void load();
    void setDefault();
    void addWebui(JSONencoder *);
    err_t setStringValue(char* value);
    const char* getStringValue();

    int32_t get() {  return _currentValue;  }
};

class AxisMaskSetting : public Setting {
private:
    int32_t _defaultValue;
    int32_t _currentValue;
    int32_t _storedValue;

public:
    AxisMaskSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, int32_t defVal, bool (*checker)(char *));

    AxisMaskSetting(type_t type, permissions_t permissions, const char* grblName, const char* name, int32_t defVal, bool (*checker)(char *) = NULL)
            : AxisMaskSetting(NULL, type, permissions, grblName, name, defVal, checker)
    { }

    void load();
    void setDefault();
    void addWebui(JSONencoder *);
    err_t setStringValue(char* value);
    const char* getCompatibleValue();
    const char* getStringValue();

    int32_t get() { return _currentValue;  }
};

class FloatSetting : public Setting {
private:
    float _defaultValue;
    float _currentValue;
    float _storedValue;
    float _minValue;
    float _maxValue;
public:
    FloatSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(char *));

    FloatSetting(type_t type, permissions_t permissions, const char* grblName, const char* name, float defVal, float minVal, float maxVal, bool (*checker)(char *) = NULL)
            : FloatSetting(NULL, type, permissions, grblName, name, defVal, minVal, maxVal, checker)
    { }

    void load();
    void setDefault();
    // There are no Float settings in WebUI
    void addWebui(JSONencoder *) {}
    err_t setStringValue(char* value);
    const char* getStringValue();

    float get() {  return _currentValue;  }
};

#define MAX_SETTING_STRING 256
class StringSetting : public Setting {
private:
    String _defaultValue;
    String _currentValue;
    String _storedValue;
    int _minLength;
    int _maxLength;
    void _setStoredValue(const char *s);
public:
    StringSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, const char* defVal, int min, int max, bool (*checker)(char *));

    StringSetting(type_t type, permissions_t permissions, const char* grblName, const char* name, const char* defVal, bool (*checker)(char *) = NULL)
        : StringSetting(NULL, type, permissions, grblName, name, defVal, 0, 0, checker)
    { };

    void load();
    void setDefault();
    void addWebui(JSONencoder *);
    err_t setStringValue(char* value);
    const char* getStringValue();

    const char* get() { return _currentValue.c_str();  }
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
    EnumSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, int8_t defVal, enum_opt_t* opts);

    EnumSetting(type_t type, permissions_t permissions, const char* grblName, const char* name, int8_t defVal, enum_opt_t* opts) :
        EnumSetting(NULL, type, permissions, grblName, name, defVal, opts)
    { }

    void load();
    void setDefault();
    void addWebui(JSONencoder *);
    err_t setStringValue(char* value);
    const char* getStringValue();

    int8_t get() { return _currentValue;  }
};

class FlagSetting : public Setting {
private:
    bool _defaultValue;
    int8_t _storedValue;
    bool _currentValue;
public:
    FlagSetting(const char *description, type_t type, permissions_t permissions, const char* grblName, const char* name, bool defVal, bool (*checker)(char *));
    FlagSetting(type_t type, permissions_t permissions, const char* grblName, const char* name, bool defVal, bool (*checker)(char *) = NULL)
        : FlagSetting(NULL, type, permissions, grblName, name, defVal, checker)
    { }

    void load();
    void setDefault();
    // There are no Flag settings in WebUI
    // The booleans are expressed as Enums
    void addWebui(JSONencoder *) {}
    err_t setStringValue(char* value);
    const char* getCompatibleValue();
    const char* getStringValue();

    bool get() {  return _currentValue;  }
};

class IPaddrSetting : public Setting {
private:
    uint32_t _defaultValue;
    uint32_t _currentValue;
    uint32_t _storedValue;

public:
    IPaddrSetting(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* name, uint32_t defVal, bool (*checker)(char *));
    IPaddrSetting(const char *description, type_t type, permissions_t permissions, const char * grblName, const char* name, const char *defVal, bool (*checker)(char *));

    void load();
    void setDefault();
    void addWebui(JSONencoder *);
    err_t setStringValue(char* value);
    const char* getStringValue();

    uint32_t get() {  return _currentValue;  }
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
class WebCommand : public Command {
    private:
        err_t (*_action)(char *, auth_t);
        const char* password;
    public:
    WebCommand(const char* description, type_t type, permissions_t permissions, const char * grblName, const char* name, err_t (*action)(char *, auth_t)) :
        Command(description, type, permissions, grblName, name),
        _action(action)
    {}
    err_t action(char* value, auth_t auth_level, ESPResponseStream* response);
};

enum : uint8_t {
    ANY_STATE = 0,
    IDLE_OR_ALARM = 0xff & ~STATE_ALARM,
    IDLE_OR_JOG = 0xff & ~STATE_JOG,
    NOT_CYCLE_OR_HOLD = STATE_CYCLE | STATE_HOLD,
};

class GrblCommand : public Command {
    private:
        err_t (*_action)(const char *, auth_t, ESPResponseStream*);
        uint8_t _disallowedStates;
    public:
        GrblCommand(const char * grblName, const char* name, err_t (*action)(const char*, auth_t, ESPResponseStream*), uint8_t disallowedStates, permissions_t auth)
        : Command(NULL, GRBLCMD, auth, grblName, name)
        , _action(action)
        , _disallowedStates(disallowedStates)
    {}

    GrblCommand(const char * grblName, const char* name, err_t (*action)(const char*, auth_t, ESPResponseStream*), uint8_t disallowedStates)
        : GrblCommand(grblName, name, action, disallowedStates, WG)
    {}
    err_t action(char* value, auth_t auth_level, ESPResponseStream* response);
};

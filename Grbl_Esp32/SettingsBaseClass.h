#pragma once
#include "grbl.h"
#include "JSONencoder.h"
//#include <string>
#include <map>

using namespace std;

// SettingsList is a linked list of all settings,
// so common code can enumerate them.
class Setting;
extern Setting *SettingsList;

// This abstract class defines the generic interface that
// is used to set and get values for all settings independent
// of their underlying data type.  The values are always
// represented as human-readable strings.  This generic
// interface is used for managing settings via the user interface.

// Derived classes implement these generic functions for different
// kinds of data.  Code that accesses settings should use only these
// generic functions and should not use derived classes directly.

class Setting {
private:
    const char* _webuiName;
    const char *_grblName;
    const char* _displayName;
    bool (*_checker)(const char *);
    // Add each constructed setting to the linked list

public:
    Setting *link;
    Setting *next() { return link; }

    bool check(const char *s) {
        return _checker ? _checker(s) : false;
    }

    ~Setting() {}
    Setting(const char *webuiName, const char * grblName, const char* displayName, bool (*checker)(const char *));
    const char* getName() { return _displayName; }
    const char* getGrblName() { return _grblName; }
    const char* getWebuiName() { return _webuiName; }

    // load() reads the backing store to get the current
    // value of the setting.  This could be slow so it
    // should be done infrequently, typically once at startup.
    virtual void load() =0;

    // commit() puts the current value of the setting into
    // the backing store.
    virtual void commit() =0;

    virtual err_t setStringValue(const char* value) =0;
    err_t setStringValue(string s) {  return setStringValue(s.c_str());  }

    virtual const char* getStringValue() =0;

    // The default implementation of addWebui() does nothing.
    // Derived classes may override it to do something.
    virtual void addWebui(JSONencoder *) { };
};

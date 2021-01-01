#include "Generator.h"

#include "Configurable.h"

#include <cstring>

namespace Configuration
{
    void Generator::enter(const char* name)
    {
        indent();
        addStr(name);
        addStr(":\n");
        indent_++;
    }

    void Generator::add(const char* key, const std::string& value)
    {
        add(key, value.c_str());
    }

    void Generator::add(const char* key, const char* value)
    {
        indent();
        addStr(key);
        addStr(": ");
        addStr(value);
        addStr("\n");
    }

    void Generator::add(const char* key, bool value)
    {
        if (value) { add(key, "true"); } 
        else { add(key, "false"); }
    }

    void Generator::add(const char* key, int value)
    {
        char tmp[11];
        snprintf(tmp, 11, "%d", value);
        add(key, tmp);
    }

    void Generator::add(const char* key, double value)
    {
        char tmp[20];
        snprintf(tmp, 20, "%f", value);
        add(key, tmp);
    }

    void Generator::add(const char* key, Pin value)
    {
        if (!value.undefined())
        {
            add(key, value.str());
        }
    }

    void Generator::add(Configuration::Configurable* configurable)
    {
        if (configurable != nullptr)
        {
            configurable->generate(*this);
        }
    }

    void Generator::leave()
    {
        addStr("\n");
        indent_--;
    }
}
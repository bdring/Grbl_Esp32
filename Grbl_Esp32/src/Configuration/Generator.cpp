#include "Generator.h"

#include "Configurable.h"

#include <cstring>
#include <cstdio>

namespace Configuration
{
    void Generator::enter(const char* name)
    {
        indent();
        dst_ << name << ":\n";
        indent_++;
    }

    void Generator::add(Configuration::Configurable* configurable)
    {
        if (configurable != nullptr)
        {
            configurable->handle(*this);
        }
    }

    void Generator::leave()
    {
        if (!lastIsNewline_)
        {
            dst_ << '\n';
            lastIsNewline_ = true;
        }

        indent_--;
    }

    void Generator::handleDetail(const char* name, Configurable* value) {
        enter(name);
        value->handle(*this);
        leave();
    }

}
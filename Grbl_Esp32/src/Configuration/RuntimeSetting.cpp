#include "RuntimeSetting.h"

#include <cstdlib>

namespace Configuration
{
    void RuntimeSetting::handleDetail(const char* name, Configuration::Configurable* value)
    {
        if (is(name) && this->value() == nullptr)
        {
            auto previous = start_;

            // Figure out next node
            auto next = start_;
            for (; *next && *next != '=' && *next != '/'; ++next)
            {
            }
            
            // Do we have a child?
            if (*next == '/') {
                ++next;
                start_ = next;

                // Handle child:
                value->handle(*this);
            }

            // Restore situation:
            start_ = previous;
        }
    }

    void RuntimeSetting::handle(const char* name, int& value)
    {
        if (is(name) && this->value() != nullptr)
        {
            value = atoi(this->value());
        }
    }

    void RuntimeSetting::handle(const char* name, double& value)
    {
        if (is(name) && this->value() != nullptr)
        {
            char* floatEnd;
            value = strtod(this->value(), &floatEnd);
        }
    }

    void RuntimeSetting::handle(const char* name, StringRange value)
    {
        if (is(name) && this->value() != nullptr)
        {
            value = this->value();
        }
    }

    void RuntimeSetting::handle(const char* name, Pin& value)
    {
        if (is(name) && this->value() != nullptr)
        {
            value = Pin::create(StringRange(this->value()));
        }
    }
}

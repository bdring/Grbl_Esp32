#include "LegacySettingRegistry.h"

#include "LegacySettingHandler.h"

namespace Configuration 
{
    bool LegacySettingRegistry::isLegacySetting(const char* str)
    {
        return str[0] == '$' && (str[1] >= '0' && str[1] <= '9');
    }

    void LegacySettingRegistry::registerHandler(LegacySettingHandler* handler)
    {
        instance().handlers_.push_back(handler);
    }

    bool LegacySettingRegistry::tryHandleLegacy(const char* str) {
        if (isLegacySetting(str)) {
            auto start = str;

            int value = 0;
            ++str;

            while (*str && *str >= '0' && *str <= '9')
            {
                value = value * 10 + (*str - '0');
                ++str;
            }

            if (*str == '=') {
                ++str;

                handleLegacy(value, str);
            }
            else {
                warn("Incorrect setting '" << start << "': cannot find '='.");
            }
            return true;
        }
        else {
            return false;
        }
    }

    void LegacySettingRegistry::handleLegacy(int index, const char* value) {
        bool handled = false;
        for (auto it : instance().handlers_) {
            if (it->index() == index)
            {
                handled = true;
                it->handle(value);
            }
        }

        if (!handled) {
            warn("Cannot find handler for $" << index << ". Setting was ignored.");
        }
    }
}
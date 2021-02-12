#pragma once

#include <vector>
#include "../Logging.h"

namespace Configuration {
    class LegacySettingHandler;

    class LegacySettingRegistry
    {
        static LegacySettingRegistry& instance() {
            static LegacySettingRegistry instance_;
            return instance_;
        }

        LegacySettingRegistry() = default;

        LegacySettingRegistry(const LegacySettingRegistry&) = delete;
        LegacySettingRegistry& operator=(const LegacySettingRegistry&) = delete;

        std::vector<LegacySettingHandler*> handlers_;

        static bool isLegacySetting(const char* str);
        static void handleLegacy(int index, const char* value);

    public:
        static void registerHandler(LegacySettingHandler* handler);
        static bool tryHandleLegacy(const char* str);
    };
}
#pragma once

#include "LegacySettingRegistry.h"

namespace Configuration
{
    class LegacySettingHandler
    {
    public:
        inline LegacySettingHandler() {
            LegacySettingRegistry::registerHandler(this);
        }

        LegacySettingHandler(const LegacySettingHandler&) = delete;
        LegacySettingHandler(LegacySettingHandler&&) = delete;
        LegacySettingHandler& operator=(const LegacySettingHandler&) = delete;
        LegacySettingHandler& operator=(LegacySettingHandler&&) = delete;

        virtual int index() = 0;
        virtual void handle(const char* value) = 0;
        virtual ~LegacySettingHandler() {
            // Remove from factory? We shouldn't remove handlers...
        }
    };
}
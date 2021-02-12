#pragma once

#include "HandlerBase.h"
#include "Configurable.h"

namespace Configuration
{
    class RuntimeSetting : public Configuration::HandlerBase
    {
    private:
        const char* setting_; // $foo/bar=12
        const char* start_;

        bool is(const char* name) const {
            if (start_ != nullptr) {
                auto len = strlen(name);
                return !strncmp(name, start_, len) && (start_[len] == '=' || start_[len] == '/');
            }
            else {
                return false;
            }
        }

        const char* value() const
        {
            for (const char* it = start_; *it; ++it)
            {
                if (*it == '/') { return nullptr; }
                else if (*it == '=') { return it + 1; }
            }
            return nullptr;
        }

    protected:
        void handleDetail(const char* name, Configuration::Configurable* value) override;

        bool matchesUninitialized(const char* name) override { return false; }

    public:
        RuntimeSetting(const char* runtimeSetting) : setting_(runtimeSetting + 1), start_(runtimeSetting+1) {}

        void handle(const char* name, int& value) override;
        void handle(const char* name, double& value) override;
        void handle(const char* name, StringRange value) override;
        void handle(const char* name, Pin& value) override;

        HandlerType handlerType() override { return HandlerType::Runtime; }
    };
}
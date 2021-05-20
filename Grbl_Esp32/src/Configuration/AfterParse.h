#pragma once

#include <vector>

#include "../Pin.h"
#include "HandlerBase.h"

namespace Configuration {
    class Configurable;

    class AfterParse : public HandlerBase {
        AfterParse(const AfterParse&) = delete;
        AfterParse& operator=(const AfterParse&) = delete;

    protected:
        void        handleDetail(const char* name, Configurable* value) override;
        bool        matchesUninitialized(const char* name) override { return false; }
        HandlerType handlerType() override { return HandlerType::AfterParse; }

    public:
        AfterParse() = default;

        void handle(const char* name, bool& value) override {}
        void handle(const char* name, int& value) override {}
        void handle(const char* name, float& value) override {}
        void handle(const char* name, double& value) override {}
        void handle(const char* name, StringRange& value) override {}
        void handle(const char* name, Pin& value) override {}
    };
}

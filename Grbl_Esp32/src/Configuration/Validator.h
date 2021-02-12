#pragma once

#include <vector>

#include "../Pin.h"
#include "HandlerBase.h"

namespace Configuration
{
    class Configurable;

    class Validator : public HandlerBase
    {
        Validator(const Validator&) = delete;
        Validator& operator=(const Validator&) = delete;

    protected:
        void handleDetail(const char* name, Configurable* value) override;
        bool matchesUninitialized(const char* name) override { return false; }
        HandlerType handlerType() override { return HandlerType::Validator; }

    public:
        Validator() = default;

        void handle(const char* name, int& value) override { }
        void handle(const char* name, double& value) override { }
        void handle(const char* name, StringRange value) override { }
        void handle(const char* name, Pin& value) override { }
    };
}
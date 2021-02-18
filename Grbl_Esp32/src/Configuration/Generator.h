#pragma once

#include <vector>

#include "../Pin.h"
#include "../StringRange.h"
#include "../StringStream.h"
#include "HandlerBase.h"

namespace Configuration
{
    class Configurable;

    class Generator : public HandlerBase
    {
        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;

        int indent_;
        SimpleOutputStream& dst_;
        bool lastIsNewline_ = false;

        inline void indent() {
            lastIsNewline_ = false;
            for (int i = 0; i < indent_ * 2; ++i)
            {
                dst_ << ' ';
            }
        }

        void enter(const char* name);
        void add(Configuration::Configurable* configurable);
        void leave();
        
    protected:
        void handleDetail(const char* name, Configurable* value) override;
        bool matchesUninitialized(const char* name) override { return false; }
        HandlerType handlerType() override { return HandlerType::Generator; }

    public:
        Generator(SimpleOutputStream& dst) : indent_(0), dst_(dst) {}

        void handle(const char* name, int& value) override {
            indent();
            dst_ << name << ": " << value << '\n';
        }

        void handle(const char* name, double& value) override {
            indent();
            dst_ << name << ": " << value << '\n';
        }

        void handle(const char* name, StringRange& value) override {
            indent();
            dst_ << name << ": " << value << '\n';
        }

        void handle(const char* name, Pin& value) override {
            indent();
            dst_ << name << ": " << value << '\n';
        }
    };
}

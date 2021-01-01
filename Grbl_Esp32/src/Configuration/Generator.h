#pragma once

#include <string>
#include <vector>

#include "../Pin.h"

namespace Configuration
{
    class Configurable;

    class Generator
    {
        Generator(const Generator&) = delete;
        Generator& operator=(const Generator&) = delete;

        std::vector<char> config_;
        int indent_;

        inline void addStr(const char* text) {
            for (auto it = text; *it; ++it)
            {
                config_.push_back(*it);
            }
        }

        inline void indent() {
            for (int i = 0; i < indent_ * 2; ++i)
            {
                config_.push_back(' ');
            }
        }

    public:
        Generator() = default;

        void enter(const char* name);
        void add(const char* key, const std::string& value);
        void add(const char* key, const char* value);
        void add(const char* key, bool value);
        void add(const char* key, int value);
        void add(const char* key, double value);
        void add(const char* key, Pin value);
        void add(Configuration::Configurable* configurable);
        void leave();

        inline std::string str() const {
            return std::string(config_.begin(), config_.end());
        }
    };
}
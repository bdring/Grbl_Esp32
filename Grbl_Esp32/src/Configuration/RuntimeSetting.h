/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "HandlerBase.h"
#include "Configurable.h"

namespace Configuration {
    class RuntimeSetting : public Configuration::HandlerBase {
    private:
        const char* setting_;  // $foo/bar=12
        const char* start_;

        bool is(const char* name) const {
            if (start_ != nullptr) {
                auto len = strlen(name);
                return !strncmp(name, start_, len) && (start_[len] == '=' || start_[len] == '/');
            } else {
                return false;
            }
        }

        const char* value() const {
            for (const char* it = start_; *it; ++it) {
                if (*it == '/') {
                    return nullptr;
                } else if (*it == '=') {
                    return it + 1;
                }
            }
            return nullptr;
        }

    protected:
        void handleDetail(const char* name, Configuration::Configurable* value) override;

        bool matchesUninitialized(const char* name) override { return false; }

    public:
        RuntimeSetting(const char* runtimeSetting);

        void handle(const char* name, int32_t& value, int32_t minValue, int32_t maxValue) override;
        void handle(const char* name, float& value, float minValue, float maxValue) override;
        void handle(const char* name, StringRange& value, int minLength, int maxLength) override;
        void handle(const char* name, Pin& value) override;
        void handle(const char* name, int& value, EnumItem* e) override {}

        HandlerType handlerType() override { return HandlerType::Runtime; }

        virtual ~RuntimeSetting();
    };
}

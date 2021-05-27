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

#include <vector>

#include "../Pin.h"
#include "HandlerBase.h"

namespace Configuration {
    class Configurable;

    class Validator : public HandlerBase {
        Validator(const Validator&) = delete;
        Validator& operator=(const Validator&) = delete;

    protected:
        void        handleDetail(const char* name, Configurable* value) override;
        bool        matchesUninitialized(const char* name) override { return false; }
        HandlerType handlerType() override { return HandlerType::Validator; }

    public:
        Validator();

        void handle(const char* name, bool& value) override {}
        void handle(const char* name, int& value) override {}
        void handle(const char* name, float& value) override {}
        void handle(const char* name, double& value) override {}
        void handle(const char* name, StringRange& value) override {}
        void handle(const char* name, Pin& value) override {}
        void handle(const char* name, int& value, EnumItem* e) override {}
    };
}

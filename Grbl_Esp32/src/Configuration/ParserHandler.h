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
#include "Parser.h"
#include "Configurable.h"
#include "../EnumItem.h"

#include "../Logging.h"

namespace Configuration {
    class ParserHandler : public Configuration::HandlerBase {
    private:
        Configuration::Parser& parser_;

    protected:
        void handleDetail(const char* name, Configuration::Configurable* value) override {
            if (value != nullptr && parser_.is(name)) {
                log_debug("Parsing configurable " << name);

                parser_.enter();
                for (; !parser_.isEndSection(); parser_.moveNext()) {
                    value->handle(*this);
                }
                parser_.leave();
            }
        }

        bool matchesUninitialized(const char* name) override { return parser_.is(name); }

    public:
        ParserHandler(Configuration::Parser& parser) : parser_(parser) {}

        void handle(const char* name, int32_t& value, int32_t minValue, int32_t maxValue) override {
            if (parser_.is(name)) {
                value = parser_.intValue();
            }
        }

        void handle(const char* name, int& value, EnumItem* e) override {
            if (parser_.is(name)) {
                value = parser_.enumValue(e);
            }
        }

        void handle(const char* name, bool& value) override {
            if (parser_.is(name)) {
                value = parser_.boolValue();
            }
        }

        void handle(const char* name, float& value, float minValue, float maxValue) override {
            if (parser_.is(name)) {
                value = parser_.floatValue();
            }
        }

        void handle(const char* name, StringRange& value, int minLength, int maxLength) override {
            if (parser_.is(name)) {
                value = parser_.stringValue();
            }
        }

        void handle(const char* name, Pin& value) override {
            if (parser_.is(name)) {
                auto parsed = parser_.pinValue();
                value.swap(parsed);
            }
        }

        void handle(const char* name, IPAddress& value) override {
            if (parser_.is(name)) {
                value = parser_.ipValue();
            }
        }

        HandlerType handlerType() override { return HandlerType::Parser; }
    };
}

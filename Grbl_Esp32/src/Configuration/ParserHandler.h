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
        Configuration::Parser& _parser;
        bool                   _previousIsLeave = false;

        void checkPreviousLeave() {
            // When a section does a 'leave', the token becomes invalid, and we need to do a
            // moveNext. See the tests for the yaml parser for what the parser expects.
            // So, let's introduce that here:
            if (_previousIsLeave) {
                _parser.moveNext();
            }
            _previousIsLeave = false;
        }

    protected:
        void handleDetail(const char* name, Configuration::Configurable* value) override {
            if (value != nullptr && _parser.is(name)) {
                log_debug("Parsing configurable " << name);
                checkPreviousLeave();

                _parser.enter();
                for (; !_parser.isEndSection(); _parser.moveNext()) {
                    value->handle(*this);
                    _previousIsLeave = false;
                }
                _parser.leave();
                _previousIsLeave = true;
            }
        }

        bool matchesUninitialized(const char* name) override { return _parser.is(name); }

    public:
        ParserHandler(Configuration::Parser& parser) : _parser(parser) {}

        void handle(const char* name, int32_t& value, int32_t minValue, int32_t maxValue) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                value = _parser.intValue();
            }
        }

        void handle(const char* name, int& value, EnumItem* e) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                value = _parser.enumValue(e);
            }
        }

        void handle(const char* name, bool& value) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                value = _parser.boolValue();
            }
        }

        void handle(const char* name, float& value, float minValue, float maxValue) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                value = _parser.floatValue();
            }
        }

        void handle(const char* name, StringRange& value, int minLength, int maxLength) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                value = _parser.stringValue();
            }
        }

        void handle(const char* name, Pin& value) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                auto parsed = _parser.pinValue();
                value.swap(parsed);
            }
        }

        void handle(const char* name, IPAddress& value) override {
            if (_parser.is(name)) {
                checkPreviousLeave();
                value = _parser.ipValue();
            }
        }

        HandlerType handlerType() override { return HandlerType::Parser; }

        void moveNext() {
            _previousIsLeave = false;
            _parser.moveNext();
        }
    };
}

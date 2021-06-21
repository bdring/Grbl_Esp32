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

#include <IPAddress.h>
#include "HandlerType.h"
#include "../Pin.h"
#include "../StringRange.h"
#include "../EnumItem.h"
#include "../SpindleDatatypes.h"

namespace Configuration {
    class Configurable;

    typedef struct {
        SpindleSpeed speed;
        float        percent;
        uint32_t     offset;
        uint32_t     scale;
    } speedEntry;

    template <typename BaseType>
    class GenericFactory;

    class HandlerBase {
    protected:
        virtual void enterSection(const char* name, Configurable* value) = 0;
        virtual bool matchesUninitialized(const char* name)              = 0;

        template <typename BaseType>
        friend class GenericFactory;

    public:
        virtual void item(const char* name, bool& value)                                                        = 0;
        virtual void item(const char* name, int32_t& value, int32_t minValue = 0, int32_t maxValue = INT32_MAX) = 0;

        /* We bound uint32_t to INT32_MAX because we use the same parser */
        void item(const char* name, uint32_t& value, uint32_t minValue = 0, uint32_t maxValue = INT32_MAX) {
            int32_t v = int32_t(value);
            item(name, v, int32_t(minValue), int32_t(maxValue));
            value = uint32_t(v);
        }

        void item(const char* name, uint8_t& value, uint8_t minValue = 0, uint8_t maxValue = UINT8_MAX) {
            int32_t v = int32_t(value);
            item(name, v, int32_t(minValue), int32_t(maxValue));
            value = uint8_t(v);
        }

        virtual void item(const char* name, float& value, float minValue = -3e38, float maxValue = 3e38) = 0;
        virtual void item(const char* name, std::vector<speedEntry>& value)                              = 0;

        virtual void item(const char* name, StringRange& value, int minLength = 0, int maxLength = 255) = 0;
        virtual void item(const char* name, Pin& value)                                                 = 0;
        virtual void item(const char* name, IPAddress& value)                                           = 0;

        virtual void item(const char* name, int& value, EnumItem* e) = 0;

        virtual void item(const char* name, String& value) {
            StringRange range(value);
            StringRange copy(value);

            item(name, range);

            // Check for changes, and update if the string is changed.
            if (range.begin() != copy.begin() || range.end() != copy.end()) {
                value = range.str();
            }
        }

        virtual HandlerType handlerType() = 0;

        template <typename T>
        void section(const char* name, T*& value) {
            if (handlerType() == HandlerType::Parser) {
                // For Parser, matchesUninitialized(name) resolves to _parser.is(name)
                if (value == nullptr && matchesUninitialized(name)) {
                    value = new T();
                    enterSection(name, value);
                }
            } else {
                if (value != nullptr) {
                    enterSection(name, value);
                }
            }
        }

        template <typename T>
        void enterFactory(const char* name, T& value) {
            enterSection(name, &value);
        }
    };
}

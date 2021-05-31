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

#include "HandlerType.h"
#include "../Pin.h"
#include "../StringRange.h"
#include "../EnumItem.h"

namespace Configuration {
    class Configurable;

    template <typename BaseType>
    class GenericFactory;

    class HandlerBase {
    protected:
        virtual void handleDetail(const char* name, Configurable* value) = 0;
        virtual bool matchesUninitialized(const char* name)              = 0;

        template <typename BaseType>
        friend class GenericFactory;

    public:
        virtual void handle(const char* name, bool& value)                                                        = 0;
        virtual void handle(const char* name, int32_t& value, int32_t minValue = 0, int32_t maxValue = INT32_MAX) = 0;

        /* We bound uint32_t to INT32_MAX because we use the same parser */
        void handle(const char* name, uint32_t& value, uint32_t minValue = 0, uint32_t maxValue = INT32_MAX) {
            int32_t v = int32_t(value);
            handle(name, v, int32_t(minValue), int32_t(maxValue));
            value = uint32_t(v);
        }

        void handle(const char* name, uint8_t& value, uint8_t minValue = 0, uint8_t maxValue = UINT8_MAX) {
            int32_t v = int32_t(value);
            handle(name, v, int32_t(minValue), int32_t(maxValue));
            value = uint8_t(v);
        }

        virtual void handle(const char* name, double& value, double minValue = -1e300, double maxValue = 1e300) = 0;
        
        void         handle(const char* name, float& value, float minValue = -1e30f, float maxValue = 1e30f) {
            double d = double(value);
            handle(name, d, double(minValue), double(maxValue));
            value = float(d);
        }

        virtual void handle(const char* name, StringRange& value) = 0;
        virtual void handle(const char* name, Pin& value)         = 0;

        virtual void handle(const char* name, int& value, EnumItem* e) = 0;
        virtual void handle(const char* name, String& value) {
            StringRange range;
            handle(name, range);
            if (range.begin() != nullptr) {
                value = range.str();
            }
        }

        virtual HandlerType handlerType() = 0;

        template <typename T>
        void handle(const char* name, T*& value) {
            if (handlerType() == HandlerType::Parser) {
                if (value == nullptr && matchesUninitialized(name)) {
                    value = new T();
                    handleDetail(name, value);
                }
            } else {
                if (value != nullptr) {
                    handleDetail(name, value);
                }
            }
        }

        template <typename T>
        void handle(const char* name, T& value) {
            handleDetail(name, &value);
        }
    };
}

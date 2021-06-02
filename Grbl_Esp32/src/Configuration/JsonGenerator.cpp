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

#include "JsonGenerator.h"

#include "Configurable.h"

#include <cstring>
#include <cstdio>
#include <atomic>

namespace Configuration {
    JsonGenerator::JsonGenerator(WebUI::JSONencoder& encoder) : _encoder(encoder) {
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);

        _currentPath[0] = '\0';
        _depth          = 0;
        _paths[0]       = _currentPath;
    }

    void JsonGenerator::enter(const char* name) {
        auto currentEnd = _paths[_depth];
        *currentEnd++   = '/';
        for (auto i = name; *i;) {
            Assert(currentEnd != _currentPath + 256, "Path out of bounds while serializing json.");
            *currentEnd++ = *i++;
        }
        ++_depth;
        _paths[_depth] = currentEnd;
        *currentEnd    = '\0';
    }

    void JsonGenerator::add(Configuration::Configurable* configurable) {
        if (configurable != nullptr) {
            configurable->handle(*this);
        }
    }

    void JsonGenerator::leave() {
        --_depth;
        Assert(_depth >= 0, "Depth out of bounds while serializing to json");
        *_paths[_depth] = '\0';
    }

    void JsonGenerator::handleDetail(const char* name, Configurable* value) {
        enter(name);
        value->handle(*this);
        leave();
    }

    void JsonGenerator::handle(const char* name, int& value, int32_t minValue, int32_t maxValue) {
        char buf[32];
        itoa(value, buf, 10);
        _encoder.begin_webui(name, _currentPath, "I", buf, minValue, maxValue);
        _encoder.end_object();
    }

    void JsonGenerator::handle(const char* name, double& value, double minValue, double maxValue) { 
        int n = int(value * 1000);
        handle(name, n, int(minValue * 1000), int(maxValue * 1000));
    }

    void JsonGenerator::handle(const char* name, StringRange& value, int minLength, int maxLength) {
        auto sv = value.str();
        _encoder.begin_webui(name, _currentPath, "S", sv.c_str(), minLength, maxLength);
        _encoder.end_object();
    }

    void JsonGenerator::handle(const char* name, Pin& value) {
        // Let's for now say these are strings.
        auto sv = value.name();
        _encoder.begin_webui(name, _currentPath, "S", sv.c_str(), 0, 255);
        _encoder.end_object();
    }

    void JsonGenerator::handle(const char* name, int& value, EnumItem* e) {
        const char* str = "unknown";
        for (; e->name; ++e) {
            if (value == e->value) {
                str = e->name;
                break;
            }
        }

        _encoder.begin_webui(name, _currentPath, "B", str);
        _encoder.begin_array("O");
        for (; e->name; ++e) {
            _encoder.begin_object();
            _encoder.member(e->name, e->value);
            _encoder.end_object();
        }
        _encoder.end_array();
        _encoder.end_object();
    }
}

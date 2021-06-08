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

#include "RuntimeSetting.h"

#include <cstdlib>
#include <atomic>

namespace Configuration {
    RuntimeSetting::RuntimeSetting(const char* runtimeSetting) : setting_(runtimeSetting + 1), start_(runtimeSetting + 1) {
        // Read fence for config. Shouldn't be necessary, but better safe than sorry.
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);
    }

    void RuntimeSetting::enterSection(const char* name, Configuration::Configurable* value) {
        if (is(name) && this->value() == nullptr) {
            auto previous = start_;

            // Figure out next node
            auto next = start_;
            for (; *next && *next != '=' && *next != '/'; ++next) {}

            // Do we have a child?
            if (*next == '/') {
                ++next;
                start_ = next;

                // Handle child:
                value->group(*this);
            }

            // Restore situation:
            start_ = previous;
        }
    }

    void RuntimeSetting::item(const char* name, int32_t& value, int32_t minValue, int32_t maxValue) {
        if (is(name) && this->value() != nullptr) {
            value = atoi(this->value());
        }
    }

    void RuntimeSetting::item(const char* name, float& value, float minValue, float maxValue) {
        if (is(name) && this->value() != nullptr) {
            char* floatEnd;
            value = strtof(this->value(), &floatEnd);
        }
    }

    void RuntimeSetting::item(const char* name, StringRange& value, int minLength, int maxLength) {
        if (is(name) && this->value() != nullptr) {
            value = this->value();
        }
    }

    void RuntimeSetting::item(const char* name, Pin& value) {
        if (is(name) && this->value() != nullptr) {
            auto parsed = Pin::create(StringRange(this->value()));
            value.swap(parsed);
        }
    }

    RuntimeSetting::~RuntimeSetting() {
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);  // Write fence for config
    }
}

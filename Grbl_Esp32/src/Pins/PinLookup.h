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

#include "../Assert.h"
#include "PinDetail.h"

#include <cstring>
#include <cstdint>

class Pin;

namespace Pins {
    class PinLookup {
    private:
        // There are 2^^8 instances max (uint8_t limits), so this is the complete set. We could
        // probably do with less, but this is as safe as it gets.
        PinDetail* _pins[256];

        // According to Arduino.h there are 40 GPIO pins. So, let's start at 41.
        static const int NumberNativePins = 41;

    public:
        PinLookup();

        static void ResetAllPins();

        uint8_t SetPin(uint8_t suggestedIndex, PinDetail* value) {
            int realIndex = -1;
            if (value->capabilities().has(Pins::PinCapabilities::Native)) {
                realIndex = suggestedIndex;
            } else {
                // Search for the first available pin index:
                int i;
                for (i = NumberNativePins; i <= 255 && _pins[i] != nullptr; ++i) {}
                if (i != 256) {
                    realIndex = i;
                }
            }

            // TODO FIXME: Check if this pin is not defined multiple times!

            // NOTE: 254 and 255 are reserved pins for 'undefined' and 'error'
            Assert(suggestedIndex >= 0 && suggestedIndex <= 253, "Pin index out of range.");
            Assert(_pins[realIndex] == nullptr, "Pin is defined multiple times.");

            _pins[realIndex] = value;

            return uint8_t(realIndex);
        }

        PinDetail* GetPin(uint8_t index) const {
            // This assertion shouldn't be hit. If it is, undefined behavior will follow.
            Assert(_pins[index] != nullptr, "Pin %d is not defined. Cannot use this pin.", index);
            return _pins[index];
        }

        int FindExisting(PinDetail* instance) const {
            // Checks if a pin with this number and capabilities already exists:
            for (int i = 0; i < 256; ++i) {
                if (_pins[i] != nullptr) {
                    if (_pins[i]->number() == instance->number() &&            // check number of pin
                        _pins[i]->capabilities() == instance->capabilities())  // check pin capabilities
                    {
                        return i;
                    }
                }
            }
            return -1;
        }

        PinLookup(const PinLookup&) = delete;
        PinLookup(PinLookup&&)      = delete;

        PinLookup& operator=(const PinLookup&) = delete;
        PinLookup& operator=(PinLookup&&) = delete;

        static PinLookup _instance;
    };
}

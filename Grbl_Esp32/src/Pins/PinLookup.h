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
            // This assertion causes message spewing making debugging impossible
            // Assert(_pins[index] != nullptr, "Pin is not defined. Cannot use this pin.");
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

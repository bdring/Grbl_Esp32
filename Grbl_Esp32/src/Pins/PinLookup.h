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

        // Should be plenty for the GPIO _pins:
        static const int NumberNativePins = 64;

    public:
        PinLookup() { memset(_pins, 0, sizeof(_pins)); }

        uint8_t SetPin(uint8_t suggestedIndex, PinDetail* value) {
            int realIndex = -1;
            if (value->traits().has(Pins::PinTraits::Native)) {
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
            Assert(_pins[index] != nullptr, "Pin is not defined. Cannot use this pin.");
            return _pins[index];
        }

        PinLookup(const PinLookup&) = delete;
        PinLookup(PinLookup&&)      = delete;

        PinLookup& operator=(const PinLookup&) = delete;
        PinLookup& operator=(PinLookup&&) = delete;

        static PinLookup _instance;
    };
}

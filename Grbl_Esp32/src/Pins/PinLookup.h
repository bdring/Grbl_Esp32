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
        PinDetail* pins[256];

    public:
        PinLookup() { memset(pins, 0, sizeof(pins)); }

        void SetPin(uint8_t index, PinDetail* value) {
            // TODO FIXME: Map GPIO pins to [index], map all other pins to the first available slot (>= 32)
            // TODO FIXME: Check if this pin is not already mapped to something else.

            Assert(pins[index] == nullptr, "Pin is defined multiple times.");
            pins[index] = value;
        }

        PinDetail* GetPin(uint8_t index) const {
            Assert(pins[index] != nullptr, "Pin is not defined. Cannot use this pin.");
            return pins[index];
        }

        PinLookup(const PinLookup&) = delete;
        PinLookup(PinLookup&&)      = delete;

        PinLookup& operator=(const PinLookup&) = delete;
        PinLookup& operator=(PinLookup&&) = delete;

        static PinLookup _instance;
    };
}

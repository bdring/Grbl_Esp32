#pragma once

#include <cstdint>

namespace Pins {
    class PinCapabilities {
        uint32_t _value;

        constexpr PinCapabilities(const uint32_t value) : _value(value) {}

    public:
        PinCapabilities(const PinCapabilities&) = default;
        PinCapabilities(PinCapabilities&)       = default;

        PinCapabilities& operator=(const PinCapabilities&) = default;
        PinCapabilities& operator=(PinCapabilities&) = default;

        // All the traits we use and test:
        static PinCapabilities None;
        static PinCapabilities Native;
        static PinCapabilities Input;
        static PinCapabilities Output;
        static PinCapabilities PullUp;
        static PinCapabilities PullDown;
        static PinCapabilities PWM;
        static PinCapabilities ISR;
        static PinCapabilities I2S;

        inline PinCapabilities operator|(PinCapabilities rhs) { return PinCapabilities(_value | rhs._value); }
        inline PinCapabilities operator&(PinCapabilities rhs) { return PinCapabilities(_value & rhs._value); }
        inline bool      operator==(PinCapabilities rhs) const { return _value == rhs._value; }
        inline bool      operator!=(PinCapabilities rhs) const { return _value != rhs._value; }

        inline operator bool() { return _value != 0; }

        inline bool has(PinCapabilities t) { return (*this & t).operator bool(); }
    };
}

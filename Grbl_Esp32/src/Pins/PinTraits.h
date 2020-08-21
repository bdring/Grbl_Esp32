#pragma once

#include <cstdint>

namespace Pins {
    class PinTraits {
        uint32_t _value;

        constexpr PinTraits(const uint32_t value) : _value(value) {}

    public:
        PinTraits(const PinTraits&) = default;
        PinTraits(PinTraits&)       = default;

        PinTraits& operator=(const PinTraits&) = default;
        PinTraits& operator=(PinTraits&) = default;

        // All the traits we use and test:
        static PinTraits None;
        static PinTraits Native;
        static PinTraits Input;
        static PinTraits Output;
        static PinTraits PullUp;
        static PinTraits PullDown;
        static PinTraits PWM;

        inline PinTraits operator|(PinTraits rhs) { return PinTraits(_value | rhs._value); }
        inline PinTraits operator&(PinTraits rhs) { return PinTraits(_value & rhs._value); }

        inline operator bool() { return _value != 0; }

        inline bool has(PinTraits t) { return (*this & t).operator bool(); }
    };
}

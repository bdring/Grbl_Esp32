#pragma once

#include <cstdint>

namespace Pins {
    class PinAttributes;
    class PinCapabilities {
        uint32_t _value;

        constexpr PinCapabilities(const uint32_t value) : _value(value) {}

        friend class PinAttributes;  // Wants access to _value for validation

    public:
        PinCapabilities(const PinCapabilities&) = default;
        PinCapabilities(PinCapabilities&)       = default;

        PinCapabilities& operator=(const PinCapabilities&) = default;
        PinCapabilities& operator=(PinCapabilities&) = default;

        // All the capabilities we use and test:
        static PinCapabilities None;

        static PinCapabilities Input;     // NOTE: Mapped in PinAttributes!
        static PinCapabilities Output;    // NOTE: Mapped in PinAttributes!
        static PinCapabilities PullUp;    // NOTE: Mapped in PinAttributes!
        static PinCapabilities PullDown;  // NOTE: Mapped in PinAttributes!

        static PinCapabilities ActiveLow;
        static PinCapabilities Native;
        static PinCapabilities ADC;
        static PinCapabilities DAC;
        static PinCapabilities PWM;
        static PinCapabilities ISR;
        static PinCapabilities I2S;

        inline PinCapabilities operator|(PinCapabilities rhs) { return PinCapabilities(_value | rhs._value); }
        inline PinCapabilities operator&(PinCapabilities rhs) { return PinCapabilities(_value & rhs._value); }
        inline bool            operator==(PinCapabilities rhs) const { return _value == rhs._value; }
        inline bool            operator!=(PinCapabilities rhs) const { return _value != rhs._value; }

        inline operator bool() { return _value != 0; }

        inline bool has(PinCapabilities t) { return (*this & t).operator bool(); }
    };
}

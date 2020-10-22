#pragma once

#include <cstdint>

namespace Pins {
    class PinAttributes;

    /*
    Pin capabilities are what a pin _can_ do using the internal hardware. For GPIO pins, these
    are the internal hardware capabilities of the pins, such as the capability to pull-up from
    hardware, wether or not a pin supports input/output, etc.
    */
    class PinCapabilities {
        uint32_t _value;

        constexpr PinCapabilities(const uint32_t value) : _value(value) {}

        friend class PinAttributes;  // Wants access to _value for validation

    public:
        PinCapabilities(const PinCapabilities&) = default;
        PinCapabilities& operator=(const PinCapabilities&) = default;

        // All the capabilities we use and test:
        static PinCapabilities None;

        static PinCapabilities Input;     // NOTE: Mapped in PinAttributes!
        static PinCapabilities Output;    // NOTE: Mapped in PinAttributes!
        static PinCapabilities PullUp;    // NOTE: Mapped in PinAttributes!
        static PinCapabilities PullDown;  // NOTE: Mapped in PinAttributes!
        static PinCapabilities ISR;       // NOTE: Mapped in PinAttributes!

        static PinCapabilities Native;
        static PinCapabilities ADC;
        static PinCapabilities DAC;
        static PinCapabilities PWM;
        static PinCapabilities UART;
        static PinCapabilities I2S;

        inline PinCapabilities operator|(PinCapabilities rhs) { return PinCapabilities(_value | rhs._value); }
        inline PinCapabilities operator&(PinCapabilities rhs) { return PinCapabilities(_value & rhs._value); }
        inline bool            operator==(PinCapabilities rhs) const { return _value == rhs._value; }
        inline bool            operator!=(PinCapabilities rhs) const { return _value != rhs._value; }

        inline operator bool() { return _value != 0; }

        inline bool has(PinCapabilities t) { return (*this & t).operator bool(); }
    };
}

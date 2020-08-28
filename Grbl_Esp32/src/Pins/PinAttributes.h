#pragma once

#include <cstdint>

#include "PinCapabilities.h"

namespace Pins {
    class PinAttributes {
        uint32_t _value;
        
        constexpr PinAttributes(const uint32_t value) : _value(value) {}

    public:
        PinAttributes(const PinAttributes&) = default;
        PinAttributes(PinAttributes&)       = default;

        PinAttributes& operator=(const PinAttributes&) = default;
        PinAttributes& operator=(PinAttributes&) = default;

        // All the capabilities we use and test:
        static PinAttributes Undefined;
        static PinAttributes None;
        static PinAttributes Input;
        static PinAttributes Output;
        static PinAttributes PullUp;
        static PinAttributes PullDown;
        static PinAttributes NonExclusive;

        inline PinAttributes operator|(PinAttributes rhs) { return PinAttributes(_value | rhs._value); }
        inline PinAttributes operator&(PinAttributes rhs) { return PinAttributes(_value & rhs._value); }
        inline bool          operator==(PinAttributes rhs) const { return _value == rhs._value; }
        inline bool          operator!=(PinAttributes rhs) const { return _value != rhs._value; }

        inline operator bool() { return _value != 0; }

        bool conflictsWith(PinAttributes t);
        bool validateWith(PinCapabilities caps);
        inline bool has(PinAttributes attr) { return ((*this) & attr).operator bool(); }
    };
}

#pragma once

#include "Pins/PinLookup.h"
#include "Pins/PinDetail.h"

#include <cstdint>
#include <cstring>

class String;

class Pin {
    uint8_t _index;

    inline Pin(uint8_t index) : _index(index) {}

public:
    inline Pin(const Pin& o) = default;
    inline Pin(Pin&& o)      = default;

    inline Pin& operator=(const Pin& o) = default;
    inline Pin& operator=(Pin&& o) = default;

    inline void write(bool high) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->write(high);
    }

    inline int read() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->read();
    }

    inline void setMode(uint8_t value) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->mode(value);
    }

    inline bool initPWM(uint32_t frequency, uint32_t maxDuty) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        detail->initPWM(frequency, maxDuty);
    }

    // Returns actual frequency which might not be exactly the same as requested(nearest supported value)
    inline uint32_t getPWMFrequency() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->getPWMFrequency();
    }

    // Returns actual maxDuty which might not be exactly the same as requested(nearest supported value)
    inline uint32_t getPWMMaxDuty() const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->getPWMMaxDuty();
    }

    inline void setPWMDuty(uint32_t duty) const {
        auto detail = Pins::PinLookup::_instance.GetPin(_index);
        return detail->setPWMDuty(duty);
    }

    inline ~Pin() = default;

    static Pin Create(String str);
};

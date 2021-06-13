#include "PinMapper.h"

#include "Assert.h"

#include <Arduino.h>

// Pin mapping. Pretty straight forward, it's just a thing that

namespace {
    class Mapping {
    public:
        Pin* _mapping[256];

        Mapping() {
            for (int i = 0; i < 256; ++i) {
                _mapping[i] = nullptr;
            }
        }

        uint8_t Claim(Pin* pin) {
            // Let's not use 0. 1 is the first pin we'll allocate.
            for (int i = 1; i < 256; ++i) {
                if (_mapping[i] == nullptr) {
                    _mapping[i] = pin;
                    return i;
                }
            }
            return 0;
        }

        void Release(uint8_t idx) { _mapping[idx] = nullptr; }

        static Mapping& instance() {
            static Mapping instance;
            return instance;
        }
    };
}

PinMapper::PinMapper() : _mappedId(0) {}

PinMapper::PinMapper(Pin& pin) {
    _mappedId = Mapping::instance().Claim(&pin);
    Assert(_mappedId != 0, "Cannot claim pin. We've reached the limit of 255 mapped pins.");
}

PinMapper::PinMapper(PinMapper&& o) : _mappedId(0) {
    std::swap(_mappedId, o._mappedId);
}

PinMapper& PinMapper::operator=(PinMapper&& o) {
    if (&o != this) {
        if (_mappedId != 0) {
            Mapping::instance().Release(_mappedId);
            _mappedId = 0;
        }
        std::swap(_mappedId, o._mappedId);
    }
    return *this;
}

PinMapper::~PinMapper() {
    if (_mappedId != 0) {
        Mapping::instance().Release(_mappedId);
    }
}

// Arduino compatibility functions, which basically forward the call to the mapper:

void IRAM_ATTR digitalWrite(uint8_t pin, uint8_t val) {
    auto thePin = Mapping::instance()._mapping[pin];
    if (thePin) {
        thePin->write(val);
    }
}

void IRAM_ATTR pinMode(uint8_t pin, uint8_t mode) {
    Pins::PinAttributes attr = Pins::PinAttributes::None;
    if ((mode & OUTPUT) == OUTPUT) {
        attr = attr | Pins::PinAttributes::Output;
    }
    if ((mode & INPUT) == INPUT) {
        attr = attr | Pins::PinAttributes::Input;
    }
    if ((mode & PULLUP) == PULLUP) {
        attr = attr | Pins::PinAttributes::PullUp;
    }
    if ((mode & PULLDOWN) == PULLDOWN) {
        attr = attr | Pins::PinAttributes::PullDown;
    }

    auto thePin = Mapping::instance()._mapping[pin];
    if (thePin) {
        thePin->setAttr(attr);
    }
}

int IRAM_ATTR digitalRead(uint8_t pin) {
    auto thePin = Mapping::instance()._mapping[pin];
    return (thePin) ? thePin->read() : 0;
}

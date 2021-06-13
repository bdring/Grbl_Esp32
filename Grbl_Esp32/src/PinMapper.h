#pragma once

// Pin mapper is a class that maps 'Pin' objects to Arduino digitalWrite / digitalRead / setMode. This
// can be useful for support of external libraries, while keeping all the things that Pin has to offer.

#include "Pin.h"

class PinMapper {
    uint8_t _mappedId;

public:
    PinMapper();
    PinMapper(Pin& pin);

    PinMapper(const Pin& o) = delete;
    PinMapper& operator=(const Pin& o) = delete;

    PinMapper(PinMapper&& o);
    PinMapper& operator=(PinMapper&& o);

    inline uint8_t pinId() const { return _mappedId; }

    ~PinMapper();
};

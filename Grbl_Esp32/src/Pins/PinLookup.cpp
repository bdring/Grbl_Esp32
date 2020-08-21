#include "PinLookup.h"

#include <Arduino.h>

namespace Pins {
    PinLookup PinLookup::_instance;
}

String pinName(uint8_t pin) {
    return Pins::PinLookup::_instance.GetPin(pin)->toString();
}

void IRAM_ATTR digitalWrite(uint8_t pin, uint8_t val) {
    Pins::PinLookup::_instance.GetPin(pin)->write(val);
}

void IRAM_ATTR pinMode(uint8_t pin, uint8_t mode) {
    Pins::PinLookup::_instance.GetPin(pin)->mode(mode);
}

int IRAM_ATTR digitalRead(uint8_t pin) {
    return Pins::PinLookup::_instance.GetPin(pin)->read();
}

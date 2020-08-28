#include "PinLookup.h"
#include "PinAttributes.h"

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

    Pins::PinAttributes attr = Pins::PinAttributes::None;
    if (mode & OUTPUT) { attr = attr | Pins::PinAttributes::Output; }
    if (mode & INPUT) { attr = attr | Pins::PinAttributes::Input; }
    if (mode & PULLUP) { attr = attr | Pins::PinAttributes::PullUp; }
    if (mode & PULLDOWN) { attr = attr | Pins::PinAttributes::PullDown; }

    Pins::PinLookup::_instance.GetPin(pin)->setAttr(attr);
}

int IRAM_ATTR digitalRead(uint8_t pin) {
    return Pins::PinLookup::_instance.GetPin(pin)->read();
}

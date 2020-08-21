#include <Arduino.h>

#include "GPIOPinDetail.h"

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

// TODO FIXME: ISR, PWM, etc

namespace Pins {
    GPIOPinDetail::GPIOPinDetail(uint8_t index, const PinOptionsParser& options) : _index(index) {
        // TODO: Handle options...
    }

    PinTraits GPIOPinDetail::traits() const {
        // TODO FIXME: Depending on the pin info, we might want to add information like
        // internal pullups.

        return PinTraits::Native | PinTraits::Input | PinTraits::Output | PinTraits::PWM | PinTraits::ISR;
    }

    void GPIOPinDetail::write(bool high) { __digitalWrite(_index, high); }
    int  GPIOPinDetail::read() { return __digitalRead(_index); }
    void GPIOPinDetail::mode(uint8_t value) { __pinMode(_index, value); }

    void GPIOPinDetail::attachInterrupt(void (*callback)(void*), void* arg, int mode) { ::attachInterruptArg(_index, callback, arg, mode); }
    void GPIOPinDetail::detachInterrupt() { ::detachInterrupt(_index); }

    bool GPIOPinDetail::initPWM(uint32_t frequency, uint32_t maxDuty) {
        // TODO FIXME: Implement
        throw "Notimplemented";
    }

    // Returns actual frequency which might not be exactly the same as requested(nearest supported value)
    uint32_t GPIOPinDetail::getPWMFrequency() {
        // TODO FIXME: Implement
        throw "Notimplemented";
    }

    // Returns actual maxDuty which might not be exactly the same as requested(nearest supported value)
    uint32_t GPIOPinDetail::getPWMMaxDuty() {
        // TODO FIXME: Implement
        throw "Notimplemented";
    }

    void GPIOPinDetail::setPWMDuty(uint32_t duty) {
        // TODO FIXME: Implement
        throw "Notimplemented";
    }

    String GPIOPinDetail::toString() const { return "GPIO." + int(_index); }

}

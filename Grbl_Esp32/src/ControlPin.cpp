#include "ControlPin.h"

#include "Report.h"   // addPinReport
#include <Arduino.h>  // IRAM_ATTR

// XXX we need to dispatch the user defined macros somehow
// user_defined_macro(N)

void IRAM_ATTR ControlPin::handleISR() {
    bool pinState = _pin.read();
    _value        = pinState;
    _rtVariable   = pinState;
}

void ControlPin::init() {
    if (_pin.defined()) {
        _pin.report(_legend);
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (_pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        _pin.setAttr(attr);
        _pin.attachInterrupt<ControlPin, &ControlPin::handleISR>(this, CHANGE);
    }
}

void ControlPin::report(char* status) {
    if (!_pin.undefined()) {
        addPinReport(status, _letter);
    }
}

ControlPin::~ControlPin() {
    _pin.detachInterrupt();
}

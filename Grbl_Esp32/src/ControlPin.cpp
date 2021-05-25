#include "Grbl.h"

#include "ControlPin.h"

// XXX we need to dispatch the user defined macros somehow
// user_defined_macro(N)

void IRAM_ATTR ControlPin::handleISR() {
    bool pinState = _pin.read();
    _value        = pinState;

    if (_rtVariable) {
        *_rtVariable = pinState;
    }
}

void ControlPin::init() {
    if (_pin.defined()) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s switch on pin %s", _legend, _pin.name());
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (_pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        _pin.setAttr(attr);
        _pin.attachInterrupt<ControlPin, &ControlPin::handleISR>(this, CHANGE);
    }
}

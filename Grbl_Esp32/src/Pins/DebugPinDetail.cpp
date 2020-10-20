#include "DebugPinDetail.h"

#include "../Grbl.h"  // for printf
#include <Arduino.h>  // for timer

namespace Pins {
    // I/O:
    void DebugPinDetail::write(int high) {
        grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Writing pin %s = %d", toString().c_str(), high);
        _implementation->write(high);
    }

    int DebugPinDetail::read() {
        auto result = _implementation->read();
        grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Reading pin %s = %d", toString().c_str(), result);
        return result;
    }
    void DebugPinDetail::setAttr(PinAttributes value) {
        char buf[10];
        int  n = 0;
        if (value.has(PinAttributes::Input)) {
            buf[n++] = 'I';
        }
        if (value.has(PinAttributes::Output)) {
            buf[n++] = 'O';
        }
        if (value.has(PinAttributes::PullUp)) {
            buf[n++] = 'U';
        }
        if (value.has(PinAttributes::PullDown)) {
            buf[n++] = 'D';
        }
        if (value.has(PinAttributes::ISR)) {
            buf[n++] = 'E';
        }
        if (value.has(PinAttributes::Exclusive)) {
            buf[n++] = 'X';
        }
        if (value.has(PinAttributes::InitialHigh)) {
            buf[n++] = '+';
        }
        buf[n++] = 0;

        if (shouldEvent()) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Setting pin attr %s = %s", toString().c_str(), buf);
        }
        _implementation->setAttr(value);
    }

    void DebugPinDetail::CallbackHandler::handle(void* arg) {
        auto handler = static_cast<CallbackHandler*>(arg);
        if (handler->_myPin->shouldEvent()) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Received ISR on pin %s", handler->_myPin->toString().c_str());
        }
        handler->callback(handler->argument);
    }

    // ISR's:
    void DebugPinDetail::attachInterrupt(void (*callback)(void*), void* arg, int mode) {
        _isrHandler._myPin   = this;
        _isrHandler.argument = arg;
        _isrHandler.callback = callback;

        if (shouldEvent()) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Attaching interrupt to pin %s, mode %d", toString().c_str(), mode);
        }
        _implementation->attachInterrupt(_isrHandler.handle, &_isrHandler, mode);
    }
    void DebugPinDetail::detachInterrupt() { _implementation->detachInterrupt(); }

    bool DebugPinDetail::shouldEvent() {
        // This method basically ensures we don't flood users:
        auto time = millis();

        if (_lastEvent + 1000 > time) {
            _lastEvent  = time;
            _eventCount = 1;
            return true;
        } else if (_eventCount < 20) {
            _lastEvent = time;
            ++_eventCount;
            return true;
        } else {
            return false;
        }
    }
}

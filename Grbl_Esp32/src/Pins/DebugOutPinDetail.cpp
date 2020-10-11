#include "DebugOutPinDetail.h"
#include "../Assert.h"

#ifdef ESP32
#include "../Grbl.h"
#define DEBUGOUT(format, ...) grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, format "\r\n", __VA_ARGS__)
#else
#include <cstdio>
#define DEBUGOUT(format, ...) printf(format "\r\n", __VA_ARGS__)
#pragma warning(disable: 4996)
#endif


namespace Pins {
    DebugOutPinDetail::DebugOutPinDetail(uint8_t index, PinOptionsParser options) :
        _index(index), _capabilities(capabilities()), _attributes(Pins::PinAttributes::Undefined), _readWriteMask(0) {
        for (auto option : options) {
            if (option.is("name")) {
                _name = option();
            } else if (option.is("low")) {
                _capabilities = _capabilities | PinCapabilities::ActiveLow;
            } else if (option.is("high")) {
                // Default: Active HIGH.
            }
        }

        if (_name == "") {
            char buf[11];
            itoa(int(_index), buf, 10);
            _name = "Debug_" + String(buf);
        }
    }

    PinCapabilities DebugOutPinDetail::capabilities() const {
        return PinCapabilities::Output | PinCapabilities::DAC | PinCapabilities::PWM;
    }

    // I/O:
    void DebugOutPinDetail::write(int high) {
        int value = _readWriteMask ^ high;
        DEBUGOUT("[MSG: pin write(%d, %d)]", _index, value);
    }

    int DebugOutPinDetail::read() { return 0; }

    void DebugOutPinDetail::setAttr(PinAttributes value) {
        // Check the attributes first:
        Assert(!value.validateWith(this->_capabilities), "The requested attributes don't match the pin capabilities");
        Assert(!_attributes.conflictsWith(value), "Attributes on this pin have been set before, and there's a conflict.");

        _attributes = value;
    }

    // PWM
    bool DebugOutPinDetail::initPWM(uint32_t frequency, uint32_t maxDuty) {
        DEBUGOUT("[MSG: pin init pwm(frequency=%d, maxDuty=%d)]", frequency, maxDuty);
        _frequency = frequency;
        _maxDuty   = maxDuty;
        return true;
    }

    uint32_t DebugOutPinDetail::getPWMFrequency() { return _frequency; }

    uint32_t DebugOutPinDetail::getPWMMaxDuty() { return _maxDuty; }

    void DebugOutPinDetail::setPWMDuty(uint32_t duty) {
        DEBUGOUT("[MSG: pin set pwm duty(duty=%d)]", duty);
    }

    String DebugOutPinDetail::toString() const { return "DebugPin." + int(_index); }
}

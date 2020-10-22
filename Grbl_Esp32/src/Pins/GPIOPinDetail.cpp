#include <Arduino.h>
#include <stdexcept>

#include "GPIOPinDetail.h"
#include "../Assert.h"

extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

namespace Pins {
    PinCapabilities GPIOPinDetail::GetDefaultCapabilities(uint8_t index) {
        // See https://randomnerdtutorials.com/esp32-pinout-reference-gpios/ for an overview:
        switch (index) {
            case 0:  // Outputs PWM signal at boot
                return PinCapabilities::Native | PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::PullUp |
                       PinCapabilities::PullDown | PinCapabilities::ADC | PinCapabilities::PWM | PinCapabilities::ISR |
                       PinCapabilities::UART;

            case 1:  // TX pin of Serial0. Note that Serial0 also runs through the Pins framework!
                return PinCapabilities::Native | PinCapabilities::Output | PinCapabilities::Input | PinCapabilities::UART;

            case 3:  // RX pin of Serial0. Note that Serial0 also runs through the Pins framework!
                return PinCapabilities::Native | PinCapabilities::Output | PinCapabilities::Input | PinCapabilities::ISR |
                       PinCapabilities::UART;

            case 5:
            case 16:
            case 17:
            case 18:
            case 19:
            case 21:
            case 22:
            case 23:
            case 29:
                return PinCapabilities::Native | PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::PullUp |
                       PinCapabilities::PullDown | PinCapabilities::PWM | PinCapabilities::ISR | PinCapabilities::UART;

            case 2:  // Normal pins
            case 4:
            case 12:  // Boot fail if pulled high
            case 13:
            case 14:  // Outputs PWM signal at boot
            case 15:  // Outputs PWM signal at boot
            case 27:
            case 32:
            case 33:
                return PinCapabilities::Native | PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::PullUp |
                       PinCapabilities::PullDown | PinCapabilities::ADC | PinCapabilities::PWM | PinCapabilities::ISR |
                       PinCapabilities::UART;

            case 25:
            case 26:
                return PinCapabilities::Native | PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::PullUp |
                       PinCapabilities::PullDown | PinCapabilities::ADC | PinCapabilities::DAC | PinCapabilities::PWM |
                       PinCapabilities::ISR | PinCapabilities::UART;

            case 6:  // SPI flash integrated
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
                return PinCapabilities::Native | PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::PWM |
                       PinCapabilities::ISR | PinCapabilities::UART;

            case 34:  // Input only pins
            case 35:
            case 36:
            case 39:
                return PinCapabilities::Native | PinCapabilities::Input | PinCapabilities::ADC | PinCapabilities::ISR | PinCapabilities::UART;
                break;

            default:  // Not mapped to actual GPIO pins
                return PinCapabilities::None;
        }
    }

    GPIOPinDetail::GPIOPinDetail(uint8_t index, PinOptionsParser options) :
        _index(index), _capabilities(GetDefaultCapabilities(index)), _attributes(Pins::PinAttributes::Undefined), _readWriteMask(0) {
        if (_capabilities == PinCapabilities::None) {
            throw std::runtime_error("Bad GPIO number");
        }
        // User defined pin capabilities
        for (auto opt : options) {
            if (opt.is("pu")) {
                _attributes = _attributes | PinAttributes::PullUp;
            } else if (opt.is("pd")) {
                _attributes = _attributes | PinAttributes::PullDown;
            } else if (opt.is("low")) {
                _attributes = _attributes  | PinAttributes::ActiveLow;
            } else if (opt.is("high")) {
                // Default: Active HIGH.
            } else {
                throw std::runtime_error("Bad GPIO option");
            }
        }

        // XXX This should not be done when the constructor is called for
        // validating a setting.
        // Update the R/W mask for ActiveLow setting
        if (_attributes.has(PinAttributes::ActiveLow)) {
            __pinMode(_index, OUTPUT);
            __digitalWrite(_index, HIGH);
            _readWriteMask = HIGH;
        } else {
            _readWriteMask = LOW;
        }
    }

    PinCapabilities GPIOPinDetail::capabilities() const { return _capabilities; }

    void GPIOPinDetail::write(int high) {
        Assert(_attributes.has(PinAttributes::Output), "Pin has no output attribute defined. Cannot write to it.");
        int value = _readWriteMask ^ high;
        __digitalWrite(_index, value);
    }
    int GPIOPinDetail::read() {
        auto raw = __digitalRead(_index);
        return raw ^ _readWriteMask;
    }

    void GPIOPinDetail::setAttr(PinAttributes value) {
        // These two assertions will fail if we do them for index 1/3 (Serial uart). This is because
        // they are initialized by HardwareSerial well before we start our main operations. Best to
        // just ignore them for now, and figure this out later. TODO FIXME!

        // Check the attributes first:
        Assert(value.validateWith(this->_capabilities) || _index == 1 || _index == 3,
               "The requested attributes don't match the pin capabilities");
        Assert(!_attributes.conflictsWith(value) || _index == 1 || _index == 3,
               "Attributes on this pin have been set before, and there's a conflict.");

        _attributes = value;

        // Handle attributes:
        uint8_t pinModeValue = 0;

        if (value.has(PinAttributes::PullUp)) {
            pinModeValue |= INPUT_PULLUP;
        } else if (value.has(PinAttributes::PullDown)) {
            pinModeValue |= INPUT_PULLDOWN;
        } else if (value.has(PinAttributes::Input)) {
            pinModeValue |= INPUT;
        } else if (value.has(PinAttributes::Output)) {
            pinModeValue |= OUTPUT;
        }

        // TODO: If the pin is ActiveLow, should we take this into account or not?
        if (value.has(PinAttributes::InitialHigh)) {
            __digitalWrite(_index, HIGH);
        }

        __pinMode(_index, pinModeValue);
    }

    void GPIOPinDetail::attachInterrupt(void (*callback)(void*), void* arg, int mode) {
        Assert(_attributes.has(PinAttributes::ISR), "Pin has no ISR attribute defined. Cannot bind ISR.");
        ::attachInterruptArg(_index, callback, arg, mode);
    }

    void GPIOPinDetail::detachInterrupt() {
        Assert(_attributes.has(PinAttributes::ISR), "Pin has no ISR attribute defined. Cannot unbind ISR.");
        ::detachInterrupt(_index);
    }

    String GPIOPinDetail::toString() const { return String("GPIO.") + int(_index); }
}

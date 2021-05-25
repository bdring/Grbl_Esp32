/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

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
        PinDetail(index), _capabilities(GetDefaultCapabilities(index)), _attributes(Pins::PinAttributes::Undefined), _readWriteMask(0) {
        // NOTE:
        //
        // RAII is very important here! If we throw an exception in the constructor, the resources
        // that were allocated by the constructor up to that point _MUST_ be freed! Otherwise, you
        // WILL get into trouble.

        Assert(_capabilities != PinCapabilities::None, "Bad GPIO number");

        // User defined pin capabilities
        for (auto opt : options) {
            if (opt.is("pu")) {
                _attributes = _attributes | PinAttributes::PullUp;
            } else if (opt.is("pd")) {
                _attributes = _attributes | PinAttributes::PullDown;
            } else if (opt.is("low")) {
                _attributes = _attributes | PinAttributes::ActiveLow;
            } else if (opt.is("high")) {
                // Default: Active HIGH.
            } else {
                Assert(false, "Bad GPIO option passed to pin %d: %s", int(index), opt());
            }
        }

        // Update the R/W mask for ActiveLow setting
        if (_attributes.has(PinAttributes::ActiveLow)) {
            _readWriteMask = HIGH;
        } else {
            _readWriteMask = LOW;
        }
    }

    PinAttributes GPIOPinDetail::getAttr() const { return _attributes; }

    PinCapabilities GPIOPinDetail::capabilities() const { return _capabilities; }

    void GPIOPinDetail::write(int high) {
        Assert(_attributes.has(PinAttributes::Output), "Pin has no output attribute defined. Cannot write to it.");
        int value = _readWriteMask ^ high;
        __digitalWrite(_index, value);
    }
    int IRAM_ATTR GPIOPinDetail::read() {
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

        if (value.has(PinAttributes::Input)) {
            pinModeValue |= INPUT;
        } else if (value.has(PinAttributes::Output)) {
            pinModeValue |= OUTPUT;
        }

        // PU/PD should be specified by the user. Code has nothing to do with them:
        if (_attributes.has(PinAttributes::PullUp)) {
            pinModeValue |= PULLUP;
        } else if (_attributes.has(PinAttributes::PullDown)) {
            pinModeValue |= PULLDOWN;
        }

        // If the pin is ActiveLow, we should take that into account here:
        if (value.has(PinAttributes::Output)) {
            if (value.has(PinAttributes::InitialOn)) {
                __digitalWrite(_index, HIGH ^ _readWriteMask);
            } else {
                __digitalWrite(_index, LOW ^ _readWriteMask);
            }
        }

        __pinMode(_index, pinModeValue);
    }

    void GPIOPinDetail::attachInterrupt(void (*callback)(void*), void* arg, int mode) {
        Assert(_attributes.has(PinAttributes::ISR),
               "Pin has no ISR attribute, which means 'setAttr' was not set, or the pin doesn't support ISR's. Cannot bind ISR.");
        ::attachInterruptArg(_index, callback, arg, mode);
    }

    void GPIOPinDetail::detachInterrupt() {
        Assert(_attributes.has(PinAttributes::ISR),
               "Pin has no ISR attribute, which means 'setAttr' was not set, or the pin doesn't support ISR's. Cannot unbind ISR.");
        ::detachInterrupt(_index);
    }

    String GPIOPinDetail::toString() const { return String("GPIO.") + int(_index); }
}

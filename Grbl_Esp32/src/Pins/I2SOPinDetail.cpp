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

#ifdef ESP32
#    include "I2SOPinDetail.h"

#    include "../I2SOut.h"
#    include "../Assert.h"

extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

namespace Pins {
    I2SOPinDetail::I2SOPinDetail(uint8_t index, const PinOptionsParser& options) :
        PinDetail(index), _capabilities(PinCapabilities::Output | PinCapabilities::I2S), _attributes(Pins::PinAttributes::Undefined),
        _readWriteMask(0) {
        // User defined pin capabilities
        for (auto opt : options) {
            if (opt.is("low")) {
                _attributes = _attributes | PinAttributes::ActiveLow;
            } else if (opt.is("high")) {
                // Default: Active HIGH.
            } else {
                Assert(false, "Bad I2S option passed to pin %d: %s", int(index), opt());
            }
        }

        // readWriteMask is xor'ed with the value to invert it if active low
        _readWriteMask = _attributes.has(PinAttributes::ActiveLow);
    }

    PinCapabilities I2SOPinDetail::capabilities() const { return PinCapabilities::Output | PinCapabilities::I2S; }

    void I2SOPinDetail::write(int high) {
        Assert(_attributes.has(PinAttributes::Output), "Pin has no output attribute defined. Cannot write to it.");
        int value = _readWriteMask ^ high;
        i2s_out_write(_index, value);
        // XXX Do we want to add i2s_out_delay() here ?
        // Doing so would eliminate the need to override
        // switchCSpin() in TrnamicDriver.cpp
        i2s_out_delay();
    }

    int I2SOPinDetail::read() {
        auto raw = i2s_out_read(_index);
        return raw ^ _readWriteMask;
    }

    void I2SOPinDetail::setAttr(PinAttributes value) {
        // Check the attributes first:
        Assert(value.validateWith(this->_capabilities), "The requested attributes don't match the pin capabilities");
        Assert(!_attributes.conflictsWith(value), "Attributes on this pin have been set before, and there's a conflict.");

        _attributes = value;

        // I2S out pins cannot be configured, hence there
        // is nothing to do here for them. We basically
        // just check for conflicts above...

        // If the pin is ActiveLow, we should take that into account here:
        i2s_out_write(_index, value.has(PinAttributes::InitialOn) ^ _readWriteMask);
    }

    PinAttributes I2SOPinDetail::getAttr() const { return _attributes; }

    String I2SOPinDetail::toString() {
        auto s = String("I2SO.") + int(_index);
        if (_attributes.has(PinAttributes::ActiveLow)) {
            s += ":low";
        }
        return s;
    }
}

#endif

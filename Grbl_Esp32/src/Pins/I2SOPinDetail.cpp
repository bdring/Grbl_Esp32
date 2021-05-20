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

        // Update the R/W mask for ActiveLow setting
        if (_attributes.has(PinAttributes::ActiveLow)) {
            _readWriteMask = HIGH;
        } else {
            _readWriteMask = LOW;
        }
    }

    PinCapabilities I2SOPinDetail::capabilities() const { return PinCapabilities::Output | PinCapabilities::I2S; }

    void I2SOPinDetail::write(int high) {
        Assert(_attributes.has(PinAttributes::Output), "Pin has no output attribute defined. Cannot write to it.");
        int value = _readWriteMask ^ high;
        i2s_out_write(_index, value);
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
        if (value.has(PinAttributes::InitialOn)) {
            i2s_out_write(_index, HIGH ^ _readWriteMask);
        } else {
            i2s_out_write(_index, LOW ^ _readWriteMask);
        }
    }

    PinAttributes I2SOPinDetail::getAttr() const { return _attributes; }

    String I2SOPinDetail::toString() const { return String("I2SO.") + int(_index); }
}

#endif

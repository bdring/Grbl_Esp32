#include "I2SPinDetail.h"

#include "../I2SOut.h"

#ifdef USE_I2S_OUT

namespace Pins {
    I2SPinDetail::I2SPinDetail(uint8_t index, const PinOptionsParser& options) : _index(index) {}

    PinCapabilities I2SPinDetail::capabilities() const { return PinCapabilities::Input | PinCapabilities::Output | PinCapabilities::I2S; }

    void I2SPinDetail::write(bool high) { i2s_out_write(pin, val); }
    int  I2SPinDetail::read() { return i2s_out_read(pin); }
    void I2SPinDetail::mode(uint8_t value) {
        // I2S out pins cannot be configured, hence there
        // is nothing to do here for them.
    }

    String I2SPinDetail::toString() { return "I2S_" + int(_index); }
}

#endif

#include "I2SPinDetail.h"

#include "../I2SOut.h"

#ifdef USE_I2S_OUT

namespace Pins {
    I2SPinDetail::I2SPinDetail(uint8_t index, const String& options) : _index(index) {}

    // GPIO_NUM_3 maps to 3, so we can simply do it like this:
    void I2SPinDetail::write(bool high) { i2s_out_write(pin, val); }
    int  I2SPinDetail::read() { return i2s_out_state(pin); }
    void I2SPinDetail::mode(uint8_t value) {
        // I2S out pins cannot be configured, hence there
        // is nothing to do here for them.
    }

    // PWM
    bool I2SPinDetail::initPWM(uint32_t frequency, uint32_t maxDuty) {
        Assert(false, "PWM support is not available for I2S");
        return false;
    }
    uint32_t I2SPinDetail::getPWMFrequency() {
        Assert(false, "PWM support is not available for I2S");
        return 0;
    }
    uint32_t I2SPinDetail::getPWMMaxDuty() {
        Assert(false, "PWM support is not available for I2S");
        return 0;
    }
    void I2SPinDetail::setPWMDuty(uint32_t duty) { Assert(false, "PWM support is not available for I2S"); }

    String I2SPinDetail::toString() { return "I2S_" + int(_index); }
}

#endif

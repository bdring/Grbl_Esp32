#include "Pin.h"
#include "I2SOut.h"

#define I2S_OUT_PIN_BASE 128

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

namespace Pins {
    class GPIOPinDetail : public PinDetail {
        uint8_t _index;

    public:
        GPIOPinDetail(uint8_t index) : _index(index) {}

        // GPIO_NUM_3 maps to 3, so we can simply do it like this:
        void Write(bool high) override { __digitalWrite(_index, high); }
        int  Read() override { return __digitalRead(_index); }
        void Mode(uint8_t value) override { __pinMode(_index, value); }

        String ToString() override { return "GPIO_" + int(_index); }

        ~GPIOPinDetail() override {}
    };

#ifdef USE_I2S_OUT
    class I2SPinDetail : public PinDetail {
        uint8_t _index;

    public:
        I2SPinDetail(uint8_t index) : _index(index) {}

        // GPIO_NUM_3 maps to 3, so we can simply do it like this:
        void Write(bool high) override { i2s_out_write(pin, val); }
        int  Read() override { return i2s_out_state(pin); }
        void Mode(uint8_t value) override {
            // I2S out pins cannot be configured, hence there
            // is nothing to do here for them.
        }

        String ToString() override { return "I2S_" + int(_index); }

        ~I2SPinDetail() override {}
    };
#endif
}

Pin Pin::Create(String str) {
    if (str.startsWith("GPIO_NUM")) {  // For compatibility
        return Pin(new Pins::GPIOPinDetail(ParseUI8(str.begin() + 8, str.end())));
    } else if (str.startsWith("GPIO")) {
        return Pin(new Pins::GPIOPinDetail(ParseUI8(str.begin() + 4, str.end())));
    }
#ifdef USE_I2S_OUT
    else if (str.startsWith("I2S")) {
        return Pin(new Pins::I2SPinDetail(ParseUI8(str.begin() + 3, str.end())));
    }
#endif
    else {
        // Some fancy error
    }
}

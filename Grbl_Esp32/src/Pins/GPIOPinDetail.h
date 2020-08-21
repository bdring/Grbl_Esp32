#pragma once

#include "PinDetail.h"

namespace Pins {
    class GPIOPinDetail : public PinDetail {
        uint8_t _index;

    public:
        GPIOPinDetail(uint8_t index, const String& options);

        PinTraits traits() const override;

        // GPIO_NUM_3 maps to 3, so we can simply do it like this:
        void write(bool high) override;
        int  read() override;
        void mode(uint8_t value) override;

        // PWM
        bool     initPWM(uint32_t frequency, uint32_t maxDuty) override;
        uint32_t getPWMFrequency() override;
        uint32_t getPWMMaxDuty() override;
        void     setPWMDuty(uint32_t duty) override;

        String toString() override;

        ~GPIOPinDetail() override {}
    };

}

#pragma once

#include "PinDetail.h"

namespace Pins {
    class GPIOPinDetail : public PinDetail {
        uint8_t _index;

    public:
        GPIOPinDetail(uint8_t index, const PinOptionsParser& options);

        PinCapabilities traits() const override;

        // I/O:
        void write(bool high) override;
        int  read() override;
        void mode(uint8_t value) override;

        // ISR's:
        void attachInterrupt(void (*callback)(void*), void* arg, int mode) override;
        void detachInterrupt() override;

        // PWM
        bool     initPWM(uint32_t frequency, uint32_t maxDuty) override;
        uint32_t getPWMFrequency() override;
        uint32_t getPWMMaxDuty() override;
        void     setPWMDuty(uint32_t duty) override;

        String toString() const override;

        ~GPIOPinDetail() override {}
    };

}

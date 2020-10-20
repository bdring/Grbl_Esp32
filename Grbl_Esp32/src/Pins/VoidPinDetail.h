#pragma once

#include "PinDetail.h"
#include "PinOptionsParser.h"

namespace Pins {
    class VoidPinDetail : public PinDetail {
        uint32_t _frequency;
        uint32_t _maxDuty;

    public:
        VoidPinDetail(const PinOptionsParser& options);

        PinCapabilities capabilities() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;

        // PWM
        bool     initPWM(uint32_t frequency, uint32_t maxDuty) override;
        uint32_t getPWMFrequency() override;
        uint32_t getPWMMaxDuty() override;
        void     setPWMDuty(uint32_t duty) override;

        String toString() const override;

        ~VoidPinDetail() override {}
    };
}

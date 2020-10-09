#pragma once

#include "PinDetail.h"

namespace Pins {
    class DebugOutPinDetail : public PinDetail {
        uint8_t         _index;
        PinCapabilities _capabilities;
        PinAttributes   _attributes;
        int _readWriteMask;

        uint32_t _frequency = 0;
        uint32_t _maxDuty = 0;
        String _name;

    public:
        DebugOutPinDetail(uint8_t index, PinOptionsParser options);

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

        ~DebugOutPinDetail() override {}
    };

}

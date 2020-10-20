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

        String toString() const override;

        ~VoidPinDetail() override {}
    };
}

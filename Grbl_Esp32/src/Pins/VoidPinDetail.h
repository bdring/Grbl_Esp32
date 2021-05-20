#pragma once

#include "PinDetail.h"
#include "PinOptionsParser.h"

namespace Pins {
    class VoidPinDetail : public PinDetail {
    public:
        VoidPinDetail(int number = 0);
        VoidPinDetail(const PinOptionsParser& options);

        PinCapabilities capabilities() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;
        PinAttributes getAttr() const override;

        String toString() const override;

        ~VoidPinDetail() override {}
    };
}

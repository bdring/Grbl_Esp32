#pragma once

#include "PinDetail.h"
#include "PinOptionsParser.h"

namespace Pins {
    class VoidPinDetail : public PinDetail {
    public:
        VoidPinDetail(int number = 0);
        VoidPinDetail(const PinOptionsParser& options);

        PinCapabilities capabilities() const override;
        PinAttributes attributes() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;
        void reset() override;

        String toString() const override;

        ~VoidPinDetail() override {}
    };
}

#pragma once

#include "PinDetail.h"
#include "PinOptionsParser.h"

namespace Pins {
    class ErrorPinDetail : public PinDetail {
    public:
        ErrorPinDetail(const PinOptionsParser& options);

        PinCapabilities capabilities() const override;

        // I/O will all give an error:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;
        PinAttributes getAttr() const override;

        String toString() const override;

        ~ErrorPinDetail() override {}
    };

}

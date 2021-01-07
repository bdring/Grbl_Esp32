#pragma once

#include "PinDetail.h"
#include "PinOptionsParser.h"

namespace Pins {
    class ErrorPinDetail : public PinDetail {
    public:
        ErrorPinDetail(const PinOptionsParser& options);

        PinCapabilities capabilities() const override;
        PinAttributes   attributes() const override;

        // I/O will all give an error:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;
        void reset() override;

        String toString() const override;

        ~ErrorPinDetail() override {}
    };

}

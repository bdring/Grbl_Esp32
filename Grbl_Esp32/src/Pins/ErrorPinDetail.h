#pragma once

#include "PinDetail.h"

namespace Pins {
    class ErrorPinDetail : public PinDetail {
    public:
        ErrorPinDetail(const String& options);

        PinCapabilities capabilities() const override;

        // I/O will all give an error:
        void write(bool high) override;
        int  read() override;
        void mode(uint8_t value) override;

        String toString() const override;

        ~ErrorPinDetail() override {}
    };

}

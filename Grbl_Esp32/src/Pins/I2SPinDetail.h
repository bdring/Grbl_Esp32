#pragma once
#ifdef ESP32

#    include "PinDetail.h"

namespace Pins {
    class I2SPinDetail : public PinDetail {
        PinCapabilities _capabilities;
        PinAttributes   _attributes;
        int             _readWriteMask;

    public:
        I2SPinDetail(uint8_t index, const PinOptionsParser& options);

        PinCapabilities capabilities() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;

        String toString() const override;

        ~I2SPinDetail() override {}
    };
}

#endif

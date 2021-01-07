#pragma once
#ifdef ESP32

#    include "PinDetail.h"

namespace Pins {
    class I2SOPinDetail : public PinDetail {
        PinCapabilities _capabilities;
        PinAttributes   _attributes;
        int             _readWriteMask;

    public:
        I2SOPinDetail(uint8_t index, const PinOptionsParser& options);

        PinCapabilities capabilities() const override;
        PinAttributes attributes() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;
        void reset() override;

        String toString() const override;

        ~I2SOPinDetail() override {}
    };
}

#endif

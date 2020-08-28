#pragma once

#include "PinDetail.h"

#ifdef USE_I2S_OUT

namespace Pins {
    class I2SPinDetail : public PinDetail {
        uint8_t         _index;
        PinCapabilities _capabilities;
        PinAttributes   _attributes;

    public:
        I2SPinDetail(uint8_t index, const PinOptionsParser& options);

        PinCapabilities capabilities() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;

        String toString() override;

        ~I2SPinDetail() override {}
    };
}

#endif

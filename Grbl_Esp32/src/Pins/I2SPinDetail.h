#pragma once

#include "PinDetail.h"

#ifdef USE_I2S_OUT

namespace Pins {
    class I2SPinDetail : public PinDetail {
        uint8_t _index;

    public:
        I2SPinDetail(uint8_t index, const PinOptionsParser& options);

        PinTraits traits() const override;

        // I/O:
        void write(bool high) override;
        int  read() override;
        void mode(uint8_t value) override;

        String toString() override;

        ~I2SPinDetail() override {}
    };
}

#endif

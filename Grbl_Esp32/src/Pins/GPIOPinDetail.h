#pragma once

#include "PinDetail.h"

namespace Pins {
    class GPIOPinDetail : public PinDetail {
        uint8_t         _index;
        PinCapabilities _capabilities;
        PinAttributes   _attributes;
        int             _readWriteMask;

        static PinCapabilities GetDefaultCapabilities(uint8_t index);

    public:
        GPIOPinDetail(uint8_t index, PinOptionsParser options);

        PinCapabilities capabilities() const override;

        // I/O:
        void write(int high) override;
        int  read() override;
        void setAttr(PinAttributes value) override;

        // ISR's:
        void attachInterrupt(void (*callback)(void*), void* arg, int mode) override;
        void detachInterrupt() override;

        String toString() const override;

        ~GPIOPinDetail() override {}
    };

}

#pragma once

#include "PinCapabilities.h"
#include "PinAttributes.h"
#include "PinOptionsParser.h"

#include <WString.h>
#include <cstdint>
#include <cstring>

namespace Pins {
    // Implementation details of pins.
    class PinDetail {
    public:
        PinDetail()                   = default;
        PinDetail(const PinDetail& o) = delete;
        PinDetail(PinDetail&& o)      = delete;
        PinDetail& operator=(const PinDetail& o) = delete;
        PinDetail& operator=(PinDetail&& o) = delete;

        virtual PinCapabilities capabilities() const = 0;

        // I/O:
        virtual void write(int high)              = 0;
        virtual int  read()                       = 0;
        virtual void setAttr(PinAttributes value) = 0;

        // ISR's.
        virtual void attachInterrupt(void (*callback)(void*), void* arg, int mode);
        virtual void detachInterrupt();

        virtual String toString() const = 0;

        virtual ~PinDetail() {}
    };
}

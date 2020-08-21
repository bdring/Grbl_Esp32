#pragma once

#include "PinTraits.h"

#include <WString.h>
#include <cstdint>

class Pin;

namespace Pins {
    class PinDetail {
    private:
        friend class Pin;
        int referenceCount_ = 0;

    public:
        PinDetail()                   = default;
        PinDetail(const PinDetail& o) = delete;
        PinDetail(PinDetail&& o)      = delete;
        PinDetail& operator=(const PinDetail& o) = delete;
        PinDetail& operator=(PinDetail&& o) = delete;

        virtual PinTraits traits() const = 0;

        virtual void write(bool high)    = 0;
        virtual int  read()              = 0;
        virtual void mode(uint8_t value) = 0;

        // PWM
        // Returns true if successful  Deassign if frequency == 0 ?
        virtual bool initPWM(uint32_t frequency, uint32_t maxDuty) = 0;

        // Returns actual frequency which might not be exactly the same as requested(nearest supported value)
        virtual uint32_t getPWMFrequency() = 0;

        // Returns actual maxDuty which might not be exactly the same as requested(nearest supported value)
        virtual uint32_t getPWMMaxDuty() = 0;

        virtual void setPWMDuty(uint32_t duty) = 0;

        virtual String toString() const = 0;

        virtual ~PinDetail() {}
    };
}

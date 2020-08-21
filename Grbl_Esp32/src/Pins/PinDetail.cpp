#include "PinDetail.h"

#include "../Assert.h"

namespace Pins {
    void PinDetail::attachInterrupt(void (*callback)(void*), void* arg, int mode) {
        Assert(false, "Interrupts are not supported by this pin.");
    }
    void PinDetail::detachInterrupt() { Assert(false, "Interrupts are not supported by this pin."); }

    bool     PinDetail::initPWM(uint32_t frequency, uint32_t maxDuty) { Assert(false, "PWM is not supported by this pin."); }
    uint32_t PinDetail::getPWMFrequency() { Assert(false, "PWM is not supported by this pin."); }
    uint32_t PinDetail::getPWMMaxDuty() { Assert(false, "PWM is not supported by this pin."); }
    void     PinDetail::setPWMDuty(uint32_t duty) { Assert(false, "PWM is not supported by this pin."); }
}

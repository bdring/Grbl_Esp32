#include "PinDetail.h"

#include "../Assert.h"

namespace Pins {
    void PinDetail::attachInterrupt(void (*callback)(void*), void* arg, int mode) {
        Assert(false, "Interrupts are not supported by this pin.");
    }
    void PinDetail::detachInterrupt() { Assert(false, "Interrupts are not supported by this pin."); }
}

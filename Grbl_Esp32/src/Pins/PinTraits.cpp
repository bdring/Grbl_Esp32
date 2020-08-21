#include "PinTraits.h"

namespace Pins {
    PinTraits PinTraits::None(0);
    PinTraits PinTraits::Native(1);
    PinTraits PinTraits::Input(2);
    PinTraits PinTraits::Output(4);
    PinTraits PinTraits::PullUp(8);
    PinTraits PinTraits::PullDown(16);
    PinTraits PinTraits::PWM(32);
}

#include "PinTraits.h"

namespace Pins {
    PinTraits PinTraits::None(0);

    // Use a little trick here to ensure we don't make mistakes...
    // Do NOT add empty lines here, or more than 32 items!!!
#define START_LINE __LINE__ + 1
    PinTraits PinTraits::Native(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::Input(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::Output(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::PullUp(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::PullDown(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::PWM(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::ISR(1 << (__LINE__ - START_LINE));
    PinTraits PinTraits::I2S(1 << (__LINE__ - START_LINE));  // I did *NOT* want I2S, but TrinamicDriver needs it because of speeds...
#undef START_LINE

}

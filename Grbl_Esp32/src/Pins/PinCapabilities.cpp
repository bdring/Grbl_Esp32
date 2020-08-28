#include "PinCapabilities.h"

namespace Pins {
    PinCapabilities PinCapabilities::None(0);

    // Use a little trick here to ensure we don't make mistakes...
    // Do NOT add empty lines here, or more than 32 items!!!
#define START_LINE __LINE__ + 1
    PinCapabilities PinCapabilities::Native(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::Input(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::Output(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::PullUp(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::PullDown(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::PWM(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::ADC(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::DAC(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::ISR(1 << (__LINE__ - START_LINE));
    PinCapabilities PinCapabilities::I2S(1 << (__LINE__ - START_LINE));  // I did *NOT* want I2S, but TrinamicDriver needs it because of speeds...
#undef START_LINE

}

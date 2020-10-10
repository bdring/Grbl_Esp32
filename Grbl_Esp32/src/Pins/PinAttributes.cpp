#include "PinAttributes.h"
#include "PinCapabilities.h"

namespace Pins {
    PinAttributes PinAttributes::Undefined(0);
    PinAttributes PinAttributes::None(0);

    // The attributes that have a mapped capability have to be at
    // the top of the list. Note that this list must match the list
    // in capabilities _exactly_!

    const int     START_LINE = __LINE__ + 1;
    PinAttributes PinAttributes::Input(1 << (__LINE__ - START_LINE));
    PinAttributes PinAttributes::Output(1 << (__LINE__ - START_LINE));
    PinAttributes PinAttributes::PullUp(1 << (__LINE__ - START_LINE));
    PinAttributes PinAttributes::PullDown(1 << (__LINE__ - START_LINE));      
    PinAttributes PinAttributes::ISR(1 << (__LINE__ - START_LINE));           // ^        These are capabilities mapped
    const int     capabilityMask = (1 << (__LINE__ - START_LINE)) - 1;        // -------- Mask capabilities till here
    PinAttributes PinAttributes::NonExclusive(1 << (__LINE__ - START_LINE));  // \/       These are attributes

    bool PinAttributes::validateWith(PinCapabilities caps) {
        auto capMask  = (caps._value & capabilityMask);
        auto attrMask = _value;

        // We're good if:
        return (capMask & attrMask) == attrMask &&  // the capabilities overlap with the required attributes AND
               (capMask | attrMask) == capMask;     // if the required attributes all match an available capability
    }

    bool PinAttributes::conflictsWith(PinAttributes t) {
        if (_value == Undefined) {
            // If nothing is set, we're good.
            return false;
        } else {
            // If the attributes overlap, we're all good. Otherwise, we say there's a conflict.
            return t._value != _value;
        }
    }
}

#include "PinAttributes.h"
#include "PinCapabilities.h"

namespace Pins {
    PinAttributes PinAttributes::Undefined(0);
    PinAttributes PinAttributes::None(0);

#define LAST_CAPABILITY PinCapabilities::PullDown

    // Let's be smart about this. If we map pins on the same pins as PinCapabilities, validation
    // becomes much easier. So let's do that!
    PinAttributes PinAttributes::Input(PinCapabilities::Input._value);
    PinAttributes PinAttributes::Output(PinCapabilities::Output._value);
    PinAttributes PinAttributes::PullUp(PinCapabilities::PullUp._value);
    PinAttributes PinAttributes::PullDown(PinCapabilities::PullDown._value);

    bool PinAttributes::validateWith(PinCapabilities caps) {
        const uint32_t _capabilityMask = (LAST_CAPABILITY._value >> 1) - 1;
        auto           capMask         = (caps._value & _capabilityMask);
        auto           attrMask        = _value;

        // We're good if:
        return (capMask & attrMask) == attrMask &&  // the capabilities overlap with the required attributes AND
               (capMask | attrMask) == capMask;     // if the required attributes all match an available capability
    }

    // Plenty of room left for other numbers:
#define START_LINE (__LINE__ + 1 - (LAST_CAPABILITY._value))
    PinAttributes PinAttributes::NonExclusive(1 << (__LINE__ - START_LINE));
#undef START_LINE
#undef LAST_CAPABILITY

    bool PinAttributes::conflictsWith(PinAttributes t) {
        if (t == Undefined) {
            return false;
        } else {
            // If the attributes overlap, we're all good. Otherwise, we say there's a conflict.
            return t._value != _value;
        }
    }
}

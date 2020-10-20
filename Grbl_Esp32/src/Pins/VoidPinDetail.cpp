#include "VoidPinDetail.h"

namespace Pins {
    VoidPinDetail::VoidPinDetail(const PinOptionsParser& options) : _frequency(0), _maxDuty(0) {}

    PinCapabilities VoidPinDetail::capabilities() const { return PinCapabilities::None; }  // Should we?

    void VoidPinDetail::write(int high) {}
    int  VoidPinDetail::read() { return 0; }
    void VoidPinDetail::setAttr(PinAttributes value) {}

    String VoidPinDetail::toString() const { return "UNDEFINED_PIN"; }

}

#include "VoidPinDetail.h"

namespace Pins {
    VoidPinDetail::VoidPinDetail() : _frequency(0), _maxDuty(0) {}
    VoidPinDetail::VoidPinDetail(const PinOptionsParser& options) : VoidPinDetail() {}

    PinCapabilities VoidPinDetail::capabilities() const { return PinCapabilities::None; }  // Should we?

    void VoidPinDetail::write(int high) {}
    int  VoidPinDetail::read() { return 0; }
    void VoidPinDetail::setAttr(PinAttributes value) {}

    String VoidPinDetail::toString() const { return ""; }

}

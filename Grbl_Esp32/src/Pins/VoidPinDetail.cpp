#include "VoidPinDetail.h"

namespace Pins {
    VoidPinDetail::VoidPinDetail(int number) : PinDetail(number) {}
    VoidPinDetail::VoidPinDetail(const PinOptionsParser& options) : VoidPinDetail() {}

    PinCapabilities VoidPinDetail::capabilities() const {
        // Void pins support basic functionality. It just won't do you any good.
        return PinCapabilities::Output | PinCapabilities::Input | PinCapabilities::ISR | PinCapabilities::Void;
    }

    void VoidPinDetail::write(int high) {}
    int  VoidPinDetail::read() { return 0; }
    void VoidPinDetail::setAttr(PinAttributes value) {}
    PinAttributes VoidPinDetail::getAttr() const { return PinAttributes::None; }

    String VoidPinDetail::toString() const { return ""; }

}

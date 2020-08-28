#include "VoidPinDetail.h"

namespace Pins {
    VoidPinDetail::VoidPinDetail(const PinOptionsParser& options) {}

    PinCapabilities VoidPinDetail::capabilities() const { return PinCapabilities::None; }  // Should we?

    void VoidPinDetail::write(int high) {}
    int  VoidPinDetail::read() { return 0; }
    void VoidPinDetail::setAttr(PinAttributes value) {}

    bool VoidPinDetail::initPWM(uint32_t frequency, uint32_t maxDuty) {
        // We just set frequency and maxDuty to ensure we at least return values
        // for getPWMFreq / getPWMMaxDuty that make sense.
        _frequency = frequency;
        _maxDuty   = maxDuty;
        return false;
    }

    // Returns actual frequency which might not be exactly the same as requested(nearest supported value)
    uint32_t VoidPinDetail::getPWMFrequency() { return _frequency; }

    // Returns actual maxDuty which might not be exactly the same as requested(nearest supported value)
    uint32_t VoidPinDetail::getPWMMaxDuty() { return _maxDuty; }

    void VoidPinDetail::setPWMDuty(uint32_t duty) {}

    String VoidPinDetail::toString() const { return "Pin::UNDEFINED"; }

}

#include "ErrorPinDetail.h"
#include "../Assert.h"

namespace Pins {
    ErrorPinDetail::ErrorPinDetail(const String& options) {}

    PinCapabilities ErrorPinDetail::traits() const { return PinCapabilities::None; }

    void ErrorPinDetail::write(bool high) { Assert(false, "Cannot write to an error pin."); }
    int  ErrorPinDetail::read() { Assert(false, "Cannot read from an error pin."); }
    void ErrorPinDetail::mode(uint8_t value) { /* Fine, this won't get you anywhere. */}

    String ErrorPinDetail::toString() const { return "ERROR_PIN"; }
}

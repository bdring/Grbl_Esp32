#include "../TestFramework.h"

#include <src/Pin.h>

namespace Pins {
    Test(Error, Pins) {
        // Error pins should throw whenever they are used.

        Pin errorPin = Pin::ERROR;

        AssertThrow(errorPin.write(true));
        AssertThrow(errorPin.read());

        errorPin.setAttr(Pin::Attr::None);

        AssertThrow(errorPin.write(true));
        AssertThrow(errorPin.read());

        AssertThrow(errorPin.attachInterrupt([](void* arg) {}, CHANGE));
        AssertThrow(errorPin.detachInterrupt());

        Assert(errorPin.capabilities() == Pin::Capabilities::Error, "Incorrect caps");

        Assert(errorPin.name() == "ERROR_PIN");
    }
}

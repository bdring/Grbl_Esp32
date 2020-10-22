#include "../TestFramework.h"

#include <src/Pin.h>

namespace Pins {
    Test(Undefined, Pins) {
        // Unassigned pins are not doing much...

        Pin unassigned = Pin::UNDEFINED;
        Assert(Pin::UNDEFINED == unassigned, "Undefined has wrong pin id");

        {
            unassigned.write(true);
            auto result = unassigned.read();
            Assert(0 == result, "Result value incorrect");
        }

        {
            unassigned.write(false);
            auto result = unassigned.read();
            Assert(0 == result, "Result value incorrect");
        }

        AssertThrow(unassigned.attachInterrupt([](void* arg) {}, CHANGE));
        AssertThrow(unassigned.detachInterrupt());

        Assert(unassigned.capabilities() == Pin::Capabilities::None);
        Assert(unassigned.name().equals(UNDEFINED_PIN));
    }
}

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

        Assert(unassigned.capabilities().has(Pin::Capabilities::Void));
        Assert(unassigned.name().equals(""));
    }

    Test(Undefined, MultipleInstances) {
        {
            Pin unassigned  = Pin::UNDEFINED;
            Pin unassigned2 = Pin::UNDEFINED;

            Assert(unassigned == unassigned2, "Should evaluate to true.");
        }

        {
            Pin unassigned  = Pin::create("");
            Pin unassigned2 = Pin::UNDEFINED;

            Assert(unassigned == unassigned2, "Should evaluate to true.");
        }

        {
            Pin unassigned = Pin::create("void.2");
            Pin unassigned2 = Pin::UNDEFINED;

            Assert(unassigned != unassigned2, "Second void pin should match first.");
        }


        {
            Pin unassigned = Pin::create("void.2");
            Pin unassigned2 = Pin::create("void.2");

            Assert(unassigned == unassigned2, "Second void pin should match first.");
        }
    }
}

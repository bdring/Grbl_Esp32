#include "../TestFramework.h"

#include <src/Pin.h>
#include "../Support/SoftwareGPIO.h"

namespace Pins {
    TEST(Pins, Unassigned) {
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
    }
}

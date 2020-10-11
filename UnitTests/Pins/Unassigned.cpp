#include "gtest/gtest.h"

#include <src/Pin.h>
#include "../Support/SoftwareGPIO.h"

namespace Pins {
    TEST(Pins, Unassigned) {
        // Unassigned pins are not doing much...

        Pin unassigned = Pin::UNDEFINED;
        EXPECT_EQ(Pin::UNDEFINED, unassigned);

        {
            unassigned.write(true);
            auto result = unassigned.read();
            EXPECT_EQ(0, result);
        }

        {
            unassigned.write(false);
            auto result = unassigned.read();
            EXPECT_EQ(0, result);
        }

        EXPECT_ANY_THROW(unassigned.attachInterrupt([](void* arg) {}, CHANGE));
        EXPECT_ANY_THROW(unassigned.detachInterrupt());
    }
}

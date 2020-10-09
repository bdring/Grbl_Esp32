#include "gtest/gtest.h"

#include <src/Pin.h>

namespace Pins
{
    TEST(Pins, Unassigned) {

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
    }
}

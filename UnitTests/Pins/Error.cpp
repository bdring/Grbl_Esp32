#include "gtest/gtest.h"

#include <src/Pin.h>
#include "../Support/SoftwareGPIO.h"

namespace Pins {
    TEST(Pins, Error) {
        // Error pins should throw whenever they are used.

        Pin errorPin = Pin::ERROR;

        EXPECT_ANY_THROW(errorPin.write(true));
        EXPECT_ANY_THROW(errorPin.read());

        errorPin.setAttr(Pin::Attr::None);

        EXPECT_ANY_THROW(errorPin.write(true));
        EXPECT_ANY_THROW(errorPin.read());

        EXPECT_ANY_THROW(errorPin.attachInterrupt([](void* arg) {}, CHANGE));
        EXPECT_ANY_THROW(errorPin.detachInterrupt());

        ASSERT_EQ(errorPin.capabilities(), Pin::Capabilities::None);
    }
}

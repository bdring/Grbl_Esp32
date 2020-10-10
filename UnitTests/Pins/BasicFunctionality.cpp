#include "gtest/gtest.h"

#include <src/Pin.h>

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

    TEST(Pins, Error) {
        // Error pins should throw when they are used.

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

    TEST(Pins, GPIOBasics) {
        {
            PinLookup::ResetAllPins();
            Pin pin = Pin::create("GPIO.16");

            pin.setAttr(Pin::Attr::Input);
            ASSERT_EQ(false, pin.read());
        }
        {
            PinLookup::ResetAllPins();
            Pin pin = Pin::create("GPIO.16");

            pin.setAttr(Pin::Attr::Input);
            EXPECT_ANY_THROW(pin.on());
        }
    }
}

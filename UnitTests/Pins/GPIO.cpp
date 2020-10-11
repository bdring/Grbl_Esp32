#include "gtest/gtest.h"

#include <src/Pin.h>
#include "../Support/SoftwareGPIO.h"

namespace Pins {
    Pin Init() {
        SoftwareGPIO::reset();
        PinLookup::ResetAllPins();
        Pin pin = Pin::create("GPIO.16");
        return pin;
    }

    TEST(Read, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);
        ASSERT_EQ(false, pin.read());

        SoftwareGPIO::instance().set(16, true);

        ASSERT_EQ(true, pin.read());
    }

    TEST(WriteInputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);
        EXPECT_ANY_THROW(pin.on());
    }

    TEST(WriteOutputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output);

        ASSERT_EQ(false, SoftwareGPIO::instance().get(16));

        pin.on();
        ASSERT_EQ(true, SoftwareGPIO::instance().get(16));

        pin.off();
        ASSERT_EQ(false, SoftwareGPIO::instance().get(16));
    }
}

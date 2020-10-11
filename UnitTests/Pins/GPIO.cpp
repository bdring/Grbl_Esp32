#include "../TestFramework.h"

#include <src/Pin.h>
#include "../Support/SoftwareGPIO.h"

namespace Pins {
    Pin Init() {
        SoftwareGPIO::reset();
        PinLookup::ResetAllPins();
        Pin pin = Pin::create("GPIO.16");
        return pin;
    }

    TEST(ReadInputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);
        Assert(false == pin.read(), "Read has incorrect value.");

        SoftwareGPIO::instance().set(16, true);

        Assert(true == pin.read(), "Read has incorrect value.");
    }

    TEST(ReadOutputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output);
        AssertThrow(pin.read());
    }

    TEST(WriteInputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);
        AssertThrow(pin.on());
    }

    TEST(WriteOutputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output);

        Assert(false == SoftwareGPIO::instance().get(16), "Incorrect gpio value.");

        pin.on();
        Assert(true == SoftwareGPIO::instance().get(16), "Incorrect gpio value.");

        pin.off();
        Assert(false == SoftwareGPIO::instance().get(16), "Incorrect gpio value.");
    }

    TEST(ReadIOPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output | Pin::Attr::Input);
        Assert(false == pin.read(), "Incorrect read");
        Assert(false == SoftwareGPIO::instance().get(16), "Incorrect value");

        pin.on();
        Assert(true == pin.read(), "Incorrect read");
        Assert(true == SoftwareGPIO::instance().get(16), "Incorrect value");

        pin.off();
        Assert(false == pin.read(), "Incorrect read");
        Assert(false == SoftwareGPIO::instance().get(16), "Incorrect value");
    }

    void TestISR(int deltaRising, int deltaFalling, int mode) {
        auto pin = Init();
        pin.setAttr(Pin::Attr::Input | Pin::Attr::Output | Pin::Attr::ISR);

        int hitCount = 0;
        int expected = 0;
        pin.attachInterrupt(
            [](void* arg) {
                int* hc = static_cast<int*>(arg);
                ++(*hc);
            },
            mode,
            &hitCount);

        // Two ways to set I/O:
        // 1. using on/off
        // 2. external source (e.g. set softwareio pin value)
        //
        // We read as well, because that shouldn't modify the state.

        pin.on();
        expected += deltaRising;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(true == pin.read(), "Read error");

        pin.off();
        expected += deltaFalling;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(false == pin.read(), "Read error");

        SoftwareGPIO::instance().set(16, true);
        expected += deltaRising;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(true == pin.read(), "Read error");

        SoftwareGPIO::instance().set(16, false);
        expected += deltaFalling;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(false == pin.read(), "Read error");

        // Detach interrupt. Regardless of what we do, it shouldn't change hitcount anymore.
        pin.detachInterrupt();
        pin.on();
        pin.off();
        SoftwareGPIO::instance().set(16, true);
        SoftwareGPIO::instance().set(16, false);
        Assert(hitCount == expected, "ISR hitcount error");
    }

    TEST(ISRRisingPin, GPIO) { TestISR(1, 0, RISING); }

    TEST(ISRFallingPin, GPIO) { TestISR(0, 1, FALLING); }

    TEST(ISRChangePin, GPIO) { TestISR(1, 1, CHANGE); }
}

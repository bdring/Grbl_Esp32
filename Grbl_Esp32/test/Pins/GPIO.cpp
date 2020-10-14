#include "../TestFramework.h"

#include <src/Pin.h>

#ifdef ESP32

// Pin tests require a user to wire pin 16 to 17.

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

struct GPIOSupport {
    static void reset() {
        __pinMode(16, INPUT | OUTPUT);
        __pinMode(17, INPUT | OUTPUT);
        __digitalWrite(16, LOW);
        __digitalWrite(17, LOW);
        delay(10);
    }
    static bool state(int index) {
        delay(10);
        return __digitalRead(index) != LOW;
    }
    static void setState(int index, bool value) {
        __digitalWrite(index, value ? HIGH : LOW);
        delay(10);
    }
};

#else
#    include <SoftwareGPIO.h>

struct GPIOSupport {
    static void reset() { SoftwareGPIO::reset(); }
    static bool state(int index) { return SoftwareGPIO::instance().getInput(index); }
    static void setState(int index, bool value) {
        // We wired pin 16 to pin 17:
        if (index == 16) {
            SoftwareGPIO::instance().setOutput(16, value);
            SoftwareGPIO::instance().setInput(17, value);
        } else if (index == 17) {
            SoftwareGPIO::instance().setOutput(17, value);
            SoftwareGPIO::instance().setInput(16, value);
        } else {
            SoftwareGPIO::instance().setOutput(index, value);
        }
    }
};

#endif

namespace Pins {
    Pin Init() {
        GPIOSupport::reset();
        PinLookup::ResetAllPins();
        Pin pin = Pin::create("GPIO.16");
        return pin;
    }

    void DebugWrite() {
#ifdef ESP32
        Debug("Pin 16: %d, Pin 17: %d", __digitalRead(16), __digitalRead(17));
#endif
    }

    // We test GPIO pin 16:

    Test(ReadInputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);

        // Set 17 to false, which implicitly sets 16 to false.
        GPIOSupport::setState(17, false);
        Assert(false == pin.read(), "Read has incorrect value; expected false.");

        GPIOSupport::setState(17, true);
        Assert(true == pin.read(), "Read has incorrect value; expected true.");
    }

    Test(ReadOutputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output);
        AssertThrow(pin.read());
    }

    Test(WriteInputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);
        AssertThrow(pin.on());
    }

    Test(WriteOutputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output);

        Assert(false == GPIOSupport::state(17), "Expected initial gpio value of 'false'.");

        pin.on();
        Assert(true == GPIOSupport::state(17), "17 is wired to 16, so we expect 'true'.");

        pin.off();
        Assert(false == GPIOSupport::state(17), "17 is wired to 16, so we expect 'false'.");
    }

    Test(ReadIOPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output | Pin::Attr::Input);
        Assert(false == pin.read(), "Incorrect read");
        Assert(false == GPIOSupport::state(16), "Incorrect value");

        pin.on();
        Assert(false == pin.read(), "Incorrect read");
        Assert(false == GPIOSupport::state(16), "Incorrect value");

        pin.off();
        Assert(false == pin.read(), "Incorrect read");
        Assert(false == GPIOSupport::state(16), "Incorrect value");
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
        //
        // NOTE: Hysteresis tells us that we get changes a lot during a small
        // window in time. Unfortunately, it's practically impossible to test
        // because it bounces all over the place... TODO FIXME, some mechanism
        // to cope with that.

        for (int i = 0; i < 10; ++i) {
            if (deltaRising) {
                auto oldCount = hitCount;
                GPIOSupport::setState(17, true);
                delay(10);
                auto newCount = hitCount;

                if (deltaRising) {
                    Assert(oldCount < newCount, "Expected rise after set state");
                } else {
                    Assert(oldCount == newCount, "Expected no change after set state");
                }

                DebugWrite();
            } else {
                GPIOSupport::setState(17, true);
            }

            if (deltaFalling) {
                auto oldCount = hitCount;
                GPIOSupport::setState(17, false);
                delay(10);
                auto newCount = hitCount;

                Assert(oldCount < newCount, "Expected rise after set state");

                DebugWrite();
            } else {
                GPIOSupport::setState(17, false);
            }
        }

        // Detach interrupt. Regardless of what we do, it shouldn't change hitcount anymore.
        pin.detachInterrupt();

        auto oldCount = hitCount;
        pin.on();
        pin.off();
        GPIOSupport::setState(17, true);
        GPIOSupport::setState(17, false);
        auto newCount = hitCount;

        Assert(oldCount == newCount, "ISR hitcount error");
    }

    Test(ISRRisingPin, GPIO) { TestISR(1, 0, RISING); }

    Test(ISRFallingPin, GPIO) { TestISR(0, 1, FALLING); }

    Test(ISRChangePin, GPIO) { TestISR(1, 1, CHANGE); }
}

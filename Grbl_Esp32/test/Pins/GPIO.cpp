#include "../TestFramework.h"

#include <src/Pin.h>

#ifdef ESP32

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

struct GPIOSupport {
    static void reset() {
        __pinMode(16, INPUT | OUTPUT);
        __pinMode(17, INPUT | OUTPUT);
        __digitalWrite(16, LOW);
        __digitalWrite(17, LOW);
    }
    static bool state(int index) { return __digitalRead(17) != LOW; }
    static void setState(int index, bool value) { return __digitalWrite(17, value ? HIGH : LOW); }
};

#else
#    include <SoftwareGPIO.h>

struct GPIOSupport {
    static void reset() { SoftwareGPIO::reset(); }
    static bool state(int index) { return SoftwareGPIO::instance().get(index); }
    static void setState(int index, bool value) { SoftwareGPIO::instance().set(index, value); }
};

#endif

namespace Pins {
    Pin Init() {
        GPIOSupport::reset();
        PinLookup::ResetAllPins();
        Pin pin = Pin::create("GPIO.16");
        return pin;
    }

    Test(ReadInputPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Input);
        Assert(false == pin.read(), "Read has incorrect value.");

        GPIOSupport::setState(16, true);

        Assert(true == pin.read(), "Read has incorrect value.");
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

        Assert(false == GPIOSupport::state(16), "Incorrect gpio value.");

        pin.on();
        Assert(true == GPIOSupport::state(16), "Incorrect gpio value.");

        pin.off();
        Assert(false == GPIOSupport::state(16), "Incorrect gpio value.");
    }

    Test(ReadIOPin, GPIO) {
        auto pin = Init();

        pin.setAttr(Pin::Attr::Output | Pin::Attr::Input);
        Assert(false == pin.read(), "Incorrect read");
        Assert(false == GPIOSupport::state(16), "Incorrect value");

        pin.on();
        Assert(true == pin.read(), "Incorrect read");
        Assert(true == GPIOSupport::state(16), "Incorrect value");

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

        pin.on();
        expected += deltaRising;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(true == pin.read(), "Read error");

        pin.off();
        expected += deltaFalling;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(false == pin.read(), "Read error");

        GPIOSupport::setState(16, true);
        expected += deltaRising;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(true == pin.read(), "Read error");

        GPIOSupport::setState(16, false);
        expected += deltaFalling;
        Assert(hitCount == expected, "ISR hitcount error");
        Assert(false == pin.read(), "Read error");

        // Detach interrupt. Regardless of what we do, it shouldn't change hitcount anymore.
        pin.detachInterrupt();
        pin.on();
        pin.off();
        GPIOSupport::setState(16, true);
        GPIOSupport::setState(16, false);
        Assert(hitCount == expected, "ISR hitcount error");
    }

    Test(ISRRisingPin, GPIO) { TestISR(1, 0, RISING); }

    Test(ISRFallingPin, GPIO) { TestISR(0, 1, FALLING); }

    Test(ISRChangePin, GPIO) { TestISR(1, 1, CHANGE); }
}

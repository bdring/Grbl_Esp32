#include "Arduino.h"

#include "SoftwareGPIO.h"

void attachInterrupt(uint8_t pin, void (*callback)(void), int mode) {
    attachInterruptArg(
        pin,
        [](void* arg) {
            auto callback = reinterpret_cast<void (*)()>(arg);
            callback();
        },
        callback,
        mode);
}

void attachInterruptArg(uint8_t pin, void (*callback)(void*), void* arg, int mode) {
    SoftwareGPIO::instance().attachISR(pin, callback, arg, mode);
}

void detachInterrupt(uint8_t pin) {
    SoftwareGPIO::instance().detachISR(pin);
}

extern "C" int __digitalRead(uint8_t pin) {
    auto& io = SoftwareGPIO::instance();
    io.testMode(pin, INPUT);
    return io.get(pin) ? 1 : 0;
}

extern "C" void __pinMode(uint8_t pin, uint8_t mode) {
    auto& io = SoftwareGPIO::instance();
    io.setMode(pin, mode);
    io.testMode(pin, mode);  // just to be sure we didn't make a bug.
}

extern "C" void __digitalWrite(uint8_t pin, uint8_t val) {
    auto& io = SoftwareGPIO::instance();
    io.testMode(pin, OUTPUT);
    return io.set(pin, val ? true : false);
}

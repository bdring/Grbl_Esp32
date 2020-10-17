#include "Arduino.h"

#include "SoftwareGPIO.h"

#include <chrono>
#include <thread>

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
    return io.read(pin);
}

extern "C" void __pinMode(uint8_t pin, uint8_t mode) {
    auto& io = SoftwareGPIO::instance();
    io.setMode(pin, mode);
}

extern "C" void __digitalWrite(uint8_t pin, uint8_t val) {
    auto& io = SoftwareGPIO::instance();
    return io.writeOutput(pin, val ? true : false);
}

void delay(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

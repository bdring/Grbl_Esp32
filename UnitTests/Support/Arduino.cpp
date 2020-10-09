#include "Arduino.h"

void attachInterrupt(uint8_t pin, void (*)(void), int mode) {}
void attachInterruptArg(uint8_t pin, void (*)(void*), void* arg, int mode) {}
void detachInterrupt(uint8_t pin) {}

extern "C" int __digitalRead(uint8_t pin) {
    return 0;
}
extern "C" void __pinMode(uint8_t pin, uint8_t mode) {}
extern "C" void __digitalWrite(uint8_t pin, uint8_t val) {}

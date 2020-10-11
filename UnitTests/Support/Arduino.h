#pragma once

#include <cstdint>

#define IRAM_ATTR

// From Arduino.h:

// Interrupt Modes
#define RISING 0x01
#define FALLING 0x02
#define CHANGE 0x03
// #define ONLOW     0x04 --> Weird. Same as falling / rising?
// #define ONHIGH    0x05
// #define ONLOW_WE  0x0C --> not sure about these
// #define ONHIGH_WE 0x0D

// From esp32-hal-gpio.h:

#define LOW 0x0
#define HIGH 0x1

// GPIO FUNCTIONS
#define INPUT 0x01
#define OUTPUT 0x02
// #define PULLUP            0x04
#define INPUT_PULLUP 0x05
// #define PULLDOWN          0x08
#define INPUT_PULLDOWN 0x09
// #define OPEN_DRAIN        0x10
// #define OUTPUT_OPEN_DRAIN 0x12

void attachInterrupt(uint8_t pin, void (*)(void), int mode);
void attachInterruptArg(uint8_t pin, void (*)(void*), void* arg, int mode);
void detachInterrupt(uint8_t pin);

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

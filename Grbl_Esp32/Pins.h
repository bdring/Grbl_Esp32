#pragma once
#include "Arduino.h"

#define UNDEFINED_PIN 255  // Can be used to show a pin has no i/O assigned

#define I2S_OUT_PIN_BASE 128

extern "C" int __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

const char* pinName(uint8_t pin);


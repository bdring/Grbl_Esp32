#pragma once

#include <Arduino.h>

const int UNDEFINED_PIN    = 255;  // Can be used to show a pin has no i/O assigned
const int I2S_OUT_PIN_BASE = 128;

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

String pinName(uint8_t pin);

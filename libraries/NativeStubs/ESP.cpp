#include <Arduino.h>
#if EPOXY_DUINO_VERSION < 10000
#    include "ESP.h"
EspClass ESP;
#else
#    include <stdint.h>

extern "C" {
void attachInterrupt(uint8_t pin, void (*isr_limit_switches)(), int change) {}
void detachInterrupt(uint8_t pin) {}
}
#endif

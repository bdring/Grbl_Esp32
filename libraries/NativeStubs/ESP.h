#pragma once

#include <stdint.h>

inline int temperatureRead() {
    return 27;
}

inline long long esp_timer_get_time() {
    return 0LL;
}

typedef int esp_err_t;

const esp_err_t ESP_OK = 0;

inline uint32_t getApbFrequency() {
    return 80000000;
}

inline void gpio_reset_pin(uint8_t pin) {}
inline int  digitalPinToInterrupt(uint8_t pin) {
    return 0;
}

#ifndef EPOXY_DUINO
// Most of the EspClass stuff is used by information reports,
// except for restart()
class EspClass {
public:
    const char*     getSdkVersion() { return "native"; }
    inline void     restart() {}
    inline uint64_t getEfuseMac() { return 0ULL; }
    inline uint32_t getCpuFreqMHz() { return 240000000; }
    inline uint32_t getFreeHeap() { return 30000; }
    inline uint32_t getFlashChipSize() { return 0x400000; }
};
extern EspClass ESP;
#endif

#define NO_TASKS

void attachInterrupt(uint8_t pin, void (*isr_limit_switches)(), int change);
void detachInterrupt(uint8_t pin);

inline void NOP() {}

// dacWrite stub - used by DacSpindle
inline void dacWrite(int _output_pin, uint8_t duty) {}

inline void ledcSetup(int channel_num, double freq, int bits) {}
inline void ledcWrite(int channel_num, int duty) {}
inline void ledcAttachPin(int pwm_pin, int channel_num) {}

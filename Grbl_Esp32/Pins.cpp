#include "Arduino.h"
#include "config.h"
#include "Pins.h"

String pinName(uint8_t pin) {
    if (pin == UNDEFINED_PIN) {
        return "NONE";
    }
    if (pin < I2S_OUT_PIN_BASE) {
        return String("GPIO_") + pin;
    } else {
        return String("I2SO_") + (pin - I2S_OUT_PIN_BASE);
    }
}

#ifdef USE_I2S_OUT
#include "i2s_out.h"

// If we are using I2S, overload these standard Arduino
// functions with versions that support I2S pins.
// Otherwise the weak aliases that already exist will apply.
void IRAM_ATTR pinMode(uint8_t pin, uint8_t mode) {
    if (pin < I2S_OUT_PIN_BASE) {
        __pinMode(pin, mode);
    }
}

void IRAM_ATTR digitalWrite(uint8_t pin, uint8_t val) {
    if (pin < I2S_OUT_PIN_BASE) {
        __digitalWrite(pin, val);
        return;
    }
    i2s_out_write(pin - I2S_OUT_PIN_BASE, val);
}

int IRAM_ATTR digitalRead(uint8_t pin) {
    if (pin < I2S_OUT_PIN_BASE) {
        return __digitalRead(pin);
    }
    return i2s_out_state(pin - I2S_OUT_PIN_BASE);
}
#endif

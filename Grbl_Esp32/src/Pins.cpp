#include "Grbl.h"
#include "I2SOut.h"

String pinName(uint8_t pin) {
    if (pin == UNDEFINED_PIN) {
        return "None";
    }
    if (pin < I2S_OUT_PIN_BASE) {
        return String("GPIO(") + pin + ")";
    } else {
        return String("I2SO(") + (pin - I2S_OUT_PIN_BASE) + ")";
    }
}

// Even if USE_I2S_OUT is not defined, it is necessary to
// override the following functions, instead of allowing
// the weak aliases in the library to apply, because of
// the UNDEFINED_PIN check.  That UNDEFINED_PIN behavior
// cleans up other code by eliminating ifdefs and checks.
void IRAM_ATTR digitalWrite(uint8_t pin, uint8_t val) {
    if (pin == UNDEFINED_PIN) {
        return;
    }
    if (pin < I2S_OUT_PIN_BASE) {
        __digitalWrite(pin, val);
        return;
    }
#ifdef USE_I2S_OUT
    i2s_out_write(pin - I2S_OUT_PIN_BASE, val);
#endif
}

void IRAM_ATTR pinMode(uint8_t pin, uint8_t mode) {
    if (pin == UNDEFINED_PIN) {
        return;
    }
    if (pin < I2S_OUT_PIN_BASE) {
        __pinMode(pin, mode);
    }
    // I2S out pins cannot be configured, hence there
    // is nothing to do here for them.
}

int IRAM_ATTR digitalRead(uint8_t pin) {
    if (pin == UNDEFINED_PIN) {
        return 0;
    }
    if (pin < I2S_OUT_PIN_BASE) {
        return __digitalRead(pin);
    }
#ifdef USE_I2S_OUT
    return i2s_out_read(pin - I2S_OUT_PIN_BASE);
#else
    return 0;
#endif
}

// For pins that are not associated with an axis
void initPin(uint8_t pin, uint8_t mode, const char *name) {
    if (!reinit) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s on pin %s", name, pinName(pin).c_str());
        pinMode(pin, mode);
    }
}

// For pins that are associated with an non-gangeable axis
void initPin(uint8_t pin, uint8_t mode, uint8_t axis, const char *name) {
    if (!reinit) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s %s on pin %s", reportAxisNameMsg(axis), name, pinName(pin).c_str());
        pinMode(pin, mode);
    }
}

// For pins that are associated with a gangeable axis
void initPin(uint8_t pin, uint8_t mode, uint8_t axis, uint8_t gang, const char *name) {
    if (!reinit) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s %s on pin %s", reportAxisNameMsg(axis, gang), name, pinName(pin).c_str());
        pinMode(pin, mode);
    }
}

// For arrays of pins such as user digital pins
void initPin(uint8_t pin, uint8_t mode, const char *name, int n) {
    if (!reinit) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s:%d on pin %s", name, n, pinName(pin).c_str());
        pinMode(pin, mode);
    }
}


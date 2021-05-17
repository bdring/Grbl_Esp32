#pragma once

// Returns if safety door is ajar(T) or closed(F), based on pin state.
bool system_check_safety_door_ajar();

class ControlPin {
private:
    uint8_t _pin;
    // invertBitNum refers to a bit in INVERT_CONTROL_PIN_MASK.  It is a
    // short-term hack to reduce the extent of the patch.
    bool           _invertBitNum;
    bool           _value;
    const char     _letter;
    volatile bool* _rtVariable;
    const char*    _name;

public:
    ControlPin(uint8_t pin, uint8_t bitNum, volatile bool* rtVariable, const char* name, char letter) :
        _pin(pin), _invertBitNum(bitNum), _value(false), _letter(letter), _rtVariable(rtVariable), _name(name) {}

    uint8_t     pin() { return _pin; }
    bool        get() { return _value; }
    const char* name() { return _name; }
    //    char        invertBitNum() { return _invertBitNum; }
    char           letter() { return _letter; }
    void           init();
    void IRAM_ATTR handle();
};

extern ControlPin safetyDoorPin;
extern ControlPin resetPin;
extern ControlPin feedHoldPin;
extern ControlPin cycleStartPin;
extern ControlPin macro0Pin;
extern ControlPin macro1Pin;
extern ControlPin macro2Pin;
extern ControlPin macro3Pin;

extern void init_control_pins();

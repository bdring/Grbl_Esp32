#include "Grbl.h"

ControlPin safetyDoorPin(CONTROL_SAFETY_DOOR_PIN, 0, &rtSafetyDoor, "Door", 'D');
ControlPin resetPin(CONTROL_RESET_PIN, 1, &rtReset, "Reset", 'R');
ControlPin feedHoldPin(CONTROL_FEED_HOLD_PIN, 2, &rtFeedHold, "FeedHold", 'H');
ControlPin cycleStartPin(CONTROL_CYCLE_START_PIN, 3, &rtCycleStart, "CycleStart", 'S');
ControlPin macro0Pin(MACRO_BUTTON_0_PIN, 4, &rtButtonMacro0, "Macro 0", '0');
ControlPin macro1Pin(MACRO_BUTTON_1_PIN, 5, &rtButtonMacro1, "Macro 1", '1');
ControlPin macro2Pin(MACRO_BUTTON_2_PIN, 6, &rtButtonMacro2, "Macro 2", '2');
ControlPin macro3Pin(MACRO_BUTTON_3_PIN, 7, &rtButtonMacro3, "Macro 3", '3');

// XXX we need to dispatch the user defined macros somehow
// user_defined_macro(N)

void IRAM_ATTR ControlPin::handle() {
    bool pinState = digitalRead(_pin) ^ bit_istrue(_invertBitNum, INVERT_CONTROL_PIN_MASK);
    _value        = pinState;
    if (_rtVariable) {
        *_rtVariable = pinState;
    }
}

void IRAM_ATTR handle_control_pin(void* arg) {
    ((ControlPin*)arg)->handle();
}

void ControlPin::init() {
    if (_pin != UNDEFINED_PIN) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s switch on pin %s", _name, pinName(_pin).c_str());
        pinMode(_pin, INPUT_PULLUP);
        attachInterruptArg(digitalPinToInterrupt(_pin), handle_control_pin, this, CHANGE);
    }
}

void init_control_pins() {
    safetyDoorPin.init();
    resetPin.init();
    feedHoldPin.init();
    cycleStartPin.init();
    macro0Pin.init();
    macro1Pin.init();
    macro2Pin.init();
    macro3Pin.init();
}

// Returns if safety door is ajar(T) or closed(F), based on pin state.
bool system_check_safety_door_ajar() {
    // If a safety door pin is not defined, this will return false
    // because that is the default for the value field, which will
    // never be changed for an undefined pin.
    return safetyDoorPin.get();
}

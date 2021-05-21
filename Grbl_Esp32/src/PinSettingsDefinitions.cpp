// This file attempts to build all the pin settings definitions, while retaining backward compatibility with the
// original machine files.
//
// Note that the source code shouldn't use #define'd pins directly, but always use the settings in the code.
// To aid that, I do NOT want to include the default <Arduino.h> from this file (indirectly). Instead, this
// file defines the GPIO_NUM_* and I2SO(*) as macro's to strings that the Pin class understands, and uses that.
//
// To make sure we don't accidentally ripple macro's some place they don't belong, I tried to do this as isolated
// as possible.
//
// NOTE: The order in which defines and includes are used here matters!
//
// All the preprocessor things are handled first, followed by the real code.

// First do the defines that take care of GPIO pin name mapping:

#include <cstdint>
#include "Config.h"

// Include the file that loads the machine-specific config file.
// machine.h must be edited to choose the desired file.
#include "Machine.h"

// machine_common.h contains settings that do not change
#include "MachineCommon.h"

// Update missing definitions with defaults:
#include "Defaults.h"

// Set defaults to all the macro's:
#ifndef SDCARD_DET_PIN
#    define SDCARD_DET_PIN UNDEFINED_PIN
#endif
const char* SDCARD_DET_PIN_DEFAULT = SDCARD_DET_PIN;
#ifndef CONTROL_SAFETY_DOOR_PIN
#    define CONTROL_SAFETY_DOOR_PIN UNDEFINED_PIN
#endif
const char* CONTROL_SAFETY_DOOR_PIN_DEFAULT = CONTROL_SAFETY_DOOR_PIN;
#ifndef CONTROL_RESET_PIN
#    define CONTROL_RESET_PIN UNDEFINED_PIN
#endif
const char* CONTROL_RESET_PIN_DEFAULT = CONTROL_RESET_PIN;
#ifndef CONTROL_FEED_HOLD_PIN
#    define CONTROL_FEED_HOLD_PIN UNDEFINED_PIN
#endif
const char* CONTROL_FEED_HOLD_PIN_DEFAULT = CONTROL_FEED_HOLD_PIN;
#ifndef CONTROL_CYCLE_START_PIN
#    define CONTROL_CYCLE_START_PIN UNDEFINED_PIN
#endif
const char* CONTROL_CYCLE_START_PIN_DEFAULT = CONTROL_CYCLE_START_PIN;
#ifndef MACRO_BUTTON_0_PIN
#    define MACRO_BUTTON_0_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_0_PIN_DEFAULT = MACRO_BUTTON_0_PIN;
#ifndef MACRO_BUTTON_1_PIN
#    define MACRO_BUTTON_1_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_1_PIN_DEFAULT = MACRO_BUTTON_1_PIN;
#ifndef MACRO_BUTTON_2_PIN
#    define MACRO_BUTTON_2_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_2_PIN_DEFAULT = MACRO_BUTTON_2_PIN;
#ifndef MACRO_BUTTON_3_PIN
#    define MACRO_BUTTON_3_PIN UNDEFINED_PIN
#endif
const char* MACRO_BUTTON_3_PIN_DEFAULT = MACRO_BUTTON_3_PIN;

#ifndef USER_DIGITAL_PIN_0
#    define USER_DIGITAL_PIN_0 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_0_DEFAULT = USER_DIGITAL_PIN_0;

#ifndef USER_DIGITAL_PIN_1
#    define USER_DIGITAL_PIN_1 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_1_DEFAULT = USER_DIGITAL_PIN_1;

#ifndef USER_DIGITAL_PIN_2
#    define USER_DIGITAL_PIN_2 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_2_DEFAULT = USER_DIGITAL_PIN_2;

#ifndef USER_DIGITAL_PIN_3
#    define USER_DIGITAL_PIN_3 UNDEFINED_PIN
#endif
const char* USER_DIGITAL_PIN_3_DEFAULT = USER_DIGITAL_PIN_3;

#ifndef USER_ANALOG_PIN_0
#    define USER_ANALOG_PIN_0 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_0_DEFAULT = USER_ANALOG_PIN_0;

#ifndef USER_ANALOG_PIN_1
#    define USER_ANALOG_PIN_1 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_1_DEFAULT = USER_ANALOG_PIN_1;

#ifndef USER_ANALOG_PIN_2
#    define USER_ANALOG_PIN_2 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_2_DEFAULT = USER_ANALOG_PIN_2;

#ifndef USER_ANALOG_PIN_3
#    define USER_ANALOG_PIN_3 UNDEFINED_PIN
#endif
const char* USER_ANALOG_PIN_3_DEFAULT = USER_ANALOG_PIN_3;

// We need settings.h for the settings classes
#include "Grbl.h"
#include "Settings.h"
#include "SettingsDefinitions.h"

// Define the pins:

PinSetting* ControlSafetyDoorPin;  // CONTROL_SAFETY_DOOR_PIN
PinSetting* ControlResetPin;       // CONTROL_RESET_PIN
PinSetting* ControlFeedHoldPin;    // CONTROL_FEED_HOLD_PIN
PinSetting* ControlCycleStartPin;  // CONTROL_CYCLE_START_PIN
PinSetting* MacroButton0Pin;       // MACRO_BUTTON_0_PIN
PinSetting* MacroButton1Pin;       // MACRO_BUTTON_1_PIN
PinSetting* MacroButton2Pin;       // MACRO_BUTTON_2_PIN
PinSetting* MacroButton3Pin;       // MACRO_BUTTON_3_PIN

PinSetting* UserDigitalPin[4];
PinSetting* UserAnalogPin[4];

PinSetting* SDCardDetPin;  // SDCARD_DET_PIN

#include "Pin.h"

// Initialize the pin settings
void make_pin_settings() {
    ControlSafetyDoorPin = new PinSetting("Control/SafetyDoor/Pin", CONTROL_SAFETY_DOOR_PIN_DEFAULT);
    ControlResetPin      = new PinSetting("Control/Reset/Pin", CONTROL_RESET_PIN_DEFAULT);
    ControlFeedHoldPin   = new PinSetting("Control/FeedHold/Pin", CONTROL_FEED_HOLD_PIN_DEFAULT);
    ControlCycleStartPin = new PinSetting("Control/CycleStart/Pin", CONTROL_CYCLE_START_PIN_DEFAULT);
    MacroButton0Pin      = new PinSetting("Macro/0/Pin", MACRO_BUTTON_0_PIN_DEFAULT);
    MacroButton1Pin      = new PinSetting("Macro/1/Pin", MACRO_BUTTON_1_PIN_DEFAULT);
    MacroButton2Pin      = new PinSetting("Macro/2/Pin", MACRO_BUTTON_2_PIN_DEFAULT);
    MacroButton3Pin      = new PinSetting("Macro/3/Pin", MACRO_BUTTON_3_PIN_DEFAULT);

    // User pins:
    UserDigitalPin[0] = new PinSetting("UserDigital/0/Pin", USER_DIGITAL_PIN_0_DEFAULT);
    UserAnalogPin[0]  = new PinSetting("UserAnalog/0/Pin", USER_ANALOG_PIN_0_DEFAULT);
    UserDigitalPin[1] = new PinSetting("UserDigital/1/Pin", USER_DIGITAL_PIN_1_DEFAULT);
    UserAnalogPin[1]  = new PinSetting("UserAnalog/1/Pin", USER_ANALOG_PIN_1_DEFAULT);
    UserDigitalPin[2] = new PinSetting("UserDigital/2/Pin", USER_DIGITAL_PIN_2_DEFAULT);
    UserAnalogPin[2]  = new PinSetting("UserAnalog/2/Pin", USER_ANALOG_PIN_2_DEFAULT);
    UserDigitalPin[3] = new PinSetting("UserDigital/3/Pin", USER_DIGITAL_PIN_3_DEFAULT);
    UserAnalogPin[3]  = new PinSetting("UserAnalog/3/Pin", USER_ANALOG_PIN_3_DEFAULT);

    SDCardDetPin = new PinSetting("SDCardDet/Pin", SDCARD_DET_PIN_DEFAULT);
}

#pragma once

extern StringSetting* startup_line_0;
extern StringSetting* startup_line_1;
extern StringSetting* build_info;

extern IntSetting* status_mask;

extern EnumSetting* spindle_type;

extern StringSetting* user_macro0;
extern StringSetting* user_macro1;
extern StringSetting* user_macro2;
extern StringSetting* user_macro3;

extern EnumSetting* message_level;

// Pins:
extern PinSetting* ControlSafetyDoorPin;  //  CONTROL_SAFETY_DOOR_PIN
extern PinSetting* ControlResetPin;       //  CONTROL_RESET_PIN
extern PinSetting* ControlFeedHoldPin;    //  CONTROL_FEED_HOLD_PIN
extern PinSetting* ControlCycleStartPin;  //  CONTROL_CYCLE_START_PIN
extern PinSetting* MacroButton0Pin;       //  MACRO_BUTTON_0_PIN
extern PinSetting* MacroButton1Pin;       //  MACRO_BUTTON_1_PIN
extern PinSetting* MacroButton2Pin;       //  MACRO_BUTTON_2_PIN
extern PinSetting* MacroButton3Pin;       //  MACRO_BUTTON_3_PIN

extern PinSetting* UserDigitalPin[4];
extern PinSetting* UserAnalogPin[4];

extern PinSetting* SDCardDetPin;  // SDCARD_DET_PIN

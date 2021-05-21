#pragma once

extern FlagSetting* verbose_errors;

extern StringSetting* startup_line_0;
extern StringSetting* startup_line_1;
extern StringSetting* build_info;

extern IntSetting* stepper_idle_lock_time;

extern AxisMaskSetting* homing_dir_mask;
extern AxisMaskSetting* homing_squared_axes;
extern AxisMaskSetting* homing_cycle[MAX_N_AXIS];

extern FlagSetting* report_inches;
extern FlagSetting* homing_enable;

extern IntSetting* status_mask;

extern FloatSetting* homing_feed_rate;
extern FloatSetting* homing_seek_rate;
extern FloatSetting* homing_debounce;
extern FloatSetting* homing_pulloff;
extern FloatSetting* coolant_start_delay;

extern IntSetting* spindle_pwm_bit_precision;

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

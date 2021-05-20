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
extern IntSetting*  laser_full_power;

extern IntSetting* status_mask;

extern FloatSetting* homing_feed_rate;
extern FloatSetting* homing_seek_rate;
extern FloatSetting* homing_debounce;
extern FloatSetting* homing_pulloff;
extern FloatSetting* spindle_pwm_freq;
extern FloatSetting* rpm_max;
extern FloatSetting* rpm_min;
extern FloatSetting* spindle_delay_spinup;
extern FloatSetting* spindle_delay_spindown;
extern FloatSetting* coolant_start_delay;
extern FlagSetting*  spindle_enbl_off_with_zero_speed;
extern FlagSetting*  spindle_enable_invert;
extern FlagSetting*  spindle_output_invert;

extern FloatSetting* spindle_pwm_off_value;
extern FloatSetting* spindle_pwm_min_value;
extern FloatSetting* spindle_pwm_max_value;
extern IntSetting*   spindle_pwm_bit_precision;

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

// Spindle pins:
extern PinSetting* SpindleOutputPin;
extern PinSetting* SpindleEnablePin;
extern PinSetting* SpindleDirectionPin;
extern PinSetting* SpindleForwardPin;
extern PinSetting* SpindleReversePin;

extern PinSetting* LaserOutputPin;
extern PinSetting* LaserEnablePin;

extern PinSetting* VFDRS485TXDPin;  // VFD_RS485_TXD_PIN
extern PinSetting* VFDRS485RXDPin;  // VFD_RS485_RXD_PIN
extern PinSetting* VFDRS485RTSPin;  // VFD_RS485_RTS_PIN

#pragma once

extern FlagSetting* verbose_errors;

extern StringSetting* machine_name;

extern IntSetting* number_axis;

extern AxisSettings* axis_settings[];

extern StringSetting* startup_line_0;
extern StringSetting* startup_line_1;
extern StringSetting* build_info;

extern IntSetting* pulse_microseconds;
extern IntSetting* stepper_idle_lock_time;
extern IntSetting* step_pulse_delay;

extern AxisMaskSetting* dir_invert_mask;
extern AxisMaskSetting* homing_dir_mask;
extern AxisMaskSetting* homing_squared_axes;
extern AxisMaskSetting* homing_cycle[MAX_N_AXIS];

extern FlagSetting* report_inches;
extern FlagSetting* soft_limits;
extern FlagSetting* hard_limits;
extern FlagSetting* homing_enable;
extern FlagSetting* laser_mode;
extern IntSetting*  laser_full_power;

extern IntSetting*   status_mask;
extern FloatSetting* junction_deviation;
extern FloatSetting* arc_tolerance;

extern FloatSetting* homing_feed_rate;
extern FloatSetting* homing_seek_rate;
extern FloatSetting* homing_debounce;
extern FloatSetting* homing_pulloff;
extern FloatSetting* spindle_pwm_freq;
extern FloatSetting* rpm_max;
extern FloatSetting* rpm_min;
extern FloatSetting* spindle_delay_spinup;
extern FloatSetting* spindle_delay_spindown;
extern FlagSetting*  spindle_enbl_off_with_zero_speed;

extern FloatSetting* spindle_pwm_off_value;
extern FloatSetting* spindle_pwm_min_value;
extern FloatSetting* spindle_pwm_max_value;
extern IntSetting*   spindle_pwm_bit_precision;

extern EnumSetting* spindle_type;

extern EnumSetting*  motor_types[MAX_N_AXIS][2];
extern FloatSetting* motor_rsense[MAX_N_AXIS][2];
extern IntSetting*   motor_address[MAX_N_AXIS][2];
extern FloatSetting* rc_servo_cal_min[MAX_N_AXIS][2];
extern FloatSetting* rc_servo_cal_max[MAX_N_AXIS][2];

extern EnumSetting*     trinamic_run_mode;
extern EnumSetting*     trinamic_homing_mode;
extern AxisMaskSetting* stallguard_debug_mask;

// Pins:
extern PinSetting* CoolantFloodPin;     // COOLANT_FLOOD_PIN
extern PinSetting* CoolantMistPin;      // COOLANT_MIST_PIN
extern PinSetting* ProbePin;            // PROBE_PIN
extern PinSetting* SDCardDetPin;        // SDCARD_DET_PIN
extern PinSetting* SteppersDisablePin;  // STEPPERS_DISABLE_PIN
extern PinSetting* StepperResetPin;     // STEPPER_RESET

extern PinSetting* ControlSafetyDoorPin;  //  CONTROL_SAFETY_DOOR_PIN
extern PinSetting* ControlResetPin;       //  CONTROL_RESET_PIN
extern PinSetting* ControlFeedHoldPin;    //  CONTROL_FEED_HOLD_PIN
extern PinSetting* ControlCycleStartPin;  //  CONTROL_CYCLE_START_PIN
extern PinSetting* MacroButton0Pin;       //  MACRO_BUTTON_0_PIN
extern PinSetting* MacroButton1Pin;       //  MACRO_BUTTON_1_PIN
extern PinSetting* MacroButton2Pin;       //  MACRO_BUTTON_2_PIN
extern PinSetting* MacroButton3Pin;       //  MACRO_BUTTON_3_PIN

extern PinSetting* DynamixelTXDPin;  // DYNAMIXEL_TXD
extern PinSetting* DynamixelRXDPin;  // DYNAMIXEL_RXD
extern PinSetting* DynamixelRTSPin;  // DYNAMIXEL_RTS

extern PinSetting* UserDigitalPin[4];
extern PinSetting* UserAnalogPin[4];

// SPI pins:
extern PinSetting* SPISSPin;    // SS
extern PinSetting* SPISCKPin;   // SCK
extern PinSetting* SPIMISOPin;  // MISO
extern PinSetting* SPIMOSIPin;  // MOSI

extern PinSetting* I2SOBCK;
extern PinSetting* I2SOWS;
extern PinSetting* I2SOData;

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

// TODO FIXME: Add motor type (enum) setting! That way we can properly hide a lot of settings, and make Motors.cpp proper!
extern PinSetting* LimitPins[MAX_N_AXIS][2];
extern PinSetting* StepPins[MAX_N_AXIS][2];
extern PinSetting* DirectionPins[MAX_N_AXIS][2];
extern PinSetting* DisablePins[MAX_N_AXIS][2];
extern PinSetting* ChipSelectPins[MAX_N_AXIS][2];
extern PinSetting* ServoPins[MAX_N_AXIS][2];
extern PinSetting* PhasePins[4][MAX_N_AXIS][2];
extern PinSetting* StepStickMS3[MAX_N_AXIS][2];

extern StringSetting* user_macro0;
extern StringSetting* user_macro1;
extern StringSetting* user_macro2;
extern StringSetting* user_macro3;

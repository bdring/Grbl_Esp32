#pragma once
extern bool motorSettingChanged;

extern AxisSettings* x_axis_settings;
extern AxisSettings* y_axis_settings;
extern AxisSettings* z_axis_settings;
extern AxisSettings* a_axis_settings;
extern AxisSettings* b_axis_settings;
extern AxisSettings* c_axis_settings;
extern AxisSettings* axis_settings[];

extern StringSetting* startup_line_0;
extern StringSetting* startup_line_1;
extern StringSetting* build_info;

extern IntSetting* pulse_microseconds;
extern IntSetting* stepper_idle_lock_time;

extern AxisMaskSetting* step_invert_mask;
extern AxisMaskSetting* dir_invert_mask;
extern AxisMaskSetting* homing_dir_mask;

extern FlagSetting* step_enable_invert;
extern FlagSetting* limit_invert;
extern FlagSetting* probe_invert;
extern FlagSetting* report_inches;
extern FlagSetting* soft_limits;
extern FlagSetting* hard_limits;
extern FlagSetting* homing_enable;
extern FlagSetting* laser_mode;

extern IntSetting* status_mask;
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

extern FloatSetting* spindle_pwm_off_value;
extern FloatSetting* spindle_pwm_min_value;
extern FloatSetting* spindle_pwm_max_value;
extern IntSetting* spindle_pwm_bit_precision;

extern EnumSetting* spindle_type;

extern AxisMaskSetting* stallguard_debug_mask;
//Plasma Torch Height Control Settings
extern FlagSetting* thc_debug_setting;
extern FloatSetting* thc_voltage_setting;
extern IntSetting* thc_debugprint_millis;
extern IntSetting* thc_arc_delay_time;
extern FloatSetting* thc_voltage_filter_value;
extern IntSetting* thc_iter_freq;
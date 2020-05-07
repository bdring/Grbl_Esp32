#pragma once
#include "SettingsDerivedClasses.h"

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

extern IntSetting* step_invert_mask;
extern IntSetting* dir_invert_mask;
// XXX need to call st_generate_step_invert_masks;
extern IntSetting* homing_dir_mask;

extern FlagSetting* step_enable_invert;
extern FlagSetting* limit_invert;
extern FlagSetting* probe_invert;
extern FlagSetting* report_inches;
extern FlagSetting* soft_limits;
// XXX need to check for HOMING_ENABLE
extern FlagSetting* hard_limits;
// XXX need to call limits_init;
extern FlagSetting* homing_enable;
// XXX also need to clear, but not set, BITFLAG_SOFT_LIMIT_ENABLE
extern FlagSetting* laser_mode;
// XXX also need to call my_spindle->init;

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

extern FloatSetting* spindle_pwm_off_value;
extern FloatSetting* spindle_pwm_min_value;
extern FloatSetting* spindle_pwm_max_value;
extern IntSetting* spindle_pwm_bit_precision;

extern EnumSetting* spindle_type;

extern StringSetting* wifi_sta_ssid;
extern StringSetting* wifi_sta_password;
// XXX hack StringSetting class to return a ***** password if checker is isPasswordValid

#ifdef ENABLE_WIFI
extern EnumSetting*   wifi_sta_mode;
extern IPaddrSetting* wifi_sta_ip;
extern IPaddrSetting* wifi_sta_gateway;
extern IPaddrSetting* wifi_sta_netmask;

//XXX for compatibility, implement wifi_sta_ip_gw_mk;

extern StringSetting* wifi_ap_ssid;
extern StringSetting* wifi_ap_password;

extern IPaddrSetting* wifi_ap_ip;

extern IntSetting* wifi_ap_channel;

extern StringSetting* wifi_hostname;
extern EnumSetting* http_enable;
extern IntSetting* http_port;
extern EnumSetting* telnet_enable;
extern IntSetting* telnet_port;
#endif

#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)
extern EnumSetting* wifi_radio_mode;
#endif

#ifdef ENABLE_BLUETOOTH
extern StringSetting* bt_name;
#endif

#ifdef ENABLE_AUTHENTICATION
// XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
extern StringSetting* user_password;
extern StringSetting* admin_password;
#endif

#ifdef ENABLE_NOTIFICATIONS
extern EnumSetting* notification_type;
extern StringSetting* notification_t1;
extern StringSetting* notification_t2;
extern StringSetting* notification_ts;
#endif

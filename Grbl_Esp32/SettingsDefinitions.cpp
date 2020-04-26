#include "SettingsDerivedClasses.h"
#include "defaults.h"

Setting *SettingsList = NULL;

// GRBL Settings

AxisSettings x_axis_settings = {
    "X",
    DEFAULT_X_STEPS_PER_MM,
    DEFAULT_X_MAX_RATE,
    DEFAULT_X_ACCELERATION,
    DEFAULT_X_MAX_TRAVEL,
    DEFAULT_X_CURRENT,
    DEFAULT_X_HOLD_CURRENT,
    DEFAULT_X_MICROSTEPS,
    DEFAULT_X_STALLGUARD
};

AxisSettings y_axis_settings = {
    "Y",
    DEFAULT_Y_STEPS_PER_MM,
    DEFAULT_Y_MAX_RATE,
    DEFAULT_Y_ACCELERATION,
    DEFAULT_Y_MAX_TRAVEL,
    DEFAULT_Y_CURRENT,
    DEFAULT_Y_HOLD_CURRENT,
    DEFAULT_Y_MICROSTEPS,
    DEFAULT_Y_STALLGUARD
};

AxisSettings z_axis_settings = {
    "Z",
    DEFAULT_Z_STEPS_PER_MM,
    DEFAULT_Z_MAX_RATE,
    DEFAULT_Z_ACCELERATION,
    DEFAULT_Z_MAX_TRAVEL,
    DEFAULT_Z_CURRENT,
    DEFAULT_Z_HOLD_CURRENT,
    DEFAULT_Z_MICROSTEPS,
    DEFAULT_Z_STALLGUARD
};

AxisSettings a_axis_settings = {
    "A",
    DEFAULT_A_STEPS_PER_MM,
    DEFAULT_A_MAX_RATE,
    DEFAULT_A_ACCELERATION,
    DEFAULT_A_MAX_TRAVEL,
    DEFAULT_A_CURRENT,
    DEFAULT_A_HOLD_CURRENT,
    DEFAULT_A_MICROSTEPS,
    DEFAULT_A_STALLGUARD
};

AxisSettings b_axis_settings = {
    "B",
    DEFAULT_B_STEPS_PER_MM,
    DEFAULT_B_MAX_RATE,
    DEFAULT_B_ACCELERATION,
    DEFAULT_B_MAX_TRAVEL,
    DEFAULT_B_CURRENT,
    DEFAULT_B_HOLD_CURRENT,
    DEFAULT_B_MICROSTEPS,
    DEFAULT_B_STALLGUARD
};

AxisSettings c_axis_settings = {
    "C",
    DEFAULT_C_STEPS_PER_MM,
    DEFAULT_C_MAX_RATE,
    DEFAULT_C_ACCELERATION,
    DEFAULT_C_MAX_TRAVEL,
    DEFAULT_C_CURRENT,
    DEFAULT_C_HOLD_CURRENT,
    DEFAULT_C_MICROSTEPS,
    DEFAULT_C_STALLGUARD
};

AxisSettings axis_settings[] = {
  x_axis_settings,
  y_axis_settings,
  z_axis_settings,
  a_axis_settings,
  b_axis_settings,
  c_axis_settings,
};

StringSetting startup_line_0("N0", "");
StringSetting startup_line_1("N1", "");
StringSetting build_info("I", "");

IntSetting pulse_microseconds("STEP_PULSE", DEFAULT_STEP_PULSE_MICROSECONDS, 3, 1000);
IntSetting stepper_idle_lock_time("STEPPER_IDLE_TIME", DEFAULT_STEPPER_IDLE_LOCK_TIME, 0, 255);

IntSetting step_invert_mask("STEP_INVERT_MASK", DEFAULT_STEPPING_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
IntSetting dir_invert_mask("DIR_INVERT_MASK", DEFAULT_DIRECTION_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
// XXX need to call st_generate_step_invert_masks()
IntSetting homing_dir_mask("HOMING_DIR_INVERT_MASK", DEFAULT_HOMING_DIR_MASK, 0, (1<<MAX_N_AXIS)-1);

FlagSetting step_enable_invert("STEP_ENABLE_INVERT", DEFAULT_INVERT_ST_ENABLE);
FlagSetting limit_invert("LIMIT_INVERT", DEFAULT_INVERT_LIMIT_PINS);
FlagSetting probe_invert("PROBE_INVERT", DEFAULT_INVERT_PROBE_PIN);
FlagSetting report_inches("REPORT_INCHES", DEFAULT_REPORT_INCHES);
FlagSetting soft_limits("SOFT_LIMITS", DEFAULT_SOFT_LIMIT_ENABLE, check_homing_enable);
// XXX need to check for HOMING_ENABLE
FlagSetting hard_limits("HARD_LIMITS", DEFAULT_HARD_LIMIT_ENABLE, limits_init);
// XXX need to call limits_init();
FlagSetting homing_enable("HOMING_ENABLE", DEFAULT_HOMING_ENABLE, also_soft_limit);
// XXX also need to clear, but not set, BITFLAG_SOFT_LIMIT_ENABLE
FlagSetting laser_mode("LASER_MODE", DEFAULT_LASER_MODE, my_spindle_init);
// XXX also need to call my_spindle->init();

IntSetting status_mask("STATUS_MASK", DEFAULT_STATUS_REPORT_MASK, 0, 2);
FloatSetting junction_deviation("JUNCTION_DEVIATION", DEFAULT_JUNCTION_DEVIATION, 0, 10);
FloatSetting arc_tolerance("ARC_TOLERANCE", DEFAULT_ARC_TOLERANCE, 0, 1);

FloatSetting homing_feed_rate("HOMING_FEED", DEFAULT_HOMING_FEED_RATE, 0, 10000);
FloatSetting homing_seek_rate("HOMING_SEEK", DEFAULT_HOMING_SEEK_RATE, 0, 10000);
FloatSetting homing_debounce("HOMING_DEBOUNCE", DEFAULT_HOMING_DEBOUNCE_DELAY, 0, 10000);
FloatSetting homing_pulloff("HOMING_PULLOFF", DEFAULT_HOMING_PULLOFF, 0, 1000);
FloatSetting spindle_pwm_freq("SPINDLE_PWM_FREQ", DEFAULT_SPINDLE_FREQ, 0, 100000);
FloatSetting rpm_max("RPM_MAX", DEFAULT_SPINDLE_RPM_MAX, 0, 100000);
FloatSetting rpm_min("RPM_MIN", DEFAULT_SPINDLE_RPM_MIN, 0, 100000);

FloatSetting spindle_pwm_off_value("SPINDLE_PWM_OFF_VALUE", DEFAULT_SPINDLE_OFF_VALUE, 0.0, 100.0); // these are percentages
FloatSetting spindle_pwm_min_value("SPINDLE_PWM_MIN_VALUE", DEFAULT_SPINDLE_MIN_VALUE, 0.0, 100.0);
FloatSetting spindle_pwm_max_value("SPINDLE_PWM_MAX_VALUE", DEFAULT_SPINDLE_MAX_VALUE, 0.0, 100.0);
IntSetting spindle_pwm_bit_precision("SPINDLE_PWM_BIT_PRECISION", DEFAULT_SPINDLE_BIT_PRECISION, 1, 16);

StringSetting spindle_type("SPINDLE_TYPE", DEFAULT_SPINDLE_TYPE);

// These are compatibily aliases for Classic GRBL
std::map<const char*, Setting*> numberedSettings = {
    { "0", &pulse_microseconds },
    { "1", &stepper_idle_lock_time },
    { "2", &step_invert_mask },
    { "3", &dir_invert_mask },
    { "4", &step_enable_invert },
    { "5", &limit_invert },
    { "6", &probe_invert },
    { "10", &status_mask },
    { "11", &junction_deviation },
    { "12", &arc_tolerance },
    { "13", &report_inches },
    { "20", &soft_limits },
    { "21", &hard_limits },
    { "22", &homing_enable },
    { "23", &homing_dir_mask },
    { "24", &homing_feed_rate },
    { "25", &homing_seek_rate },
    { "26", &homing_debounce },
    { "27", &homing_pulloff },
    { "30", &rpm_max },
    { "31", &rpm_min },
    { "32", &laser_mode },
    { "100", &x_axis_settings.steps_per_mm },
    { "101", &y_axis_settings.steps_per_mm },
    { "102", &z_axis_settings.steps_per_mm },
    { "110", &x_axis_settings.max_rate },
    { "111", &y_axis_settings.max_rate },
    { "112", &z_axis_settings.max_rate },
    { "120", &x_axis_settings.acceleration },
    { "121", &y_axis_settings.acceleration },
    { "122", &z_axis_settings.acceleration },
    { "130", &x_axis_settings.max_travel },
    { "131", &y_axis_settings.max_travel },
    { "132", &z_axis_settings.max_travel },
};

// WebUI Settings

std::map<const char*, int8_t> onoffOptions = {
  {  "OFF", 0, },
  { "ON", 1, }
};

#ifdef ENABLE_WIFI
StringSetting wifi_sta_ssid("Station SSID", "STA_SSID", DEFAULT_STA_SSID, WiFiConfig::isSSIDValid);
StringSetting wifi_sta_password("Station Password", "STA_PWD", DEFAULT_STA_PWD, WiFiConfig::isPasswordValid);
// XXX hack StringSetting class to return a ***** password if checker is isPasswordValid

std::map<const char*, int8_t> staModeOptions = {
    { "DHCP",   DHCP_MODE , }
    { "Static", STATIC_MODE , }
};
EnumSetting   wifi_sta_mode("Station IP Mode", "STA_IP_MODE", DEFAULT_STA_IP_MODE, staModeOptions);
IPaddrSetting wifi_sta_ip("Station Static IP", "STA_IP", DEFAULT_STA_IP);
IPaddrSetting wifi_sta_gateway("Station Static Gateway", "STA_GW", DEFAULT_STA_GW);
IPaddrSetting wifi_sta_netmask("Station Static Mask", "STA_MK", DEFAULT_STA_MK);

//XXX for compatibility, implement wifi_sta_ip_gw_mk()

StringSetting wifi_ap_ssid("AP SSID", "AP_SSID", DEFAULT_AP_SSID, WiFiConfig::isSSIDValid);
StringSetting wifi_ap_password("AP Password", "AP_PWD", DEFAULT_AP_PWD, WiFiConfig::isPasswordValid);

IPaddrSetting wifi_ap_ip("AP Static IP", "AP_IP", DEFAULT_AP_IP);

IntSetting wifi_ap_channel("AP Channel", "AP_CHANNEL", DEFAULT_AP_CHANNEL, MIN_CHANNEL, MAX_CHANNEL);

StringSetting wifi_hostname("Hostname", "ESP_HOSTNAME", DEFAULT_HOSTNAME, WiFiConfig::isHostnameValid);
EnumSetting http_enable("HTTP protocol", "HTTP_ON", DEFAULT_HTTP_STATE, onoffOptions);
IntSetting http_port(NULL, "HTTP_PORT", DEFAULT_HTTP_PORT, MIN_HTTP_PORT, MAX_HTTP_PORT);
EnumSetting telnet_enable("Telnet protocol", "TELNET_ON", DEFAULT_TELNET_STATE, onoffOptions);
IntSetting telnet_port(NULL, "TELNET_PORT", DEFAULT_TELNET_PORT, MIN_TELNET_PORT, MAX_TELNET_PORT);

#endif
std::map<const char*, int8_t> radioOptions = {
    { "None", ESP_RADIO_OFF, },
    { "STA", ESP_WIFI_STA, },
    { "AP", ESP_WIFI_AP, },
    { "BT", ESP_BT, },
};
std::map<const char*, int8_t> radioEnabledOptions = {
    { "NONE", ESP_RADIO_OFF, },
#ifdef ENABLE_WIFI
    { "STA", ESP_WIFI_STA, },
    { "AP", ESP_WIFI_AP, },
#endif
#ifdef ENABLE_BLUETOOTH
    { "BT", ESP_BT, },
#endif
};

#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)

EnumSetting wifi_radio_mode("Radio mode", "RADIO_MODE", DEFAULT_RADIO_MODE, radioEnabledOptions);

#endif

#ifdef ENABLE_BLUETOOTH
StringSetting bt_name("Bluetooth name", "BT_NAME", DEFAULT_BT_NAME, BTConfig::isBTnameValid);
#endif

#ifdef ENABLE_AUTHENTICATION
// XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
StringSetting user_password("USER_PWD", DEFAULT_USER_PWD, isLocalPasswordValid);
StringSetting admin_password("ADMIN_PWD", DEFAULT_ADMIN_PWD, isLocalPasswordValid);
#endif

#ifdef ENABLE_NOTIFICATIONS
std::map<const char*, int8_t> notificationOptions = {
    { "NONE", 0, }
    { "LINE", 3, }
    { "PUSHOVER", 1, }
    { "EMAIL", 2, }
};

EnumSetting notification_type("Notification type", "NOTIF_TYPE", DEFAULT_NOTIFICATION_TYPE, notificationOptions);
StringSetting notification_t1("Notification Token 1", "NOTIF_T1", DEFAULT_TOKEN , MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH);
StringSetting notification_t2("Notification Token 2", "NOTIF_T2", DEFAULT_TOKEN, MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH);
StringSetting notification_ts("Notification Settings", "NOTIF_T2", DEFAULT_TOKEN, 0, MAX_NOTIFICATION_SETTING_LENGTH);
#endif

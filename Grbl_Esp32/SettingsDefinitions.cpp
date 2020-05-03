#include "grbl.h"
#ifdef NEW_SETTINGS
#include "SettingsDerivedClasses.h"

Setting *SettingsList = NULL;

StringSetting* startup_line_0;
StringSetting* startup_line_1;
StringSetting* build_info;

IntSetting* pulse_microseconds;
IntSetting* stepper_idle_lock_time;

IntSetting* step_invert_mask;
IntSetting* dir_invert_mask;
// XXX need to call st_generate_step_invert_masks;
IntSetting* homing_dir_mask;

FlagSetting* step_enable_invert;
FlagSetting* limit_invert;
FlagSetting* probe_invert;
FlagSetting* report_inches;
FlagSetting* soft_limits;
// XXX need to check for HOMING_ENABLE
FlagSetting* hard_limits;
// XXX need to call limits_init;
FlagSetting* homing_enable;
// XXX also need to clear, but not set, BITFLAG_SOFT_LIMIT_ENABLE
FlagSetting* laser_mode;
// XXX also need to call my_spindle->init;

IntSetting* status_mask;
FloatSetting* junction_deviation;
FloatSetting* arc_tolerance;

FloatSetting* homing_feed_rate;
FloatSetting* homing_seek_rate;
FloatSetting* homing_debounce;
FloatSetting* homing_pulloff;
FloatSetting* spindle_pwm_freq;
FloatSetting* rpm_max;
FloatSetting* rpm_min;

FloatSetting* spindle_pwm_off_value;
FloatSetting* spindle_pwm_min_value;
FloatSetting* spindle_pwm_max_value;
IntSetting* spindle_pwm_bit_precision;

EnumSetting* spindle_type;

StringSetting* wifi_sta_ssid;
StringSetting* wifi_sta_password;
// XXX hack StringSetting class to return a ***** password if checker is isPasswordValid

#ifdef ENABLE_WIFI
EnumSetting*   wifi_sta_mode;
IPaddrSetting* wifi_sta_ip;
IPaddrSetting* wifi_sta_gateway;
IPaddrSetting* wifi_sta_netmask;

//XXX for compatibility, implement wifi_sta_ip_gw_mk;

StringSetting* wifi_ap_ssid;
StringSetting* wifi_ap_password;

IPaddrSetting* wifi_ap_ip;

IntSetting* wifi_ap_channel;

StringSetting* wifi_hostname;
EnumSetting* http_enable;
IntSetting* http_port;
EnumSetting* telnet_enable;
IntSetting* telnet_port;
std::map<const char*, int8_t> staModeOptions = {
    { "DHCP",   DHCP_MODE , },
    { "Static", STATIC_MODE , },
};
#endif

#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)
EnumSetting* wifi_radio_mode;
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
#endif

#ifdef ENABLE_BLUETOOTH
StringSetting* bt_name;
#endif

#ifdef ENABLE_AUTHENTICATION
// XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
StringSetting* user_password;
StringSetting* admin_password;
#endif

#ifdef ENABLE_NOTIFICATIONS
std::map<const char*, int8_t> notificationOptions = {
    { "NONE", 0, },
    { "LINE", 3, },
    { "PUSHOVER", 1, },
    { "EMAIL", 2, },
};
EnumSetting* notification_type;
StringSetting* notification_t1;
StringSetting* notification_t2;
StringSetting* notification_ts;
#endif
std::map<const char*, int8_t> onoffOptions = {
  {  "OFF", 0, },
  { "ON", 1, }
};
std::map<const char*, int8_t> spindleTypes = {
  { "NONE", SPINDLE_TYPE_NONE, },
  { "PWM", SPINDLE_TYPE_PWM, },
  { "RELAY", SPINDLE_TYPE_RELAY, },
  { "LASER", SPINDLE_TYPE_LASER, },
  { "DAC", SPINDLE_TYPE_DAC, },
  { "HUANYANG", SPINDLE_TYPE_HUANYANG, },
  { "BESC", SPINDLE_TYPE_BESC, },
};

AxisSettings* x_axis_settings;
AxisSettings* y_axis_settings;
AxisSettings* z_axis_settings;
AxisSettings* a_axis_settings;
AxisSettings* b_axis_settings;
AxisSettings* c_axis_settings;

AxisSettings* axis_settings[MAX_N_AXIS];

void make_settings()
{
    AxisSettings** p = axis_settings; 
    *p++ = x_axis_settings = new AxisSettings {
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

    *p++ = y_axis_settings = new AxisSettings {
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

    *p++ = z_axis_settings = new AxisSettings {
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

    *p++ = a_axis_settings = new AxisSettings {
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

    *p++ = b_axis_settings = new AxisSettings {
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

    *p++ = c_axis_settings = new AxisSettings {
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

    // GRBL Settings
    startup_line_0 = new StringSetting("N0", "");

    startup_line_1 = new StringSetting("N1", "");
    build_info = new StringSetting("I", "");

    pulse_microseconds = new IntSetting("STEP_PULSE", DEFAULT_STEP_PULSE_MICROSECONDS, 3, 1000);

    stepper_idle_lock_time = new IntSetting("STEPPER_IDLE_TIME", DEFAULT_STEPPER_IDLE_LOCK_TIME, 0, 255);

    step_invert_mask = new IntSetting("STEP_INVERT_MASK", DEFAULT_STEPPING_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
    dir_invert_mask = new IntSetting("DIR_INVERT_MASK", DEFAULT_DIRECTION_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);

    // XXX need to call st_generate_step_invert_masks()
    homing_dir_mask = new IntSetting("HOMING_DIR_INVERT_MASK", DEFAULT_HOMING_DIR_MASK, 0, (1<<MAX_N_AXIS)-1);


    step_enable_invert = new FlagSetting("STEP_ENABLE_INVERT", DEFAULT_INVERT_ST_ENABLE);
    limit_invert = new FlagSetting("LIMIT_INVERT", DEFAULT_INVERT_LIMIT_PINS);
    probe_invert = new FlagSetting("PROBE_INVERT", DEFAULT_INVERT_PROBE_PIN);
    report_inches = new FlagSetting("REPORT_INCHES", DEFAULT_REPORT_INCHES);
    soft_limits = new FlagSetting("SOFT_LIMITS", DEFAULT_SOFT_LIMIT_ENABLE);
    // XXX need to check for HOMING_ENABLE
    hard_limits = new FlagSetting("HARD_LIMITS", DEFAULT_HARD_LIMIT_ENABLE);
    // XXX need to call limits_init();
    homing_enable = new FlagSetting("HOMING_ENABLE", DEFAULT_HOMING_ENABLE);
    // XXX also need to clear, but not set, BITFLAG_SOFT_LIMIT_ENABLE
    laser_mode = new FlagSetting("LASER_MODE", DEFAULT_LASER_MODE);
    // XXX also need to call my_spindle->init();

    status_mask = new IntSetting("STATUS_MASK", DEFAULT_STATUS_REPORT_MASK, 0, 2);
    junction_deviation = new FloatSetting("JUNCTION_DEVIATION", DEFAULT_JUNCTION_DEVIATION, 0, 10);
    arc_tolerance = new FloatSetting("ARC_TOLERANCE", DEFAULT_ARC_TOLERANCE, 0, 1);

    homing_feed_rate = new FloatSetting("HOMING_FEED", DEFAULT_HOMING_FEED_RATE, 0, 10000);
    homing_seek_rate = new FloatSetting("HOMING_SEEK", DEFAULT_HOMING_SEEK_RATE, 0, 10000);
    homing_debounce = new FloatSetting("HOMING_DEBOUNCE", DEFAULT_HOMING_DEBOUNCE_DELAY, 0, 10000);
    homing_pulloff = new FloatSetting("HOMING_PULLOFF", DEFAULT_HOMING_PULLOFF, 0, 1000);
    spindle_pwm_freq = new FloatSetting("SPINDLE_PWM_FREQ", DEFAULT_SPINDLE_FREQ, 0, 100000);
    rpm_max = new FloatSetting("RPM_MAX", DEFAULT_SPINDLE_RPM_MAX, 0, 100000);
    rpm_min = new FloatSetting("RPM_MIN", DEFAULT_SPINDLE_RPM_MIN, 0, 100000);

    spindle_pwm_off_value = new FloatSetting("SPINDLE_PWM_OFF_VALUE", DEFAULT_SPINDLE_OFF_VALUE, 0.0, 100.0); // these are percentages
    spindle_pwm_min_value = new FloatSetting("SPINDLE_PWM_MIN_VALUE", DEFAULT_SPINDLE_MIN_VALUE, 0.0, 100.0);
    spindle_pwm_max_value = new FloatSetting("SPINDLE_PWM_MAX_VALUE", DEFAULT_SPINDLE_MAX_VALUE, 0.0, 100.0);
    // IntSetting spindle_pwm_bit_precision("SPINDLE_PWM_BIT_PRECISION", DEFAULT_SPINDLE_BIT_PRECISION, 1, 16);
    spindle_type = new EnumSetting(NULL, "SPINDLE_TYPE", SPINDLE_TYPE, spindleTypes);

    // WebUI Settings
#ifdef ENABLE_WIFI
    wifi_sta_ssid = new StringSetting("Station SSID", "STA_SSID", DEFAULT_STA_SSID, 0, 0, WiFiConfig::isSSIDValid);
    wifi_sta_password = new StringSetting("Station Password", "STA_PWD", DEFAULT_STA_PWD, 0, 0, WiFiConfig::isPasswordValid);
    // XXX hack StringSetting class to return a ***** password if checker is isPasswordValid

    wifi_sta_mode = new EnumSetting("Station IP Mode", "STA_IP_MODE", DEFAULT_STA_IP_MODE, staModeOptions);
    wifi_sta_ip = new IPaddrSetting("Station Static IP", "STA_IP", DEFAULT_STA_IP, NULL);
    wifi_sta_gateway = new IPaddrSetting("Station Static Gateway", "STA_GW", DEFAULT_STA_GW, NULL);
    wifi_sta_netmask = new IPaddrSetting("Station Static Mask", "STA_MK", DEFAULT_STA_MK, NULL);

    //XXX for compatibility, implement wifi_sta_ip_gw_mk()

    wifi_ap_ssid = new StringSetting("AP SSID", "AP_SSID", DEFAULT_AP_SSID, 0, 0, WiFiConfig::isSSIDValid);
    wifi_ap_password = new StringSetting("AP Password", "AP_PWD", DEFAULT_AP_PWD, 0, 0, WiFiConfig::isPasswordValid);

    wifi_ap_ip = new IPaddrSetting("AP Static IP", "AP_IP", DEFAULT_AP_IP, NULL);

    wifi_ap_channel = new IntSetting("AP Channel", "AP_CHANNEL", DEFAULT_AP_CHANNEL, MIN_CHANNEL, MAX_CHANNEL, NULL);

    wifi_hostname = new StringSetting("Hostname", "ESP_HOSTNAME", DEFAULT_HOSTNAME, 0, 0, WiFiConfig::isHostnameValid);
    http_enable = new EnumSetting("HTTP protocol", "HTTP_ON", DEFAULT_HTTP_STATE, onoffOptions);
    http_port = new IntSetting("HTTP Port", "HTTP_PORT", DEFAULT_WEBSERVER_PORT, MIN_HTTP_PORT, MAX_HTTP_PORT, NULL);
    telnet_enable = new EnumSetting("Telnet protocol", "TELNET_ON", DEFAULT_TELNET_STATE, onoffOptions);
    telnet_port = new IntSetting("Telnet Port", "TELNET_PORT", DEFAULT_TELNETSERVER_PORT, MIN_TELNET_PORT, MAX_TELNET_PORT, NULL);

#endif
#if defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH)

    wifi_radio_mode = new EnumSetting("Radio mode", "RADIO_MODE", DEFAULT_RADIO_MODE, radioEnabledOptions);

#endif

#ifdef ENABLE_BLUETOOTH
    bt_name = new StringSetting("Bluetooth name", "BT_NAME", DEFAULT_BT_NAME, 0, 0, BTConfig::isBTnameValid);
#endif

#ifdef ENABLE_AUTHENTICATION
    // XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
    user_password = new StringSetting("USER_PWD", DEFAULT_USER_PWD, isLocalPasswordValid);
    admin_password = new StringSetting("ADMIN_PWD", DEFAULT_ADMIN_PWD, isLocalPasswordValid);
#endif

#ifdef ENABLE_NOTIFICATIONS
    notification_type = new EnumSetting("Notification type", "NOTIF_TYPE", DEFAULT_NOTIFICATION_TYPE, notificationOptions);
    notification_t1 = new StringSetting("Notification Token 1", "NOTIF_T1", DEFAULT_TOKEN , MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
    notification_t2 = new StringSetting("Notification Token 2", "NOTIF_T2", DEFAULT_TOKEN, MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
    notification_ts = new StringSetting("Notification Settings", "NOTIF_T2", DEFAULT_TOKEN, 0, MAX_NOTIFICATION_SETTING_LENGTH, NULL);
#endif
}
#endif // NEW_SETTINGS

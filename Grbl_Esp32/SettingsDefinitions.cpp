#include "grbl.h"

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
typedef std::map<const char *, int8_t, cmp_str> enum_opt_t;
enum_opt_t staModeOptions = {
    { "DHCP",   DHCP_MODE , },
    { "Static", STATIC_MODE , },
};
#endif

#if defined( ENABLE_WIFI) ||  defined( ENABLE_BLUETOOTH)
EnumSetting* wifi_radio_mode;
enum_opt_t radioOptions = {
    { "None", ESP_RADIO_OFF, },
    { "STA", ESP_WIFI_STA, },
    { "AP", ESP_WIFI_AP, },
    { "BT", ESP_BT, },
};
enum_opt_t radioEnabledOptions = {
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
enum_opt_t notificationOptions = {
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
enum_opt_t onoffOptions = {
  { "OFF", 0, },
  { "ON", 1, }
};
enum_opt_t spindleTypes = {
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

typedef struct {
    const char *name;
    float steps_per_mm;
    float max_rate;
    float acceleration;
    float max_travel;
    float run_current;
    float hold_current;
    uint16_t microsteps;
    uint16_t stallguard;
} axis_defaults_t;
axis_defaults_t axis_defaults[] = {
    {
        "x",
        DEFAULT_X_STEPS_PER_MM,
        DEFAULT_X_MAX_RATE,
        DEFAULT_X_ACCELERATION / SEC_PER_MIN_SQ,
        DEFAULT_X_MAX_TRAVEL,
        DEFAULT_X_CURRENT,
        DEFAULT_X_HOLD_CURRENT,
        DEFAULT_X_MICROSTEPS,
        DEFAULT_X_STALLGUARD
    },
    {
        "y",
        DEFAULT_Y_STEPS_PER_MM,
        DEFAULT_Y_MAX_RATE,
        DEFAULT_Y_ACCELERATION / SEC_PER_MIN_SQ,
        DEFAULT_Y_MAX_TRAVEL,
        DEFAULT_Y_CURRENT,
        DEFAULT_Y_HOLD_CURRENT,
        DEFAULT_Y_MICROSTEPS,
        DEFAULT_Y_STALLGUARD
    },
    {
        "z",
        DEFAULT_Z_STEPS_PER_MM,
        DEFAULT_Z_MAX_RATE,
        DEFAULT_Z_ACCELERATION / SEC_PER_MIN_SQ,
        DEFAULT_Z_MAX_TRAVEL,
        DEFAULT_Z_CURRENT,
        DEFAULT_Z_HOLD_CURRENT,
        DEFAULT_Z_MICROSTEPS,
        DEFAULT_Z_STALLGUARD
    },
    {
        "a",
        DEFAULT_A_STEPS_PER_MM,
        DEFAULT_A_MAX_RATE,
        DEFAULT_A_ACCELERATION / SEC_PER_MIN_SQ,
        DEFAULT_A_MAX_TRAVEL,
        DEFAULT_A_CURRENT,
        DEFAULT_A_HOLD_CURRENT,
        DEFAULT_A_MICROSTEPS,
        DEFAULT_A_STALLGUARD
    },
    {
        "b",
        DEFAULT_B_STEPS_PER_MM,
        DEFAULT_B_MAX_RATE,
        DEFAULT_B_ACCELERATION / SEC_PER_MIN_SQ,
        DEFAULT_B_MAX_TRAVEL,
        DEFAULT_B_CURRENT,
        DEFAULT_B_HOLD_CURRENT,
        DEFAULT_B_MICROSTEPS,
        DEFAULT_B_STALLGUARD
    },
    {
        "c",
        DEFAULT_C_STEPS_PER_MM,
        DEFAULT_C_MAX_RATE,
        DEFAULT_C_ACCELERATION  / SEC_PER_MIN_SQ,
        DEFAULT_C_MAX_TRAVEL,
        DEFAULT_C_CURRENT,
        DEFAULT_C_HOLD_CURRENT,
        DEFAULT_C_MICROSTEPS,
        DEFAULT_C_STALLGUARD
    }
};

// Construct e.g. X_MAX_RATE from axisName "X" and tail "_MAX_RATE"
// in dynamically allocated memory that will not be freed.
static const char *makename(const char *axisName, const char *tail) {
    char *retval = (char *)malloc(strlen(axisName) + strlen(tail) + 1);
    strcpy(retval, axisName);
    return strcat(retval, tail);
}

// Generates a string like "122" from axisNum 2 and base 120
static const char *makeGrblName(int axisNum, int base) {
    // To omit A,B,C axes:
    // if (axisNum > 2) return NULL;
    char buf[4];
    snprintf(buf, 4, "%d", axisNum + base);
    char *retval = (char *)malloc(strlen(buf));
    return strcpy(retval, buf);
}

void make_settings()
{
    if (!_handle) {
        if (esp_err_t err = nvs_open(NVS_PARTITION_NAME, NVS_READWRITE, &_handle)) {
            grbl_sendf(CLIENT_SERIAL, "nvs_open failed with error %d\r\n", err);
        }
    }
    // WebUI Settings
    #ifdef ENABLE_NOTIFICATIONS
        notification_ts = new StringSetting(NULL, WEBUI, "Notification Settings", "NotifyTS", DEFAULT_TOKEN, 0, MAX_NOTIFICATION_SETTING_LENGTH, NULL);
        notification_t2 = new StringSetting(NULL, WEBUI, "Notification Token 2", "NotifyT2", DEFAULT_TOKEN, MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_t1 = new StringSetting(NULL, WEBUI, "Notification Token 1", "NotifyT1", DEFAULT_TOKEN , MIN_NOTIFICATION_TOKEN_LENGTH, MAX_NOTIFICATION_TOKEN_LENGTH, NULL);
        notification_type = new EnumSetting(NULL, WEBUI, "Notification type", "NotifyType", DEFAULT_NOTIFICATION_TYPE, &notificationOptions);
    #endif

    #ifdef ENABLE_AUTHENTICATION
        // XXX need ADMIN_ONLY and if it is called without a parameter it sets the default
        admin_password = new StringSetting(WEBUI, NULL, "AdminPwd", DEFAULT_ADMIN_PWD, isLocalPasswordValid);
        user_password = new StringSetting(WEBUI, NULL, "UserPwd", DEFAULT_USER_PWD, isLocalPasswordValid);
    #endif

    #ifdef ENABLE_BLUETOOTH
        bt_name = new StringSetting("Bluetooth name", WEBUI, "ESP140", "BTName", DEFAULT_BT_NAME, 0, 0, BTConfig::isBTnameValid);
    #endif

    #if defined(ENABLE_WIFI) || defined(ENABLE_BLUETOOTH)
        wifi_radio_mode = new EnumSetting("Radio mode", WEBUI, "ESP110", "RadioMode", DEFAULT_RADIO_MODE, &radioEnabledOptions);
    #endif

    #ifdef ENABLE_WIFI
        telnet_port = new IntSetting("Telnet Port", WEBUI, "ESP131", "TelnetPort", DEFAULT_TELNETSERVER_PORT, MIN_TELNET_PORT, MAX_TELNET_PORT, NULL);
        telnet_enable = new EnumSetting("Telnet protocol", WEBUI, "ESP130", "TelnetOn", DEFAULT_TELNET_STATE, &onoffOptions);
        http_enable = new EnumSetting("HTTP protocol", WEBUI, "ESP120", "HTTPOn", DEFAULT_HTTP_STATE, &onoffOptions);
        http_port = new IntSetting("HTTP Port", WEBUI, "ESP121", "HTTPPort", DEFAULT_WEBSERVER_PORT, MIN_HTTP_PORT, MAX_HTTP_PORT, NULL);
        wifi_hostname = new StringSetting("Hostname", WEBUI, "ESP112", "ESPHostname", DEFAULT_HOSTNAME, 0, 0, WiFiConfig::isHostnameValid);
        wifi_ap_channel = new IntSetting("AP Channel", WEBUI, "ESP108", "APChannel", DEFAULT_AP_CHANNEL, MIN_CHANNEL, MAX_CHANNEL, NULL);
        wifi_ap_ip = new IPaddrSetting("AP Static IP", WEBUI, "ESP107", "APIP", DEFAULT_AP_IP, NULL);
        wifi_ap_password = new StringSetting("AP Password", WEBUI, "ESP106", "APPassword", DEFAULT_AP_PWD, 0, 0, WiFiConfig::isPasswordValid);
        wifi_ap_ssid = new StringSetting("AP SSID", WEBUI, "ESP105", "ApSSID", DEFAULT_AP_SSID, 0, 0, WiFiConfig::isSSIDValid);
        //XXX for compatibility, implement wifi_sta_ip_gw_mk()
        wifi_sta_netmask = new IPaddrSetting("Station Static Mask", WEBUI, NULL, "StaNetmask", DEFAULT_STA_MK, NULL);
        wifi_sta_gateway = new IPaddrSetting("Station Static Gateway", WEBUI, NULL, "StaGateway", DEFAULT_STA_GW, NULL);
        wifi_sta_ip = new IPaddrSetting("Station Static IP", WEBUI, NULL, "StaIP", DEFAULT_STA_IP, NULL);
        wifi_sta_mode = new EnumSetting("Station IP Mode", WEBUI, "ESP102", "StaIPMode", DEFAULT_STA_IP_MODE, &staModeOptions);
        // XXX hack StringSetting class to return a ***** password if checker is isPasswordValid
        wifi_sta_password = new StringSetting("Station Password", WEBUI, "ESP101", "StaPwd", DEFAULT_STA_PWD, 0, 0, WiFiConfig::isPasswordValid);
        wifi_sta_ssid = new StringSetting("Station SSID", WEBUI, "ESP100", "StaSSID", DEFAULT_STA_SSID, 0, 0, WiFiConfig::isSSIDValid);
    #endif

    // The following horrid code accomodates people who insist that axis settings
    // be grouped by setting type rather than by axis.
    int axis;
    axis_defaults_t* def;
    for (axis = 0; axis < N_AXIS; axis++) {
        def = &axis_defaults[axis];
        axis_settings[axis] = new AxisSettings(def->name);
    }
    AxisSettings** p = axis_settings; 
    x_axis_settings = axis_settings[X_AXIS];
    y_axis_settings = axis_settings[Y_AXIS];
    z_axis_settings = axis_settings[Z_AXIS];
    a_axis_settings = axis_settings[A_AXIS];
    b_axis_settings = axis_settings[B_AXIS];
    c_axis_settings = axis_settings[C_AXIS];
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new IntSetting(EXTENDED, makeGrblName(axis, 170), makename(def->name, "StallGuard"), def->stallguard, 0, 100);
        setting->setAxis(axis); 
        axis_settings[axis]->stallguard = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new IntSetting(EXTENDED, makeGrblName(axis, 160), makename(def->name, "Microsteps"), def->microsteps, 0, 256);
        setting->setAxis(axis);
        axis_settings[axis]->microsteps = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(EXTENDED, makeGrblName(axis, 150), makename(def->name, "HoldCurrent"), def->hold_current, 0.0, 100.0);
        setting->setAxis(axis);
        axis_settings[axis]->hold_current = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(EXTENDED, makeGrblName(axis, 140), makename(def->name, "RunCurrent"), def->run_current, 0.05, 20.0);
        setting->setAxis(axis);
        axis_settings[axis]->run_current = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, makeGrblName(axis, 130), makename(def->name, "MaxTravel"), def->max_travel, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->max_travel = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, makeGrblName(axis, 120), makename(def->name, "Acceleration"), def->acceleration, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->acceleration = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, makeGrblName(axis, 110), makename(def->name, "MaxRate"), def->max_rate, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->max_rate = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, makeGrblName(axis, 100), makename(def->name, "StepsPerMm"), def->steps_per_mm, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->steps_per_mm = setting;
    }
    // End of horrid code

    // Spindle Settings
    spindle_pwm_max_value = new FloatSetting(EXTENDED, "36", "SpindleMaxPWM", DEFAULT_SPINDLE_MAX_VALUE, 0.0, 100.0);
    spindle_pwm_min_value = new FloatSetting(EXTENDED, "35", "SpindleMinPWM", DEFAULT_SPINDLE_MIN_VALUE, 0.0, 100.0);
    spindle_pwm_off_value = new FloatSetting(EXTENDED, "34", "SpindleOffPWM", DEFAULT_SPINDLE_OFF_VALUE, 0.0, 100.0); // these are percentages
    // IntSetting spindle_pwm_bit_precision("SpindlePWMbitPrecision", DEFAULT_SPINDLE_BIT_PRECISION, 1, 16);
    spindle_type = new EnumSetting(NULL, EXTENDED, "33", "SpindleType", SPINDLE_TYPE, &spindleTypes);

    // GRBL Non-numbered settings
    startup_line_0 = new StringSetting(GRBL, NULL, "N0", "");
    startup_line_1 = new StringSetting(GRBL, NULL, "N1", "");
    build_info = new StringSetting(GRBL, NULL, "I", "");

    // GRBL Numbered Settings
    laser_mode = new FlagSetting(GRBL, "32", "LaserMode", DEFAULT_LASER_MODE);
    // XXX also need to call my_spindle->init();
    rpm_min = new FloatSetting(GRBL, "31", "RpmMin", DEFAULT_SPINDLE_RPM_MIN, 0, 100000);
    rpm_max = new FloatSetting(GRBL, "30", "RpmMax", DEFAULT_SPINDLE_RPM_MAX, 0, 100000);


    homing_pulloff = new FloatSetting(GRBL, "27", "HomingPulloff", DEFAULT_HOMING_PULLOFF, 0, 1000);
    homing_debounce = new FloatSetting(GRBL, "26", "HomingDebounce", DEFAULT_HOMING_DEBOUNCE_DELAY, 0, 10000);
    homing_seek_rate = new FloatSetting(GRBL, "25", "HomingSeek", DEFAULT_HOMING_SEEK_RATE, 0, 10000);
    homing_feed_rate = new FloatSetting(GRBL, "24", "HomingFeed", DEFAULT_HOMING_FEED_RATE, 0, 10000);

    // XXX need to call st_generate_step_invert_masks()
    homing_dir_mask = new IntSetting(GRBL, "23", "HomingDirInvertMask", DEFAULT_HOMING_DIR_MASK, 0, (1<<MAX_N_AXIS)-1);
    // XXX need to call limits_init();
    homing_enable = new FlagSetting(GRBL, "22", "HomingEnable", DEFAULT_HOMING_ENABLE);
    // XXX need to check for HOMING_ENABLE
    hard_limits = new FlagSetting(GRBL, "21", "HardLimits", DEFAULT_HARD_LIMIT_ENABLE);
    soft_limits = new FlagSetting(GRBL, "20", "SoftLimits", DEFAULT_SOFT_LIMIT_ENABLE, NULL);

    report_inches = new FlagSetting(GRBL, "13", "ReportInches", DEFAULT_REPORT_INCHES);
    // XXX also need to clear, but not set, BITFLAG_SOFT_LIMIT_ENABLE
    arc_tolerance = new FloatSetting(GRBL, "12", "ArcTolerance", DEFAULT_ARC_TOLERANCE, 0, 1);
    junction_deviation = new FloatSetting(GRBL, "11", "JunctionDeviation", DEFAULT_JUNCTION_DEVIATION, 0, 10);
    status_mask = new IntSetting(GRBL, "10", "StatusMask", DEFAULT_STATUS_REPORT_MASK, 0, 2);

    probe_invert = new FlagSetting(GRBL, "6", "ProbeInvert", DEFAULT_INVERT_PROBE_PIN);
    limit_invert = new FlagSetting(GRBL, "5", "LimitInvert", DEFAULT_INVERT_LIMIT_PINS);
    step_enable_invert = new FlagSetting(GRBL, "4", "StepEnableInvert", DEFAULT_INVERT_ST_ENABLE);
    dir_invert_mask = new IntSetting(GRBL, "3", "DirInvertMask", DEFAULT_DIRECTION_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
    step_invert_mask = new IntSetting(GRBL, "2", "StepInvertMask", DEFAULT_STEPPING_INVERT_MASK, 0, (1<<MAX_N_AXIS)-1);
    stepper_idle_lock_time = new IntSetting(GRBL, "1", "StepperIdleTime", DEFAULT_STEPPER_IDLE_LOCK_TIME, 0, 255);
    pulse_microseconds = new IntSetting(GRBL, "0", "StepPulse", DEFAULT_STEP_PULSE_MICROSECONDS, 3, 1000);
    spindle_pwm_freq = new FloatSetting(EXTENDED, NULL, "SpindlePWMFreq", DEFAULT_SPINDLE_FREQ, 0, 100000);
}

err_t report_nvs_stats(uint8_t client) {
    nvs_stats_t stats;
    if( err_t err = nvs_get_stats(NULL, &stats)) {
        return err;
    }
    grbl_sendf(client, "[MSG: NVS Used: %d Free: %d Total: %d]\r\n",
        stats.used_entries, stats.free_entries, stats.total_entries);
    #if 0  // The SDK we use does not have this yet
        nvs_iterator_t it = nvs_entry_find(NULL, NULL, NVS_TYPE_ANY);
        while (it != NULL) {
            nvs_entry_info_t info;
            nvs_entry_info(it, &info);
            it = nvs_entry_next(it);
            grbl_sendf(client, "namespace %s key '%s', type '%d' \n", info.namespace_name, info.key, info.type);
        }
    #endif
    return STATUS_OK;
}

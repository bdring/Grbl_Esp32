#include "grbl.h"
#include "SettingsClass.h"

// Command *CommandsList = NULL;

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
// XXX also need to clear, but not set, soft_limits
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
        DEFAULT_X_ACCELERATION,
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
        DEFAULT_Y_ACCELERATION,
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
        DEFAULT_Z_ACCELERATION,
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
        DEFAULT_A_ACCELERATION,
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
        DEFAULT_B_ACCELERATION,
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
        DEFAULT_C_ACCELERATION,
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

static bool checkStartupLine(char* value) {
    // Modify the string in place to make it upper case,
    // because Grbl is case-insensitive for GCode, but it is
    // done by modifying incoming lines before interpretation.
    for (char* s = value; *s; s++) {
        char c = *s;
        if (islower(c)) {
            *s = toupper(c);
        }
    }
    return gc_execute_line(value, CLIENT_SERIAL) == 0;
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
    Setting::init();

    // Create the axis settings in the order that people are
    // accustomed to seeing.
    int axis;
    axis_defaults_t* def;
    for (axis = 0; axis < N_AXIS; axis++) {
        def = &axis_defaults[axis];
        axis_settings[axis] = new AxisSettings(def->name);
    }
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

    // Spindle Settings
    spindle_pwm_max_value = new FloatSetting(EXTENDED, "36", "SpindleMaxPWM", DEFAULT_SPINDLE_MAX_VALUE, 0.0, 100.0);
    spindle_pwm_min_value = new FloatSetting(EXTENDED, "35", "SpindleMinPWM", DEFAULT_SPINDLE_MIN_VALUE, 0.0, 100.0);
    spindle_pwm_off_value = new FloatSetting(EXTENDED, "34", "SpindleOffPWM", DEFAULT_SPINDLE_OFF_VALUE, 0.0, 100.0); // these are percentages
    // IntSetting spindle_pwm_bit_precision("SpindlePWMbitPrecision", DEFAULT_SPINDLE_BIT_PRECISION, 1, 16);
    spindle_type = new EnumSetting(NULL, EXTENDED, "33", "SpindleType", SPINDLE_TYPE_NONE, &spindleTypes);

    // GRBL Non-numbered settings
    startup_line_0 = new StringSetting(GRBL, NULL, "N0", "", checkStartupLine);
    startup_line_1 = new StringSetting(GRBL, NULL, "N1", "", checkStartupLine);

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
    // XXX also need to clear, but not set, soft_limits
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

err_t report_nvs_stats(const char* value, uint8_t client) {
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

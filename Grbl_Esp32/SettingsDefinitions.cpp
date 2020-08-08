#include "grbl.h"

bool motorSettingChanged = false;

StringSetting* startup_line_0;
StringSetting* startup_line_1;
StringSetting* build_info;

IntSetting* pulse_microseconds;
IntSetting* stepper_idle_lock_time;

AxisMaskSetting* step_invert_mask;
AxisMaskSetting* dir_invert_mask;
// TODO Settings - need to call st_generate_step_invert_masks;
AxisMaskSetting* homing_dir_mask;
AxisMaskSetting* stallguard_debug_mask;

FlagSetting* step_enable_invert;
FlagSetting* limit_invert;
FlagSetting* probe_invert;
FlagSetting* report_inches;
FlagSetting* soft_limits;
// TODO Settings - need to check for HOMING_ENABLE
FlagSetting* hard_limits;
// TODO Settings - need to call limits_init;
FlagSetting* homing_enable;
// TODO Settings - also need to clear, but not set, soft_limits
FlagSetting* laser_mode;
// TODO Settings - also need to call my_spindle->init;

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
FloatSetting* spindle_delay_spinup;
FloatSetting* spindle_delay_spindown;

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
    { "10V", SPINDLE_TYPE_10V, },
};
//Torch Height Control Settings
FlagSetting* thc_debug_setting;
FloatSetting* thc_voltage_setting;
IntSetting* thc_debugprint_millis;
IntSetting* thc_arc_delay_time;
FloatSetting* thc_voltage_filter_value;
IntSetting* thc_iter_freq;

AxisSettings* x_axis_settings;
AxisSettings* y_axis_settings;
AxisSettings* z_axis_settings;
AxisSettings* a_axis_settings;
AxisSettings* b_axis_settings;
AxisSettings* c_axis_settings;

AxisSettings* axis_settings[MAX_N_AXIS];

typedef struct {
    const char* name;
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
        "X",
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
        "Y",
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
        "Z",
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
        "A",
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
        "B",
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
        "C",
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
    char* retval = (char *)malloc(strlen(axisName) + strlen(tail) + 2);

    strcpy(retval, axisName);
    strcat(retval, "/");
    return strcat(retval, tail);
}

static bool checkStartupLine(char* value) {
    if (sys.state != STATE_IDLE)
        return STATUS_IDLE_ERROR;
    return gc_execute_line(value, CLIENT_SERIAL) == 0;
}

static bool checkStallguard(char* value) {
    motorSettingChanged = true;
    return true;
}

static bool checkMicrosteps(char* value) {
    motorSettingChanged = true;
    return true;
}

static bool checkRunCurrent(char* value) {
    motorSettingChanged = true;
    return true;
}

static bool checkHoldcurrent(char* value) {
    motorSettingChanged = true;
    return true;
}


static bool checkStallguardDebugMask(char* val) {
    motorSettingChanged = true;
    return true;
}

// Generates a string like "122" from axisNum 2 and base 120
static const char* makeGrblName(int axisNum, int base) {
    // To omit A,B,C axes:
    // if (axisNum > 2) return NULL;
    char buf[4];
    snprintf(buf, 4, "%d", axisNum + base);
    char* retval = (char*)malloc(strlen(buf));
    return strcpy(retval, buf);
}

void make_settings() {
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
        auto setting = new IntSetting(EXTENDED, WG, makeGrblName(axis, 170), makename(def->name, "StallGuard"), def->stallguard, -64, 63, checkStallguard);
        setting->setAxis(axis);
        axis_settings[axis]->stallguard = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new IntSetting(EXTENDED, WG, makeGrblName(axis, 160), makename(def->name, "Microsteps"), def->microsteps, 0, 256, checkMicrosteps);
        setting->setAxis(axis);
        axis_settings[axis]->microsteps = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(EXTENDED, WG, makeGrblName(axis, 150), makename(def->name, "Current/Hold"), def->hold_current, 0.05, 20.0, checkHoldcurrent); // Amps
        setting->setAxis(axis);
        axis_settings[axis]->hold_current = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(EXTENDED, WG, makeGrblName(axis, 140), makename(def->name, "Current/Run"), def->run_current, 0.0, 20.0, checkRunCurrent); // Amps
        setting->setAxis(axis);
        axis_settings[axis]->run_current = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, WG, makeGrblName(axis, 130), makename(def->name, "MaxTravel"), def->max_travel, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->max_travel = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, WG, makeGrblName(axis, 120), makename(def->name, "Acceleration"), def->acceleration, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->acceleration = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, WG, makeGrblName(axis, 110), makename(def->name, "MaxRate"), def->max_rate, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->max_rate = setting;
    }
    for (axis = N_AXIS - 1; axis >= 0; axis--) {
        def = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, WG, makeGrblName(axis, 100), makename(def->name, "StepsPerMm"), def->steps_per_mm, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->steps_per_mm = setting;
    }

    // Spindle Settings
    spindle_pwm_max_value = new FloatSetting(EXTENDED, WG, "36", "Spindle/PWM/Max", DEFAULT_SPINDLE_MAX_VALUE, 0.0, 100.0);
    spindle_pwm_min_value = new FloatSetting(EXTENDED, WG, "35", "Spindle/PWM/Min", DEFAULT_SPINDLE_MIN_VALUE, 0.0, 100.0);
    spindle_pwm_off_value = new FloatSetting(EXTENDED, WG, "34", "Spindle/PWM/Off", DEFAULT_SPINDLE_OFF_VALUE, 0.0, 100.0); // these are percentages
    // IntSetting spindle_pwm_bit_precision(EXTENDED, WG, "Spindle/PWM/Precision", DEFAULT_SPINDLE_BIT_PRECISION, 1, 16);
    spindle_pwm_freq = new FloatSetting(EXTENDED, WG, "33", "Spindle/PWM/Frequency", DEFAULT_SPINDLE_FREQ, 0, 100000);
    spindle_delay_spinup = new FloatSetting(EXTENDED, WG, NULL, "Spindle/Delay/SpinUp", DEFAULT_SPINDLE_DELAY_SPINUP, 0, 30);
    spindle_delay_spindown = new FloatSetting(EXTENDED, WG, NULL, "Spindle/Delay/SpinDown", DEFAULT_SPINDLE_DELAY_SPINUP, 0, 30);

    // GRBL Non-numbered settings
    startup_line_0 = new StringSetting(GRBL, WG, "N0", "GCode/Line0", "", checkStartupLine);
    startup_line_1 = new StringSetting(GRBL, WG, "N1", "GCode/Line1", "", checkStartupLine);

    // GRBL Numbered Settings
    laser_mode = new FlagSetting(GRBL, WG, "32", "GCode/LaserMode", DEFAULT_LASER_MODE);
    // TODO Settings - also need to call my_spindle->init();
    rpm_min = new FloatSetting(GRBL, WG, "31", "GCode/MinS", DEFAULT_SPINDLE_RPM_MIN, 0, 100000);
    rpm_max = new FloatSetting(GRBL, WG, "30", "GCode/MaxS", DEFAULT_SPINDLE_RPM_MAX, 0, 100000);


    homing_pulloff = new FloatSetting(GRBL, WG, "27", "Homing/Pulloff", DEFAULT_HOMING_PULLOFF, 0, 1000);
    homing_debounce = new FloatSetting(GRBL, WG, "26", "Homing/Debounce", DEFAULT_HOMING_DEBOUNCE_DELAY, 0, 10000);
    homing_seek_rate = new FloatSetting(GRBL, WG, "25", "Homing/Seek", DEFAULT_HOMING_SEEK_RATE, 0, 10000);
    homing_feed_rate = new FloatSetting(GRBL, WG, "24", "Homing/Feed", DEFAULT_HOMING_FEED_RATE, 0, 10000);

    // TODO Settings - need to call st_generate_step_invert_masks()
    homing_dir_mask = new AxisMaskSetting(GRBL, WG, "23", "Homing/DirInvert", DEFAULT_HOMING_DIR_MASK);

    // TODO Settings - need to call limits_init();
    homing_enable = new FlagSetting(GRBL, WG, "22", "Homing/Enable", DEFAULT_HOMING_ENABLE);
    // TODO Settings - need to check for HOMING_ENABLE
    hard_limits = new FlagSetting(GRBL, WG, "21", "Limits/Hard", DEFAULT_HARD_LIMIT_ENABLE);
    soft_limits = new FlagSetting(GRBL, WG, "20", "Limits/Soft", DEFAULT_SOFT_LIMIT_ENABLE, NULL);

    report_inches = new FlagSetting(GRBL, WG, "13", "Report/Inches", DEFAULT_REPORT_INCHES);
    // TODO Settings - also need to clear, but not set, soft_limits
    arc_tolerance = new FloatSetting(GRBL, WG, "12", "GCode/ArcTolerance", DEFAULT_ARC_TOLERANCE, 0, 1);
    junction_deviation = new FloatSetting(GRBL, WG, "11", "GCode/JunctionDeviation", DEFAULT_JUNCTION_DEVIATION, 0, 10);
    status_mask = new IntSetting(GRBL, WG, "10", "Report/Status", DEFAULT_STATUS_REPORT_MASK, 0, 2);

    probe_invert = new FlagSetting(GRBL, WG, "6", "Probe/Invert", DEFAULT_INVERT_PROBE_PIN);
    limit_invert = new FlagSetting(GRBL, WG, "5", "Limits/Invert", DEFAULT_INVERT_LIMIT_PINS);
    step_enable_invert = new FlagSetting(GRBL, WG, "4", "Stepper/EnableInvert", DEFAULT_INVERT_ST_ENABLE);
    dir_invert_mask = new AxisMaskSetting(GRBL, WG, "3", "Stepper/DirInvert", DEFAULT_DIRECTION_INVERT_MASK);
    step_invert_mask = new AxisMaskSetting(GRBL, WG, "2", "Stepper/StepInvert", DEFAULT_STEPPING_INVERT_MASK);
    stepper_idle_lock_time = new IntSetting(GRBL, WG, "1", "Stepper/IdleTime", DEFAULT_STEPPER_IDLE_LOCK_TIME, 0, 255);
    pulse_microseconds = new IntSetting(GRBL, WG, "0", "Stepper/Pulse", DEFAULT_STEP_PULSE_MICROSECONDS, 3, 1000);
    spindle_type = new EnumSetting(NULL, EXTENDED, WG, NULL, "Spindle/Type", SPINDLE_TYPE, &spindleTypes);
    stallguard_debug_mask = new AxisMaskSetting(EXTENDED, WG, NULL, "Report/StallGuard", 0, checkStallguardDebugMask);
	//Register THC Settings
    thc_debug_setting = new FlagSetting(EXTENDED, WG, "300", "THCDebug(0/1)", DEFAULT_THC_DEBUG);
    thc_debugprint_millis = new IntSetting(EXTENDED, WG, "301", "THCDebugPrintTime(ms)", DEFAULT_THC_DEBUG_PRINT_MILLIS, 500, 10000);
    thc_voltage_setting = new FloatSetting(EXTENDED, WG, "302", "THCVoltageTarget(volts)", DEFAULT_THC_TARGET_VOLTAGE, 0, 200);
    thc_arc_delay_time = new IntSetting(EXTENDED, WG, "303", "THCArcOnDelayTime(ms)", DEFAULT_THC_ARC_DELAY_TIME, 0, 1000);
    thc_voltage_filter_value = new FloatSetting(EXTENDED, WG, "304", "THCVoltageFilterTc", DEFAULT_THC_VOLTAGE_FILTER_VALUE, 0.0, 0.999);
    thc_iter_freq = new IntSetting(EXTENDED, WG, "305", "THCIterFreq(ms)", DEFAULT_THC_ITER_FREQ, 1, 1000);
}

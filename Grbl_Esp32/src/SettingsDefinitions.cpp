#include "Grbl.h"

FlagSetting* verbose_errors;

StringSetting* machine_name;

IntSetting* number_axis;

StringSetting* startup_line_0;
StringSetting* startup_line_1;
StringSetting* build_info;

IntSetting* pulse_microseconds;
IntSetting* stepper_idle_lock_time;
IntSetting* step_pulse_delay;

AxisMaskSetting* dir_invert_mask;
AxisMaskSetting* homing_dir_mask;
AxisMaskSetting* homing_squared_axes;

EnumSetting*     trinamic_run_mode;
EnumSetting*     trinamic_homing_mode;
AxisMaskSetting* stallguard_debug_mask;

FlagSetting* report_inches;
FlagSetting* soft_limits;
// TODO Settings - need to check for HOMING_ENABLE
FlagSetting* hard_limits;
// TODO Settings - need to call limits_init;
FlagSetting* homing_enable;
// TODO Settings - also need to clear, but not set, soft_limits
FlagSetting* laser_mode;
// TODO Settings - also need to call my_spindle->init;
IntSetting* laser_full_power;

IntSetting*   status_mask;
FloatSetting* junction_deviation;
FloatSetting* arc_tolerance;

FloatSetting*    homing_feed_rate;
FloatSetting*    homing_seek_rate;
FloatSetting*    homing_debounce;
FloatSetting*    homing_pulloff;
AxisMaskSetting* homing_cycle[MAX_N_AXIS];
FloatSetting*    spindle_pwm_freq;
FloatSetting*    rpm_max;
FloatSetting*    rpm_min;
FloatSetting*    spindle_delay_spinup;
FloatSetting*    spindle_delay_spindown;
FlagSetting*     spindle_enbl_off_with_zero_speed;

FloatSetting* spindle_pwm_off_value;
FloatSetting* spindle_pwm_min_value;
FloatSetting* spindle_pwm_max_value;
IntSetting*   spindle_pwm_bit_precision;

EnumSetting* spindle_type;

EnumSetting*  motor_types[MAX_N_AXIS][2];
FloatSetting* motor_rsense[MAX_N_AXIS][2];
IntSetting*   motor_address[MAX_N_AXIS][2];

enum_opt_t spindleTypes = {
    // clang-format off
    { "NONE", int8_t(SpindleType::NONE) },
    { "PWM", int8_t(SpindleType::PWM) },
    { "RELAY", int8_t(SpindleType::RELAY) },
    { "LASER", int8_t(SpindleType::LASER) },
    { "DAC", int8_t(SpindleType::DAC) },
    { "HUANYANG", int8_t(SpindleType::HUANYANG) },
    { "BESC", int8_t(SpindleType::BESC) },
    { "10V", int8_t(SpindleType::_10V) },
    { "H2A", int8_t(SpindleType::H2A) },
    // clang-format on
};

enum_opt_t motorTypes = {
    // clang-format off
    { "None", int8_t(MotorType::None) },
    { "StepStick", int8_t(MotorType::StepStick) },
    { "External", int8_t(MotorType::External) },
    { "TMC2130", int8_t(MotorType::TMC2130) },
    { "TMC5160", int8_t(MotorType::TMC5160) },
    { "TMC2208", int8_t(MotorType::TMC2208) },
    { "TMC2209", int8_t(MotorType::TMC2209) },
    { "Unipolar", int8_t(MotorType::Unipolar) },
    { "RCServo", int8_t(MotorType::RCServo) },
    { "Dynamixel", int8_t(MotorType::Dynamixel) },
    { "Solenoid", int8_t(MotorType::Solenoid) },
    // clang-format on
};

enum_opt_t trinamicModes = {
    // clang-format off
    { "StealthChop", int8_t(TrinamicMode::StealthChop) },
    { "CoolStep", int8_t(TrinamicMode::CoolStep) },
    { "StallGuard", int8_t(TrinamicMode::StallGuard) },
    // clang-format on
};

AxisSettings* axis_settings[MAX_N_AXIS];

StringSetting* user_macro0;
StringSetting* user_macro1;
StringSetting* user_macro2;
StringSetting* user_macro3;

typedef struct {
    const char* name;
    float       steps_per_mm;
    float       max_rate;
    float       acceleration;
    float       max_travel;
    float       home_mpos;
    float       run_current;
    float       hold_current;
    uint16_t    microsteps;
    uint16_t    stallguard;
} axis_defaults_t;
axis_defaults_t axis_defaults[] = { { "X",
                                      DEFAULT_X_STEPS_PER_MM,
                                      DEFAULT_X_MAX_RATE,
                                      DEFAULT_X_ACCELERATION,
                                      DEFAULT_X_MAX_TRAVEL,
                                      DEFAULT_X_HOMING_MPOS,
                                      DEFAULT_X_CURRENT,
                                      DEFAULT_X_HOLD_CURRENT,
                                      DEFAULT_X_MICROSTEPS,
                                      DEFAULT_X_STALLGUARD },
                                    { "Y",
                                      DEFAULT_Y_STEPS_PER_MM,
                                      DEFAULT_Y_MAX_RATE,
                                      DEFAULT_Y_ACCELERATION,
                                      DEFAULT_Y_MAX_TRAVEL,
                                      DEFAULT_Y_HOMING_MPOS,
                                      DEFAULT_Y_CURRENT,
                                      DEFAULT_Y_HOLD_CURRENT,
                                      DEFAULT_Y_MICROSTEPS,
                                      DEFAULT_Y_STALLGUARD },
                                    { "Z",
                                      DEFAULT_Z_STEPS_PER_MM,
                                      DEFAULT_Z_MAX_RATE,
                                      DEFAULT_Z_ACCELERATION,
                                      DEFAULT_Z_MAX_TRAVEL,
                                      DEFAULT_Z_HOMING_MPOS,
                                      DEFAULT_Z_CURRENT,
                                      DEFAULT_Z_HOLD_CURRENT,
                                      DEFAULT_Z_MICROSTEPS,
                                      DEFAULT_Z_STALLGUARD },
                                    { "A",
                                      DEFAULT_A_STEPS_PER_MM,
                                      DEFAULT_A_MAX_RATE,
                                      DEFAULT_A_ACCELERATION,
                                      DEFAULT_A_MAX_TRAVEL,
                                      DEFAULT_A_HOMING_MPOS,
                                      DEFAULT_A_CURRENT,
                                      DEFAULT_A_HOLD_CURRENT,
                                      DEFAULT_A_MICROSTEPS,
                                      DEFAULT_A_STALLGUARD },
                                    { "B",
                                      DEFAULT_B_STEPS_PER_MM,
                                      DEFAULT_B_MAX_RATE,
                                      DEFAULT_B_ACCELERATION,
                                      DEFAULT_B_MAX_TRAVEL,
                                      DEFAULT_B_HOMING_MPOS,
                                      DEFAULT_B_CURRENT,
                                      DEFAULT_B_HOLD_CURRENT,
                                      DEFAULT_B_MICROSTEPS,
                                      DEFAULT_B_STALLGUARD },
                                    { "C",
                                      DEFAULT_C_STEPS_PER_MM,
                                      DEFAULT_C_MAX_RATE,
                                      DEFAULT_C_ACCELERATION,
                                      DEFAULT_C_MAX_TRAVEL,
                                      DEFAULT_C_HOMING_MPOS,
                                      DEFAULT_C_CURRENT,
                                      DEFAULT_C_HOLD_CURRENT,
                                      DEFAULT_C_MICROSTEPS,
                                      DEFAULT_C_STALLGUARD } };

// Construct e.g. X_MAX_RATE from axisName "X" and tail "_MAX_RATE"
// in dynamically allocated memory that will not be freed.

static const char* makename(const char* axisName, const char* tail) {
    char* retval = (char*)malloc(strlen(axisName) + strlen(tail) + 2);

    strcpy(retval, axisName);
    strcat(retval, "/");
    return strcat(retval, tail);
}

static bool checkStartupLine(char* value) {
    if (!value) {  // No POST functionality
        return true;
    }
    if (sys.state != State::Idle) {
        return false;
    }
    return gc_execute_line(value, CLIENT_SERIAL) == Error::Ok;
}

static bool postMotorSetting(char* value) {
    if (!value) {
        motors_read_settings();
    }
    return true;
}

static bool checkSpindleChange(char* val) {
    if (!val) {
        // if not in disable (M5) ...
        if (gc_state.modal.spindle != SpindleState::Disable) {
            gc_state.modal.spindle = SpindleState::Disable;
            if (spindle->use_delays && spindle_delay_spindown->get() != 0) {  // old spindle
                vTaskDelay(spindle_delay_spindown->get() * 1000);
            }
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Spindle turned off with setting change");
        }
        gc_state.spindle_speed = 0;   // Set S value to 0
        spindle->deinit();            // old spindle
        Spindles::Spindle::select();  // get new spindle
        return true;
    }
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

void make_coordinate(CoordIndex index, const char* name) {
    float coord_data[MAX_N_AXIS] = { 0.0 };
    auto  coord                  = new Coordinates(name);
    coords[index]                = coord;
    if (!coord->load()) {
        coords[index]->setDefault();
    }
}

extern void make_pin_settings();

void make_settings() {
    Setting::init();

    machine_name = new StringSetting(EXTENDED, WG, NULL, "Machine/Name", MACHINE_NAME);

    number_axis = new IntSetting(EXTENDED, WG, NULL, "Axes", N_AXIS, 3, 6, NULL, false);
    number_axis->load();

    // Propagate old coordinate system data to the new format if necessary.
    // G54 - G59 work coordinate systems, G28, G30 reference positions, etc
    make_coordinate(CoordIndex::G54, "G54");
    make_coordinate(CoordIndex::G55, "G55");
    make_coordinate(CoordIndex::G56, "G56");
    make_coordinate(CoordIndex::G57, "G57");
    make_coordinate(CoordIndex::G58, "G58");
    make_coordinate(CoordIndex::G59, "G59");
    make_coordinate(CoordIndex::G28, "G28");
    make_coordinate(CoordIndex::G30, "G30");

    verbose_errors = new FlagSetting(EXTENDED, WG, NULL, "Errors/Verbose", DEFAULT_VERBOSE_ERRORS);

    // Create the axis settings in the order that people are
    // accustomed to seeing.
    int              axis;
    axis_defaults_t* def;
    for (axis = 0; axis < number_axis->get(); axis++) {
        def                 = &axis_defaults[axis];
        axis_settings[axis] = new AxisSettings(def->name);
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new IntSetting(
            EXTENDED, WG, makeGrblName(axis, 170), makename(def->name, "StallGuard"), def->stallguard, -64, 63, postMotorSetting);

        setting->setAxis(axis);
        axis_settings[axis]->stallguard = setting;
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new IntSetting(
            EXTENDED, WG, makeGrblName(axis, 160), makename(def->name, "Microsteps"), def->microsteps, 0, 256, postMotorSetting);
        setting->setAxis(axis);
        axis_settings[axis]->microsteps = setting;
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new FloatSetting(
            EXTENDED, WG, makeGrblName(axis, 150), makename(def->name, "Current/Hold"), def->hold_current, 0.05, 20.0, postMotorSetting);  // Amps
        setting->setAxis(axis);
        axis_settings[axis]->hold_current = setting;
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new FloatSetting(
            EXTENDED, WG, makeGrblName(axis, 140), makename(def->name, "Current/Run"), def->run_current, 0.0, 20.0, postMotorSetting);  // Amps
        setting->setAxis(axis);
        axis_settings[axis]->run_current = setting;
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, WG, makeGrblName(axis, 130), makename(def->name, "MaxTravel"), def->max_travel, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->max_travel = setting;
    }

    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new FloatSetting(EXTENDED, WG, NULL, makename(def->name, "Home/Mpos"), def->home_mpos, -100000.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->home_mpos = setting;
    }

    for (axis = 0; axis < number_axis->get(); axis++) {
        def = &axis_defaults[axis];
        auto setting =
            new FloatSetting(GRBL, WG, makeGrblName(axis, 120), makename(def->name, "Acceleration"), def->acceleration, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->acceleration = setting;
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def          = &axis_defaults[axis];
        auto setting = new FloatSetting(GRBL, WG, makeGrblName(axis, 110), makename(def->name, "MaxRate"), def->max_rate, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->max_rate = setting;
    }
    for (axis = 0; axis < number_axis->get(); axis++) {
        def = &axis_defaults[axis];
        auto setting =
            new FloatSetting(GRBL, WG, makeGrblName(axis, 100), makename(def->name, "StepsPerMm"), def->steps_per_mm, 1.0, 100000.0);
        setting->setAxis(axis);
        axis_settings[axis]->steps_per_mm = setting;
    }

    motor_types[X_AXIS][0] = new EnumSetting(NULL, EXTENDED, WG, NULL, "X/Motor/Type", static_cast<int8_t>(X_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[X_AXIS][1] =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "X2/Motor/Type", static_cast<int8_t>(X2_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[Y_AXIS][0] = new EnumSetting(NULL, EXTENDED, WG, NULL, "Y/Motor/Type", static_cast<int8_t>(Y_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[Y_AXIS][1] =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "Y2/Motor/Type", static_cast<int8_t>(Y2_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[Z_AXIS][0] = new EnumSetting(NULL, EXTENDED, WG, NULL, "Z/Motor/Type", static_cast<int8_t>(Z_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[Z_AXIS][1] =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "Z2/Motor/Type", static_cast<int8_t>(Z2_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[A_AXIS][0] = new EnumSetting(NULL, EXTENDED, WG, NULL, "A/Motor/Type", static_cast<int8_t>(A_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[A_AXIS][1] =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "A2/Motor/Type", static_cast<int8_t>(A2_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[B_AXIS][0] = new EnumSetting(NULL, EXTENDED, WG, NULL, "B/Motor/Type", static_cast<int8_t>(B_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[B_AXIS][1] =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "B2/Motor/Type", static_cast<int8_t>(B2_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[C_AXIS][0] = new EnumSetting(NULL, EXTENDED, WG, NULL, "C/Motor/Type", static_cast<int8_t>(C_MOTOR_TYPE), &motorTypes, NULL);
    motor_types[C_AXIS][1] =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "C2/Motor/Type", static_cast<int8_t>(C2_MOTOR_TYPE), &motorTypes, NULL);

    motor_rsense[X_AXIS][0] = new FloatSetting(EXTENDED, WG, NULL, "X/Driver/RSense", X_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[X_AXIS][1] = new FloatSetting(EXTENDED, WG, NULL, "X2/Driver/RSense", X2_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[Y_AXIS][0] = new FloatSetting(EXTENDED, WG, NULL, "Y/Driver/RSense", Y_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[Y_AXIS][1] = new FloatSetting(EXTENDED, WG, NULL, "Y2/Driver/RSense", Y2_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[Z_AXIS][0] = new FloatSetting(EXTENDED, WG, NULL, "Z/Driver/RSense", Z_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[Z_AXIS][1] = new FloatSetting(EXTENDED, WG, NULL, "Z2/Driver/RSense", Z2_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[A_AXIS][0] = new FloatSetting(EXTENDED, WG, NULL, "A/Driver/RSense", A_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[A_AXIS][1] = new FloatSetting(EXTENDED, WG, NULL, "A2/Driver/RSense", A2_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[B_AXIS][0] = new FloatSetting(EXTENDED, WG, NULL, "B/Driver/RSense", B_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[B_AXIS][1] = new FloatSetting(EXTENDED, WG, NULL, "B2/Driver/RSense", B2_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[C_AXIS][0] = new FloatSetting(EXTENDED, WG, NULL, "C/Driver/RSense", C_DRIVER_RSENSE, 0.0, 100.0, NULL);
    motor_rsense[C_AXIS][1] = new FloatSetting(EXTENDED, WG, NULL, "C2/Driver/RSense", C2_DRIVER_RSENSE, 0.0, 100.0, NULL);

    motor_address[X_AXIS][0] = new IntSetting(EXTENDED, WG, NULL, "X/Driver/RSense", X_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[X_AXIS][1] = new IntSetting(EXTENDED, WG, NULL, "X2/Driver/RSense", X2_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[Y_AXIS][0] = new IntSetting(EXTENDED, WG, NULL, "Y/Driver/RSense", Y_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[Y_AXIS][1] = new IntSetting(EXTENDED, WG, NULL, "Y2/Driver/RSense", Y2_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[Z_AXIS][0] = new IntSetting(EXTENDED, WG, NULL, "Z/Driver/RSense", Z_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[Z_AXIS][1] = new IntSetting(EXTENDED, WG, NULL, "Z2/Driver/RSense", Z2_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[A_AXIS][0] = new IntSetting(EXTENDED, WG, NULL, "A/Driver/RSense", A_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[A_AXIS][1] = new IntSetting(EXTENDED, WG, NULL, "A2/Driver/RSense", A2_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[B_AXIS][0] = new IntSetting(EXTENDED, WG, NULL, "B/Driver/RSense", B_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[B_AXIS][1] = new IntSetting(EXTENDED, WG, NULL, "B2/Driver/RSense", B2_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[C_AXIS][0] = new IntSetting(EXTENDED, WG, NULL, "C/Driver/RSense", C_DRIVER_ADDRESS, 1, 255, NULL);
    motor_address[C_AXIS][1] = new IntSetting(EXTENDED, WG, NULL, "C2/Driver/RSense", C2_DRIVER_ADDRESS, 1, 255, NULL);

    // Spindle Settings
    spindle_type =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "Spindle/Type", static_cast<int8_t>(SPINDLE_TYPE), &spindleTypes, checkSpindleChange);

    spindle_pwm_max_value =
        new FloatSetting(EXTENDED, WG, "36", "Spindle/PWM/Max", DEFAULT_SPINDLE_MAX_VALUE, 0.0, 100.0, checkSpindleChange);
    spindle_pwm_min_value =
        new FloatSetting(EXTENDED, WG, "35", "Spindle/PWM/Min", DEFAULT_SPINDLE_MIN_VALUE, 0.0, 100.0, checkSpindleChange);
    spindle_pwm_off_value = new FloatSetting(
        EXTENDED, WG, "34", "Spindle/PWM/Off", DEFAULT_SPINDLE_OFF_VALUE, 0.0, 100.0, checkSpindleChange);  // these are percentages
    // IntSetting spindle_pwm_bit_precision(EXTENDED, WG, "Spindle/PWM/Precision", DEFAULT_SPINDLE_BIT_PRECISION, 1, 16);
    spindle_pwm_freq = new FloatSetting(EXTENDED, WG, "33", "Spindle/PWM/Frequency", DEFAULT_SPINDLE_FREQ, 0, 100000, checkSpindleChange);

    spindle_delay_spinup   = new FloatSetting(EXTENDED, WG, NULL, "Spindle/Delay/SpinUp", DEFAULT_SPINDLE_DELAY_SPINUP, 0, 30);
    spindle_delay_spindown = new FloatSetting(EXTENDED, WG, NULL, "Spindle/Delay/SpinDown", DEFAULT_SPINDLE_DELAY_SPINUP, 0, 30);

    spindle_enbl_off_with_zero_speed =
        new FlagSetting(GRBL, WG, NULL, "Spindle/Enable/OffWithSpeed", DEFAULT_SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED, checkSpindleChange);

    // GRBL Non-numbered settings
    startup_line_0 = new StringSetting(GRBL, WG, "N0", "GCode/Line0", "", checkStartupLine);
    startup_line_1 = new StringSetting(GRBL, WG, "N1", "GCode/Line1", "", checkStartupLine);

    // GRBL Numbered Settings
    laser_mode       = new FlagSetting(GRBL, WG, "32", "GCode/LaserMode", DEFAULT_LASER_MODE);
    laser_full_power = new IntSetting(EXTENDED, WG, NULL, "Laser/FullPower", DEFAULT_LASER_FULL_POWER, 0, 10000, checkSpindleChange);

    // TODO Settings - also need to call my_spindle->init();
    rpm_min = new FloatSetting(GRBL, WG, "31", "GCode/MinS", DEFAULT_SPINDLE_RPM_MIN, 0, 100000, checkSpindleChange);
    rpm_max = new FloatSetting(GRBL, WG, "30", "GCode/MaxS", DEFAULT_SPINDLE_RPM_MAX, 0, 100000, checkSpindleChange);

    homing_pulloff      = new FloatSetting(GRBL, WG, "27", "Homing/Pulloff", DEFAULT_HOMING_PULLOFF, 0, 1000);
    homing_debounce     = new FloatSetting(GRBL, WG, "26", "Homing/Debounce", DEFAULT_HOMING_DEBOUNCE_DELAY, 0, 10000);
    homing_seek_rate    = new FloatSetting(GRBL, WG, "25", "Homing/Seek", DEFAULT_HOMING_SEEK_RATE, 0, 10000);
    homing_feed_rate    = new FloatSetting(GRBL, WG, "24", "Homing/Feed", DEFAULT_HOMING_FEED_RATE, 0, 10000);
    homing_squared_axes = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Squared", DEFAULT_HOMING_SQUARED_AXES);

    // TODO Settings - need to call st_generate_step_invert_masks()
    homing_dir_mask = new AxisMaskSetting(GRBL, WG, "23", "Homing/DirInvert", DEFAULT_HOMING_DIR_MASK);

    // TODO Settings - need to call limits_init();
    homing_enable = new FlagSetting(GRBL, WG, "22", "Homing/Enable", DEFAULT_HOMING_ENABLE);
    // TODO Settings - need to check for HOMING_ENABLE
    hard_limits = new FlagSetting(GRBL, WG, "21", "Limits/Hard", DEFAULT_HARD_LIMIT_ENABLE);
    soft_limits = new FlagSetting(GRBL, WG, "20", "Limits/Soft", DEFAULT_SOFT_LIMIT_ENABLE, NULL);

    build_info    = new StringSetting(EXTENDED, WG, NULL, "Firmware/Build", "");
    report_inches = new FlagSetting(GRBL, WG, "13", "Report/Inches", DEFAULT_REPORT_INCHES);
    // TODO Settings - also need to clear, but not set, soft_limits
    arc_tolerance      = new FloatSetting(GRBL, WG, "12", "GCode/ArcTolerance", DEFAULT_ARC_TOLERANCE, 0, 1);
    junction_deviation = new FloatSetting(GRBL, WG, "11", "GCode/JunctionDeviation", DEFAULT_JUNCTION_DEVIATION, 0, 10);
    status_mask        = new IntSetting(GRBL, WG, "10", "Report/Status", DEFAULT_STATUS_REPORT_MASK, 0, 3);

    dir_invert_mask        = new AxisMaskSetting(GRBL, WG, "3", "Stepper/DirInvert", DEFAULT_DIRECTION_INVERT_MASK);
    stepper_idle_lock_time = new IntSetting(GRBL, WG, "1", "Stepper/IdleTime", DEFAULT_STEPPER_IDLE_LOCK_TIME, 0, 255);
    pulse_microseconds     = new IntSetting(GRBL, WG, "0", "Stepper/Pulse/Duration", DEFAULT_STEP_PULSE_MICROSECONDS, 3, 1000);
    step_pulse_delay       = new IntSetting(GRBL, WG, "0", "Stepper/Pulse/Delay", 0, 0, 8);

    trinamic_run_mode =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "Trinamic/RunMode", static_cast<int8_t>(TRINAMIC_RUN_MODE), &trinamicModes, NULL);
    trinamic_homing_mode =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "Trinamic/HomingMode", static_cast<int8_t>(TRINAMIC_HOMING_MODE), &trinamicModes, NULL);
    stallguard_debug_mask = new AxisMaskSetting(EXTENDED, WG, NULL, "Report/StallGuard", 0, postMotorSetting);

    homing_cycle[5] = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Cycle5", DEFAULT_HOMING_CYCLE_5);
    homing_cycle[4] = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Cycle4", DEFAULT_HOMING_CYCLE_4);
    homing_cycle[3] = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Cycle3", DEFAULT_HOMING_CYCLE_3);
    homing_cycle[2] = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Cycle2", DEFAULT_HOMING_CYCLE_2);
    homing_cycle[1] = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Cycle1", DEFAULT_HOMING_CYCLE_1);
    homing_cycle[0] = new AxisMaskSetting(EXTENDED, WG, NULL, "Homing/Cycle0", DEFAULT_HOMING_CYCLE_0);

    user_macro3 = new StringSetting(EXTENDED, WG, NULL, "User/Macro3", DEFAULT_USER_MACRO3);
    user_macro2 = new StringSetting(EXTENDED, WG, NULL, "User/Macro2", DEFAULT_USER_MACRO2);
    user_macro1 = new StringSetting(EXTENDED, WG, NULL, "User/Macro1", DEFAULT_USER_MACRO1);
    user_macro0 = new StringSetting(EXTENDED, WG, NULL, "User/Macro0", DEFAULT_USER_MACRO0);

    make_pin_settings();  // Created in PinSettingsDefinitions.cpp
}

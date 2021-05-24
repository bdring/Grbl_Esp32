#include "Grbl.h"

StringSetting* startup_line_0;
StringSetting* startup_line_1;
StringSetting* build_info;

IntSetting* status_mask;

EnumSetting* spindle_type;

EnumSetting* message_level;

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

enum_opt_t messageLevels = {
    // clang-format off
    { "None", int8_t(MsgLevel::None) },
    { "Error", int8_t(MsgLevel::Error) },
    { "Warning", int8_t(MsgLevel::Warning) },
    { "Info", int8_t(MsgLevel::Info) },
    { "Debug", int8_t(MsgLevel::Debug) },
    { "Verbose", int8_t(MsgLevel::Verbose) },
    // clang-format on
};

StringSetting* user_macro0;
StringSetting* user_macro1;
StringSetting* user_macro2;
StringSetting* user_macro3;

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
#ifdef LATER
    if (!value) {
        motors_read_settings();
    }
#endif
    return true;
}

static bool checkSpindleChange(char* val) {
    if (!val) {
        gc_state.spindle_speed = 0;   // Set S value to 0
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

    // Spindle Settings
    spindle_type =
        new EnumSetting(NULL, EXTENDED, WG, NULL, "Spindle/Type", static_cast<int8_t>(SPINDLE_TYPE), &spindleTypes, checkSpindleChange);

    // GRBL Non-numbered settings
    startup_line_0 = new StringSetting(EXTENDED, WG, "N0", "GCode/Line0", "", checkStartupLine);
    startup_line_1 = new StringSetting(EXTENDED, WG, "N1", "GCode/Line1", "", checkStartupLine);

    // GRBL Numbered Settings
    build_info = new StringSetting(EXTENDED, WG, NULL, "Firmware/Build", "");

    // TODO: These affect the sender communication protocol so they
    // need to be be available as $ commands
    // verbose_errors = new FlagSetting(EXTENDED, WG, NULL, "Errors/Verbose", DEFAULT_VERBOSE_ERRORS);
    // report_inches = new FlagSetting(GRBL, WG, "13", "Report/Inches", DEFAULT_REPORT_INCHES);

    status_mask = new IntSetting(GRBL, WG, "10", "Report/Status", DEFAULT_STATUS_REPORT_MASK, 0, 3);

    user_macro3 = new StringSetting(EXTENDED, WG, NULL, "User/Macro3", DEFAULT_USER_MACRO3);
    user_macro2 = new StringSetting(EXTENDED, WG, NULL, "User/Macro2", DEFAULT_USER_MACRO2);
    user_macro1 = new StringSetting(EXTENDED, WG, NULL, "User/Macro1", DEFAULT_USER_MACRO1);
    user_macro0 = new StringSetting(EXTENDED, WG, NULL, "User/Macro0", DEFAULT_USER_MACRO0);

    message_level = +new EnumSetting(NULL, EXTENDED, WG, NULL, "Message/Level", static_cast<int8_t>(MsgLevel::Info), &messageLevels, NULL);

    make_pin_settings();  // Created in PinSettingsDefinitions.cpp
}

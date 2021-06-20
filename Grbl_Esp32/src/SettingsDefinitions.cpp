#include "Grbl.h"

StringSetting* config_filename;

StringSetting* startup_line_0;
StringSetting* startup_line_1;
StringSetting* build_info;

IntSetting* status_mask;

EnumSetting* message_level;

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

static bool checkStartupLine(char* value) {
    if (!value) {  // No POST functionality
        return true;
    }
    if (sys.state != State::Idle) {
        return false;
    }
    return gc_execute_line(value, CLIENT_SERIAL) == Error::Ok;
}

void make_coordinate(CoordIndex index, const char* name) {
    float coord_data[MAX_N_AXIS] = { 0.0 };
    auto  coord                  = new Coordinates(name);
    coords[index]                = coord;
    if (!coord->load()) {
        coords[index]->setDefault();
    }
}

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

    // GRBL Non-numbered settings
    startup_line_0 = new StringSetting(EXTENDED, WG, "N0", "GCode/Line0", "", checkStartupLine);
    startup_line_1 = new StringSetting(EXTENDED, WG, "N1", "GCode/Line1", "", checkStartupLine);

    // GRBL Numbered Settings
    build_info = new StringSetting(EXTENDED, WG, NULL, "Firmware/Build", "");

    status_mask = new IntSetting(GRBL, WG, "10", "Report/Status", DEFAULT_STATUS_REPORT_MASK, 0, 3);

    user_macro3 = new StringSetting(EXTENDED, WG, NULL, "User/Macro3", DEFAULT_USER_MACRO3);
    user_macro2 = new StringSetting(EXTENDED, WG, NULL, "User/Macro2", DEFAULT_USER_MACRO2);
    user_macro1 = new StringSetting(EXTENDED, WG, NULL, "User/Macro1", DEFAULT_USER_MACRO1);
    user_macro0 = new StringSetting(EXTENDED, WG, NULL, "User/Macro0", DEFAULT_USER_MACRO0);

    message_level = +new EnumSetting(NULL, EXTENDED, WG, NULL, "Message/Level", static_cast<int8_t>(MsgLevel::Info), &messageLevels, NULL);

    config_filename = new StringSetting(EXTENDED, WG, NULL, "Config/Filename", "config.yaml");
}

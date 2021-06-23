#pragma once

#include <map>
#include <cstdint>

// Override bit maps. Realtime bitflags to control feed, rapid, spindle, and coolant overrides.
// Spindle/coolant and feed/rapids are separated into two controlling flag variables.

struct ExecAccessoryBits {
    uint8_t spindleOvrStop : 1;
    uint8_t coolantFloodOvrToggle : 1;
    uint8_t coolantMistOvrToggle : 1;
};

union ExecAccessory {
    uint8_t           value;
    ExecAccessoryBits bit;
};

// Alarm codes.
enum class ExecAlarm : uint8_t {
    None               = 0,
    HardLimit          = 1,
    SoftLimit          = 2,
    AbortCycle         = 3,
    ProbeFailInitial   = 4,
    ProbeFailContact   = 5,
    HomingFailReset    = 6,
    HomingFailDoor     = 7,
    HomingFailPulloff  = 8,
    HomingFailApproach = 9,
    SpindleControl     = 10,
};

extern std::map<ExecAlarm, const char*> AlarmNames;

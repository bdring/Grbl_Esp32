#include "Exec.h"

std::map<ExecAlarm, const char*> AlarmNames = {
    { ExecAlarm::None, "None"},
    { ExecAlarm::HardLimit, "Hard Limit"},
    { ExecAlarm::SoftLimit, "Soft Limit"},
    { ExecAlarm::AbortCycle, "Abort Cycle"},
    { ExecAlarm::ProbeFailInitial, "Probe Fail Initial"},
    { ExecAlarm::ProbeFailContact, "Probe Fail Contact"},
    { ExecAlarm::HomingFailReset, "Homing Fail Reset"},
    { ExecAlarm::HomingFailDoor, "Homing Fail Door"},
    { ExecAlarm::HomingFailPulloff, "Homing Fail Pulloff"},
    { ExecAlarm::HomingFailApproach, "Homing Fail Approach"},
    { ExecAlarm::SpindleControl, "Spindle Control"},
};

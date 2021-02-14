#pragma once

#include "Assert.h"
#include "Configuration/GenericFactory.h"
#include "Configuration/HandlerBase.h"
#include "Configuration/Configurable.h"

namespace Motors {
    class Motor;
}

class Axis : public Configuration::Configurable {
public:
    Axis() = default;

    Motors::Motor* motor_ = nullptr;

    // Configuration system helpers:
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    // Checks if a motor matches this axis:
    bool hasMotor(const Motors::Motor* const motor) const;

    ~Axis();
};

class Axes : public Configuration::Configurable {
    static const int MAX_NUMBER_AXIS   = 6;
    static const int MAX_NUMBER_GANGED = 2;

    Axis* axis_[MAX_NUMBER_AXIS][MAX_NUMBER_GANGED + 1];

public:
    Axes();

    // Some small helpers to find the axis index and axis ganged index for a given motor. This
    // is helpful for some motors that need this info, as well as debug information.
    size_t findAxisIndex(const Motors::Motor* const motor) const;

    size_t findAxisGanged(const Motors::Motor* const motor) const;

    // These are used for setup and to talk to the motors as a group.
    void init();
    void read_settings(); // more like 'after read settings, before init'. Oh well...

    // The return value is a bitmask of axes that can home
    uint8_t set_homing_mode(uint8_t homing_mask, bool isHoming);
    void    set_disable(bool disable);
    void    step(uint8_t step_mask, uint8_t dir_mask);
    void    unstep();

    // Configuration helpers:
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    ~Axes();
};

class MachineConfig : public Configuration::Configurable {
public:
    Axes* axes_ = nullptr;

    MachineConfig() = default;

    static MachineConfig*& instance() {
        static MachineConfig* instance = nullptr;
        return instance;
    }

    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    bool load(const char* file = "/spiffs/config.yaml");

    ~MachineConfig();
};

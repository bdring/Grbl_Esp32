#pragma once

#include "Assert.h"
#include "Configuration/GenericFactory.h"
#include "Configuration/HandlerBase.h"
#include "Configuration/Configurable.h"
#include "CoolantControl.h"
#include "Probe.h"

namespace Motors {
    class Motor;
}

class Endstops : public Configuration::Configurable {
    Pin  _positive;
    Pin  _negative;
    Pin  _dual;
    bool _hardLimits = true;

public:
    Endstops() = default;

    // Configuration system helpers:
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;
};

class Gang : public Configuration::Configurable {
public:
    Gang() = default;

    Motors::Motor* _motor    = nullptr;
    Endstops*      _endstops = nullptr;

    // Configuration system helpers:
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    ~Gang();
};

class Homing : public Configuration::Configurable {
public:
    Homing() = default;

    int  _cycle             = -1;
    bool _positiveDirection = true;
    int  _mpos              = 0;
    int  _feedRate          = 500;
    int  _seekRate          = 100;
    int  _debounce          = 10;

    // Configuration system helpers:
    void validate() const override { Assert(_cycle >= 1, "Cycle has to be defined as >= 1 for homing sequence."); }

    void handle(Configuration::HandlerBase& handler) override {
        handler.handle("cycle", _cycle);
        handler.handle("positive_direction", _positiveDirection);
        handler.handle("mpos", _mpos);
        handler.handle("feed_rate", _feedRate);
        handler.handle("seek_rate", _seekRate);
        handler.handle("debounce", _debounce);
    }
};

class Axis : public Configuration::Configurable {
public:
    Axis() {
        for (int i = 0; i < MAX_NUMBER_GANGED; ++i) {
            _gangs[i] = nullptr;
        }
    }
    static const int MAX_NUMBER_GANGED = 2;

    Gang*   _gangs[MAX_NUMBER_GANGED];
    Homing* _homing = nullptr;

    int  _stepsPerMm   = 320;
    int  _maxRate      = 1000;
    int  _acceleration = 25;
    int  _maxTravel    = 200;
    bool _softLimits   = false;
    bool _autoSquaring = false;

    // Configuration system helpers:
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    // Checks if a motor matches this axis:
    bool hasMotor(const Motors::Motor* const motor) const;

    ~Axis();
};

class Axes : public Configuration::Configurable {
    static const int MAX_NUMBER_AXIS = 6;

    Axis* axis_[MAX_NUMBER_AXIS];

public:
    Axes();

    int _numberAxis = 3;

    // Some small helpers to find the axis index and axis ganged index for a given motor. This
    // is helpful for some motors that need this info, as well as debug information.
    size_t findAxisIndex(const Motors::Motor* const motor) const;
    size_t findAxisGanged(const Motors::Motor* const motor) const;

    // These are used for setup and to talk to the motors as a group.
    void init();
    void read_settings();  // more like 'after read settings, before init'. Oh well...

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

class I2SO : public Configuration::Configurable {
public:
    I2SO() = default;

    Pin _bck;
    Pin _data;
    Pin _ws;

    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    ~I2SO() {}
};

class MachineConfig : public Configuration::Configurable {
public:
    MachineConfig()          = default;
    Axes*           _axes    = nullptr;
    I2SO*           _i2so    = nullptr;
    CoolantControl* _coolant = nullptr;
    Probe*          _probe   = nullptr;

    static MachineConfig*& instance() {
        static MachineConfig* instance = nullptr;
        return instance;
    }

    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;

    bool load(const char* file = "/spiffs/config.yaml");

    ~MachineConfig();
};

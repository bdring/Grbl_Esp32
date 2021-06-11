/*
    Part of Grbl_ESP32
    2021 -  Stefan de Bruijn

    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Grbl_ESP32 is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Assert.h"
#include "Configuration/GenericFactory.h"
#include "Configuration/HandlerBase.h"
#include "Configuration/Configurable.h"
#include "CoolantControl.h"
#include "WebUI/BTConfig.h"
#include "Control.h"
#include "Probe.h"
#include "SDCard.h"
#include "Spindles/Spindle.h"
#include "EnumItem.h"
#include "Stepper.h"
#include "Logging.h"

// TODO FIXME: Split this file up into several files, perhaps put it in some folder and namespace Machine?

namespace Motors {
    class Motor;
}

class Endstops : public Configuration::Configurable {
public:
    Endstops() = default;

    //     Pin  _positive;
    //     Pin  _negative;
    Pin  _dual;
    bool _hardLimits = false;

    // Configuration system helpers:
    void validate() const override;
    void group(Configuration::HandlerBase& handler) override;
};

class Gang : public Configuration::Configurable {
public:
    Gang() = default;

    Motors::Motor* _motor    = nullptr;
    Endstops*      _endstops = nullptr;

    // Configuration system helpers:
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;

    ~Gang();
};

class Homing : public Configuration::Configurable {
public:
    Homing() = default;

    int   _cycle             = -1;
    bool  _square            = false;
    bool  _positiveDirection = true;
    float _mpos              = 0;
    float _feedRate          = 500;
    float _seekRate          = 100;
    float _pulloff           = 1.0;  // mm
    int   _debounce          = 10;

    // Configuration system helpers:
    void validate() const override { Assert(_cycle >= 1, "Cycle has to be defined as >= 1 for homing sequence."); }

    void group(Configuration::HandlerBase& handler) override {
        handler.item("cycle", _cycle);
        handler.item("positive_direction", _positiveDirection);
        handler.item("mpos", _mpos);
        handler.item("feed_rate", _feedRate);
        handler.item("seek_rate", _seekRate);
        handler.item("debounce", _debounce);
        handler.item("pulloff", _pulloff);
        handler.item("square", _square);
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

    // Configuration system helpers:
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;

    // Checks if a motor matches this axis:
    bool hasMotor(const Motors::Motor* const motor) const;

    ~Axis();
};

class Axes : public Configuration::Configurable {
    static const int MAX_NUMBER_AXIS = 6;

public:
    Axes();

    Pin _sharedStepperDisable;

    int   _numberAxis = 3;
    Axis* _axis[MAX_NUMBER_AXIS];

    // Some small helpers to find the axis index and axis ganged index for a given motor. This
    // is helpful for some motors that need this info, as well as debug information.
    size_t findAxisIndex(const Motors::Motor* const motor) const;
    size_t findAxisGanged(const Motors::Motor* const motor) const;

    inline bool hasSoftLimits() const {
        for (int i = 0; i < _numberAxis; ++i) {
            if (_axis[i]->_softLimits) {
                return true;
            }
        }
        return false;
    }

    inline bool hasHardLimits() const {
        for (int i = 0; i < _numberAxis; ++i) {
            for (int j = 0; j < Axis::MAX_NUMBER_GANGED; ++j) {
                if (_axis[i]->_gangs[j]->_endstops != nullptr && _axis[i]->_gangs[j]->_endstops->_hardLimits) {
                    return true;
                }
            }
        }
        return false;
    }

    // These are used for setup and to talk to the motors as a group.
    void init();
    void read_settings();  // more like 'after read settings, before init'. Oh well...

    // The return value is a bitmask of axes that can home
    uint8_t set_homing_mode(uint8_t homing_mask, bool isHoming);
    void    set_disable(bool disable);
    void    step(uint8_t step_mask, uint8_t dir_mask);
    void    unstep();

    // Configuration helpers:
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;

    ~Axes();
};

class I2SOBus : public Configuration::Configurable {
public:
    I2SOBus() = default;

    Pin _bck;
    Pin _data;
    Pin _ws;

    void validate() const override;
    void group(Configuration::HandlerBase& handler) override;

    ~I2SOBus() = default;
};

class SPIBus : public Configuration::Configurable {
public:
    SPIBus() = default;

    Pin _ss;
    Pin _miso;
    Pin _mosi;
    Pin _sck;

    void validate() const override;
    void group(Configuration::HandlerBase& handler) override;
    void afterParse() override;

    ~SPIBus() = default;
};

class UserOutputs : public Configuration::Configurable {
public:
    UserOutputs() = default;

    Pin _analogOutput[4];
    Pin _digitalOutput[4];

    void group(Configuration::HandlerBase& handler) override;

    ~UserOutputs() = default;
};

class WifiConfig : public Configuration::Configurable {
public:
    IPAddress _ipAddress;
    IPAddress _gateway;
    IPAddress _netmask;

    WifiConfig() : _ipAddress(10, 0, 0, 1), _gateway(10, 0, 0, 1), _netmask(255, 255, 0, 0) {}

    String _ssid = "GRBL_ESP";

    // Passwords don't belong in a YAML!
    // String _password = "12345678";

    bool _dhcp = true;

    void group(Configuration::HandlerBase& handler) override {
        handler.item("ssid", _ssid);
        // handler.item("password", _password);

        handler.item("ip_address", _ipAddress);
        handler.item("gateway", _gateway);
        handler.item("netmask", _netmask);

        handler.item("dhcp", _dhcp);
    }
};

class WifiAPConfig : public WifiConfig {
public:
    WifiAPConfig() = default;

    int _channel = 1;

    void validate() const override {
        WifiConfig::validate();
        Assert(_channel >= 1 && _channel <= 16, "WIFI channel %d is out of bounds", _channel);  // TODO: I guess?
    }

    void group(Configuration::HandlerBase& handler) override {
        WifiConfig::group(handler);
        handler.item("channel", _channel);
    }

    ~WifiAPConfig() = default;
};

class WifiSTAConfig : public WifiConfig {
public:
    WifiSTAConfig() = default;

    void validate() const override { WifiConfig::validate(); }

    void group(Configuration::HandlerBase& handler) override { WifiConfig::group(handler); }

    ~WifiSTAConfig() = default;
};

class Communications : public Configuration::Configurable {
public:
    Communications() = default;

    // Passwords don't belong in a YAML!
    //
    // String _userPassword  = "";
    // String _adminPassword = "";

    bool _telnetEnable = true;
    int  _telnetPort   = 23;

    bool _httpEnable = true;
    int  _httpPort   = 80;

    String _hostname = "grblesp";

    WebUI::BTConfig* _bluetoothConfig = nullptr;
    WifiAPConfig*    _apConfig        = nullptr;
    WifiSTAConfig*   _staConfig       = nullptr;

    void group(Configuration::HandlerBase& handler) override {
        // handler.item("user_password", _userPassword);
        // handler.item("admin_password", _adminPassword);

        handler.item("telnet_enable", _telnetEnable);
        handler.item("telnet_port", _telnetPort);

        handler.item("http_enable", _httpEnable);
        handler.item("http_port", _httpPort);

        handler.item("hostname", _hostname);

        handler.section("bluetooth", _bluetoothConfig);
        handler.section("wifi_ap", _apConfig);
        handler.section("wifi_sta", _staConfig);
    }

    ~Communications() {
        delete _bluetoothConfig;
        delete _apConfig;
        delete _staConfig;
    }
};

class MachineConfig : public Configuration::Configurable {
public:
    MachineConfig() = default;

    Axes*              _axes        = nullptr;
    SPIBus*            _spi         = nullptr;
    I2SOBus*           _i2so        = nullptr;
    CoolantControl*    _coolant     = nullptr;
    Probe*             _probe       = nullptr;
    Communications*    _comms       = nullptr;
    Control*           _control     = nullptr;
    UserOutputs*       _userOutputs = nullptr;
    SDCard*            _sdCard      = nullptr;
    Spindles::Spindle* _spindle     = nullptr;

    int _pulseMicroSeconds          = 3;
    int _directionDelayMicroSeconds = 0;
    int _disableDelayMicroSeconds   = 0;

    bool    _laserMode         = false;
    float   _arcTolerance      = 0.002;
    float   _junctionDeviation = 0.01;
    uint8_t _idleTime          = 255;
    bool    _verboseErrors     = false;
    bool    _reportInches      = false;
    bool    _homingInitLock    = false;
    int     _stepType          = ST_RMT;

    String _board = "None";
    String _name  = "None";

#if 1
    static MachineConfig*& instance() {
        static MachineConfig* instance = nullptr;
        return instance;
    }
#endif

    void afterParse() override;
    void group(Configuration::HandlerBase& handler) override;

    static size_t readFile(const char* file, char*& buffer);
    static bool   load(const char* file);

    ~MachineConfig();
};

extern MachineConfig* config;

inline bool hasWiFi() {
    return config && config->_comms && (config->_comms->_staConfig != nullptr || config->_comms->_apConfig != nullptr);
}
inline bool hasBluetooth() {
    return !hasWiFi() && (config && config->_comms && config->_comms->_bluetoothConfig != nullptr);
}

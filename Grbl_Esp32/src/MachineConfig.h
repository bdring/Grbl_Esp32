#pragma once

#include "Assert.h"
#include "Configuration/GenericFactory.h"
#include "Configuration/HandlerBase.h"
#include "Configuration/Configurable.h"
#include "CoolantControl.h"
#include "Probe.h"

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
    bool _hardLimits = true;

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
    void afterParse() override;

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
    void validate() const override;
    void handle(Configuration::HandlerBase& handler) override;
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
    void handle(Configuration::HandlerBase& handler) override;

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
    void handle(Configuration::HandlerBase& handler) override;

    ~SPIBus() = default;
};

class BluetoothConfig : public Configuration::Configurable {
public:
    BluetoothConfig() = default;

    String _name = "grbl_esp32";

    void validate() const override { Assert(_name.length() > 0, "Bluetooth must have a name if it's configured"); }

    void handle(Configuration::HandlerBase& handler) override { handler.handle("_name", _name); }
};

class WifiConfig : public Configuration::Configurable {
public:
    WifiConfig() = default;

    String _ssid     = "GRBL_ESP";
    String _password = "12345678";

    uint32_t _ipAddress = 0x0a000001;  //  10.  0.  0.  1
    uint32_t _gateway   = 0x0a000001;  //  10.  0.  0.  1
    uint32_t _netmask   = 0xffffff00;  // 255.255.255.  0

    bool _dhcp = true;

    void validate() const override {}

    bool tryParseIP(StringRange ip, uint32_t& dst) {
        if (ip.begin() != nullptr) {
            uint32_t value = 0;
            uint32_t tmp   = 0;
            int      c     = 0;
            for (auto ch : ip) {
                if (ch >= '0' && ch <= '9') {
                    tmp = tmp * 10 + ch - '0';
                } else if (ch == '.') {
                    ++c;
                    if (c >= 4) {
                        return false;
                    }
                    if (tmp > 255) {
                        return false;
                    }
                    value = (value << 8) + tmp;
                    tmp   = 0;
                } else if (ch != ' ') {
                    // For convenience / layouting.
                    return false;
                }
            }
            if (tmp > 255) {
                return false;
            }
            value = (value << 8) + tmp;

            // Correct.
            dst = value;

            return true;
        }
        return false;
    }

    void handle(Configuration::HandlerBase& handler) override {
        handler.handle("ssid", _ssid);
        handler.handle("password", _password);

        StringRange ip;
        handler.handle("ip_address", ip);
        tryParseIP(ip, _ipAddress);

        StringRange gateway;
        handler.handle("gateway", gateway);
        tryParseIP(gateway, _gateway);

        StringRange netmask;
        handler.handle("netmask", netmask);
        tryParseIP(netmask, _netmask);

        handler.handle("dhcp", _dhcp);
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

    void handle(Configuration::HandlerBase& handler) override {
        WifiConfig::handle(handler);
        handler.handle("channel", _channel);
    }

    ~WifiAPConfig() = default;
};

class WifiSTAConfig : public WifiConfig {
public:
    WifiSTAConfig() = default;

    void validate() const override { WifiConfig::validate(); }

    void handle(Configuration::HandlerBase& handler) override { WifiConfig::handle(handler); }

    ~WifiSTAConfig() = default;
};

class Communications : public Configuration::Configurable {
public:
    Communications() = default;

    String _userPassword  = "";
    String _adminPassword = "";

    bool _telnetEnable = true;
    int  _telnetPort   = 23;

    bool _httpEnable = true;
    int  _httpPort   = 80;

    String _hostname = "grblesp";

    BluetoothConfig* _bluetoothConfig = nullptr;
    WifiAPConfig*    _apConfig        = nullptr;
    WifiSTAConfig*   _staConfig       = nullptr;

    void validate() const override {}
    void handle(Configuration::HandlerBase& handler) override {
        handler.handle("user_password", _userPassword);
        handler.handle("admin_password", _adminPassword);

        handler.handle("telnet_enable", _telnetEnable);
        handler.handle("telnet_port", _telnetPort);

        handler.handle("http_enable", _httpEnable);
        handler.handle("http_port", _httpPort);

        handler.handle("hostname", _hostname);

        handler.handle("bluetooth", _bluetoothConfig);
        handler.handle("wifi_ap", _apConfig);
        handler.handle("wifi_sta", _staConfig);
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

    Axes*           _axes    = nullptr;
    SPIBus*         _spi     = nullptr;
    I2SOBus*        _i2so    = nullptr;
    CoolantControl* _coolant = nullptr;
    Probe*          _probe   = nullptr;
    Communications* _comms   = nullptr;

    int _pulseMicroSeconds          = 3;
    int _directionDelayMilliSeconds = 0;
    int _disableDelayMilliSeconds   = 0;

    bool  _laserMode         = false;
    float _arcTolerance      = 0.002;
    float _junctionDeviation = 0.01;

    String _board = "None";
    String _name  = "None";

    static MachineConfig*& instance() {
        static MachineConfig* instance = nullptr;
        return instance;
    }

    void validate() const override;
    void afterParse() override;
    void handle(Configuration::HandlerBase& handler) override;

    bool load(const char* file = "/spiffs/config.yaml");

    ~MachineConfig();
};

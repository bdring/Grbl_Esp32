#pragma once

/*
	PWMSpindle.h

	This is a full featured TTL PWM spindle This does not include speed/power
	compensation. Use the Laser class for that.

	Part of Grbl_ESP32
	2020 -	Bart Dring

	Grbl is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	Grbl is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "Spindle.h"

#include <cstring>
#include <vector>
#include <tuple>

namespace Settings {
    class YamlParser {
        // I didn't create a full parser, as we don't need it.
        // See: https://en.wikipedia.org/wiki/YAML
        // 
        // Supported:
        // - We start with '---' and assume a single document. If we encounter '---', error.
        // - # is a comment
        // - indentation is with spaces
        // - Tabs are not allowed for indentation
        // - key: value pairs
        // - strings with quotes
        //
        // Unsupported:
        // - '-' lists
        // - c-style escaping in strings
        // - repeated nodes are initially denoted by an ampersand (&) and thereafter referenced with an asterisk (*).
        // - '['..']', !! and % are simply not supported.

    public:
        // TODO FIXME: Create a parser. This is just a test.
        //
        // We have to think this through a bit. We want to stream the key/values
        // in some way, but at the same time make it easy to use. Perhaps a
        // simple 'movenext', 'token type', 'key' and 'value' will do.

        bool        moveNext() { return n++ < 1; }
        const char* key() { return "spindles"; }
        const char* value() { return "gpio.12"; }
        /*
        const char* getValue(const char* key) {
            if (!strcmp(key, "output")) {
                return "gpio.12";
            } else if (!strcmp(key, "direction")) {
                return "gpio.13";
            } else if (!strcmp(key, "enable")) {
                return "gpio.14";
            }
            return nullptr;
        }

        const char* getNextSection()
        {
            if (n == 0) {
                return "spindles";
            }
            else {
                return "pwm";
            }
        }
        */
    };

    // Everything that uses the parser derives from this.
    class SettingsParser {
    public:
        virtual void* parse(YamlParser& parser) = 0;
    };

    class SettingLeaf;

    class SettingsNode : public SettingsParser {
    protected:
        std::vector<SettingLeaf*> myLeafs;

    public:
        static SettingsNode*& CurrentContainer() {
            static SettingsNode* current = nullptr;
            return current;
        }

        SettingsNode() { CurrentContainer() = this; }

        void Add(SettingLeaf* leaf) { myLeafs.push_back(leaf); }
    };

    class SettingsCollection {
        struct Item {
            const char*   name;
            const char*   parent;
            SettingsNode* builder;
        };

        SettingsCollection() = default;

        static SettingsCollection& instance() {
            static SettingsCollection instance;
            return instance;
        }

        std::vector<Item> builders_;

    public:
        static void registerSetting(const char* name, const char* parent, SettingsNode* builder) {
            instance().builders_.push_back({ name, parent, builder });
        }

        static SettingsNode* find(const char* name, const char* parent) {
            if (parent != nullptr) {
                for (auto& it : instance().builders_) {
                    if (!strcmp(it.name, name) && !strcmp(it.parent, parent)) {
                        return it.builder;
                    }
                }
            } else {
                for (auto& it : instance().builders_) {
                    if (!strcmp(it.name, name) && it.parent == nullptr) {
                        return it.builder;
                    }
                }
            }
            return nullptr;
        }
    };

    // Leafs: basically key's with some value.
    class SettingLeaf {
    protected:
        const char* key_;

    public:
        SettingLeaf(const char* key) : key_(key) {
            auto currentContainer = SettingsNode::CurrentContainer();
            if (currentContainer != nullptr) {
                currentContainer->Add(this);
            }
        }

        const char*  key() const { return key_; }
        virtual void parse(YamlParser& parser) = 0;
    };

    class PinSetting : SettingLeaf {
        Pin value_;

    public:
        PinSetting(const char* key) : SettingLeaf(key) {}

        void parse(YamlParser& parser) override { value_ = Pin::create(parser.value()); }
        Pin  value() const { return value_; }
    };

    // When an error occurs, we want details. Let's just throw an error with these details, and have the
    // framework clean up the mess.
    class SettingsError {
        SettingsError() = default;

    public:
        SettingsError(const char* message, ...) {
            // vsnprintf etc.
        }
    };

    // NOTE: Settings just _define_ settings, and are very temporary objects. The lifetime of the members in a
    // Settings object is basically the lifetime at which that section is parsed. This is very important, because
    // if you use member variables for -say- motors, they can bleed through in other axis. In other words: it's
    // best not to use member variables in settings expect for leaf settings.
    template <typename Category>
    class Setting : SettingsNode {
    public:
        Setting(const char* settingName, const char* parentName) { SettingsCollection::registerSetting(settingName, parentName, this); }

        virtual Category* create() = 0;

        virtual void* parse(YamlParser& parser) {
            for (auto leaf : myLeafs) {
                leaf->parse(parser);
            }
            return create();
        }

        virtual ~Setting() {}
    };
}

class SpindleCollection {
public:
    std::vector<Spindles::Spindle*> spindles_;

    struct MySettings : Settings::Setting<SpindleCollection> {
        const char* collectionName = "spindles";

        void* parse(Settings::YamlParser& parser) override {
            SpindleCollection collection;

            while (parser.moveNext()) {
                auto builder = Settings::SettingsCollection::find(parser.key(), collectionName);
                Assert(builder != nullptr, "Settings invalid; incorrect key found: %s", parser.key());

                // Unfortunately we cannot use dynamic_cast here, because we lack RTTI.
                auto spindle = static_cast<Spindles::Spindle*>(builder->parse(parser));
                collection.spindles_.push_back(spindle);
            }

            // Use copy constructor here. It costs some time, but makes RAII much easier.
            return new SpindleCollection(collection);
        }
    };

    ~SpindleCollection() {
        for (auto spindle : spindles_) {
            delete spindle;
        }
    }
};

class Machine {
public:
    SpindleCollection* spindles_ = nullptr;
    // Axis, misc devices, etc.

    struct MySettings : Settings::Setting<Machine> {
        MySettings() : Setting("machine", nullptr) {}

        void* parse(Settings::YamlParser& parser) override {
            Machine machine;

            while (parser.moveNext()) {
                auto builder = Settings::SettingsCollection::find(parser.key(), nullptr);
                Assert(builder != nullptr, "Settings invalid; incorrect key found: %s", parser.key());

                // Unfortunately we cannot use dynamic_cast here, because we lack RTTI.
                if (!strcmp(parser.key(), "spindles")) {
                    Assert(machine.spindles_ == nullptr, "No spindles should be defined at this point. Only one spindle section is allowed.");
                    machine.spindles_ = static_cast<SpindleCollection*>(builder->parse(parser));
                }
                // else more sections...
            }

            // Use copy constructor here. It costs some time, but makes RAII much easier.
            return new Machine(machine);
        }
    };

    ~Machine() { delete spindles_; }
};

namespace Spindles {
    // This adds support for PWM
    class PWM : public Spindle {
    public:
        struct MySettings : Settings::Setting<Spindle> {
            MySettings() : Setting("pwm", "spindles") {}  // note we can make 'spindles' a const char* in the Spindle class

            Settings::PinSetting outputPin    = "output";
            Settings::PinSetting directionPin = "direction";
            Settings::PinSetting enablePin    = "enable";

            Spindle* create() override {
                if (outputPin.value() == Pin::UNDEFINED) {
                    throw Settings::SettingsError("Output pin is undefined.");
                }
                // etc.

                return new PWM(outputPin.value(), enablePin.value(), directionPin.value(), 0, 10000);
            }
        };
        PWM() = default;

        PWM(Pin output, Pin enable, Pin direction, uint32_t minRpm, uint32_t maxRpm) :
            _min_rpm(minRpm), _max_rpm(maxRpm), _output_pin(output), _enable_pin(enable), _direction_pin(direction) {}

        PWM(const PWM&) = delete;
        PWM(PWM&&)      = delete;
        PWM& operator=(const PWM&) = delete;
        PWM& operator=(PWM&&) = delete;

        void             init() override;
        virtual uint32_t set_rpm(uint32_t rpm) override;
        void             set_state(SpindleState state, uint32_t rpm) override;
        SpindleState     get_state() override;
        void             stop() override;
        void             config_message() override;

        virtual ~PWM() {}

    protected:
        int32_t  _current_pwm_duty;
        uint32_t _min_rpm;
        uint32_t _max_rpm;
        uint32_t _pwm_off_value;
        uint32_t _pwm_min_value;
        uint32_t _pwm_max_value;
        Pin      _output_pin;
        Pin      _enable_pin;
        Pin      _direction_pin;
        uint8_t  _pwm_chan_num;
        uint32_t _pwm_freq;
        uint32_t _pwm_period;  // how many counts in 1 period
        uint8_t  _pwm_precision;
        bool     _piecewide_linear;
        bool     _off_with_zero_speed;
        bool     _invert_pwm;
        //uint32_t _pwm_gradient; // Precalulated value to speed up rpm to PWM conversions.

        virtual void set_dir_pin(bool Clockwise);
        virtual void set_output(uint32_t duty);
        virtual void set_enable_pin(bool enable_pin);

        void    get_pins_and_settings();
        uint8_t calc_pwm_precision(uint32_t freq);
    };
}

// Register settings class works like this:
namespace {
    Spindles::PWM::MySettings pwmSettings;
}

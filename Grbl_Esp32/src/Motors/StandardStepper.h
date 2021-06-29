#pragma once

#include "../Machine/MachineConfig.h"  // config->_stepType
#include "../Stepper.h"                // ST_I2S_*
#include "Motor.h"

#include <driver/rmt.h>

namespace Motors {
    class StandardStepper : public Motor {
    public:
        //StandardStepper(uint8_t axis_index, Pin step_pin, Pin dir_pin, Pin disable_pin);

        StandardStepper() = default;

        // Overrides for inherited methods
        void init() override;

        // No special action, but return true to say homing is possible
        bool set_homing_mode(bool isHoming) override { return true; }
        void set_disable(bool) override;
        void set_direction(bool) override;
        void step() override;
        void unstep() override;
        void read_settings() override;

        void init_step_dir_pins();

    protected:
        void config_message() override;

        Pin _step_pin;
        Pin _dir_pin;
        Pin _disable_pin;

        // Configuration handlers:
        void validate() const override {
            Assert(_step_pin.defined(), "Step pin should be configured.");
            Assert(_dir_pin.defined(), "Direction pin should be configured.");
            bool isI2SO = config->_stepType == ST_I2S_STREAM || config->_stepType == ST_I2S_STATIC;
            Assert(!isI2SO || _step_pin.name().startsWith("I2SO"), "Step pin must be an I2SO pin");
            Assert(!isI2SO || _dir_pin.name().startsWith("I2SO"), "Direction pin must be an I2SO pin");
        }

        void group(Configuration::HandlerBase& handler) override {
            handler.item("step", _step_pin);
            handler.item("direction", _dir_pin);
            handler.item("disable", _disable_pin);
        }

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override { return "standard_stepper"; }

    private:
        // Initialized after configuration for RMT steps:
        bool _invert_step;
        bool _invert_disable;

        rmt_channel_t _rmt_chan_num;

        static rmt_channel_t get_next_RMT_chan_num();
        static rmt_item32_t  rmtItem[2];
        static rmt_config_t  rmtConfig;
    };
}

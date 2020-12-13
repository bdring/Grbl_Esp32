#pragma once

#include "Motor.h"

namespace Motors {
    class StandardStepper : public Motor {
    public:
        StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

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

#ifdef USE_RMT_STEPS
        rmt_channel_t _rmt_chan_num;
#endif
        bool    _invert_step_pin;
        bool    _invert_dir_pin;
        uint8_t _step_pin;
        uint8_t _dir_pin;
        uint8_t _disable_pin;

    private:
        static rmt_channel_t get_next_RMT_chan_num();
        static rmt_item32_t  rmtItem[2];
        static rmt_config_t  rmtConfig;
    };
}

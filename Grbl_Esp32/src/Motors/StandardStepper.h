#pragma once

#include "Motor.h"

namespace Motors {
    class StandardStepper : public Motor {
    public:
        StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

        virtual void config_message();
        virtual void init();
        virtual void set_direction(bool);
        void         step();
        void         unstep();
        void         init_step_dir_pins();
        virtual void set_disable(bool);

    protected:
#ifdef USE_RMT_STEPS
        rmt_channel_t _rmt_chan_num;
#endif
        bool    _invert_step_pin;
        bool    _invert_dir_pin;
        uint8_t _step_pin;
        uint8_t _dir_pin;
        uint8_t _disable_pin;
    };
}

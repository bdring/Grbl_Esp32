#pragma once

#include "Motor.h"

namespace Motors {
    class StandardStepper : public Motor {
    public:
        StandardStepper();
        StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

        virtual void config_message();
        virtual void init();
        virtual void set_direction_pins(uint8_t onMask);
        void         init_step_dir_pins();
        virtual void set_disable(bool disable);
        uint8_t      step_pin;

    protected:
        bool    _invert_step_pin;
        uint8_t dir_pin;
        uint8_t disable_pin;
    };
}

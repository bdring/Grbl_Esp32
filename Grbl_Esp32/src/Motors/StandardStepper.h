#pragma once

#include "Motor.h"
#include "../Pin.h"

namespace Motors {
    class StandardStepper : public Motor {
    public:
        StandardStepper();
        StandardStepper(uint8_t axis_index, Pin step_pin, Pin dir_pin, Pin disable_pin);

        virtual void config_message();
        virtual void init();
        virtual void set_direction_pins(uint8_t onMask);
        void         init_step_dir_pins();
        virtual void set_disable(bool disable);

    protected:
        bool _invert_step_pin;

        Pin dir_pin;
        Pin disable_pin;
        Pin step_pin;
    };
}

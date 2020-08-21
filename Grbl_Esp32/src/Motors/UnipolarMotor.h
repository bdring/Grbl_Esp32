#pragma once

#include "Motor.h"
#include "../Pin.h"

namespace Motors {
    class UnipolarMotor : public Motor {
    public:
        UnipolarMotor();
        UnipolarMotor(uint8_t axis_index, Pin pin_phase0, Pin pin_phase1, Pin pin_phase2, Pin pin_phase3);

        void init();
        void config_message();
        void set_disable(bool disable);
        void step(uint8_t step_mask, uint8_t dir_mask);  // only used on Unipolar right now

    private:
        Pin     _pin_phase0;
        Pin     _pin_phase1;
        Pin     _pin_phase2;
        Pin     _pin_phase3;
        uint8_t _current_phase;
        bool    _half_step;
        bool    _enabled;
    };
}

#pragma once

#include "Motor.h"

namespace Motors {
    class UnipolarMotor : public Motor {
    public:
        UnipolarMotor();
        UnipolarMotor(uint8_t axis_index, uint8_t pin_phase0, uint8_t pin_phase1, uint8_t pin_phase2, uint8_t pin_phase3);
        void init();
        void config_message();
        void set_disable(bool disable);
        void step(uint8_t step_mask, uint8_t dir_mask);  // only used on Unipolar right now

    private:
        uint8_t _pin_phase0;
        uint8_t _pin_phase1;
        uint8_t _pin_phase2;
        uint8_t _pin_phase3;
        uint8_t _current_phase;
        bool    _half_step;
        bool    _enabled;
    };
}

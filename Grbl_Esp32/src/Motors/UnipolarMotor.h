#pragma once

#include "Motor.h"

namespace Motors {
    class UnipolarMotor : public Motor {
    public:
        UnipolarMotor(uint8_t axis_index, uint8_t pin_phase0, uint8_t pin_phase1, uint8_t pin_phase2, uint8_t pin_phase3);

        // Overrides for inherited methods
        void init() override;
        bool set_homing_mode(bool isHoming) override { return true; }
        void set_disable(bool disable) override;
        void set_direction(bool) override;
        void step() override;

    private:
        uint8_t _pin_phase0;
        uint8_t _pin_phase1;
        uint8_t _pin_phase2;
        uint8_t _pin_phase3;
        uint8_t _current_phase;
        bool    _half_step;
        bool    _enabled;
        bool    _dir;

  protected:
        void config_message() override;
    };
}

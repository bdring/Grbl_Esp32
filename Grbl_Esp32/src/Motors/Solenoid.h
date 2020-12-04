#pragma once

#include "RcServo.h"

namespace Motors {
    class Solenoid : public RcServo {
    public:
        Solenoid();
        Solenoid(uint8_t axis_index, gpio_num_t pwm_pin, float transition_poiont);
        void set_location();
        void update() override;
        void init() override;
        void set_disable(bool disable) override;

        float _transition_poiont;
  protected:
        void config_message() override;
    };
}

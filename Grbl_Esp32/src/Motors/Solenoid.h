#pragma once

#include "RcServo.h"

namespace Motors {
    class Solenoid : public RcServo {
    public:
        Solenoid();
        Solenoid(uint8_t axis_index, gpio_num_t pwm_pin, float transition_poiont);
        void config_message();
        void set_location();
        void update();
        void init();
        void set_disable(bool disable);

        float _transition_poiont;
    };
}

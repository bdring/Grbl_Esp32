#pragma once

#include "RcServo.h"
#include "../Pin.h"

namespace Motors {
    class Solenoid : public RcServo {
        float _transition_poiont;

    public:
        Solenoid();
        Solenoid(uint8_t axis_index, Pin pwm_pin, float transition_poiont);

        void config_message();
        void set_location();
        void update();
        void init();
        void set_disable(bool disable);
    };
}

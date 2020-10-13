#pragma once

#include "Motor.h"

namespace Motors {
    class Nullmotor : public Motor {
    public:
        Nullmotor(uint8_t axis_index);
        bool set_homing_mode(bool isHoming) { return false; }
    };
}

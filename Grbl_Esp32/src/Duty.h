#pragma once

#include <cstdint>

class Duty {
    int32_t value_;

public:
    inline Duty() : value_(0) {}

    Duty(const Duty& o) = default;
    Duty& operator=(const Duty& o) = default;

    inline Duty(int percentage) {
        if (percentage <= 0) {
            value_ = 0;
        } else if (percentage >= 100) {
            value_ = 1000000000;
        } else {
            value_ = int32_t(percentage * 10000000);
        }
    }

    inline Duty(float percentage) {
        if (percentage <= 0.0f) {
            value_ = 0;
        } else if (percentage >= 100.0f) {
            value_ = 1000000000;
        } else {
            value_ = int32_t(percentage * 10000000);
        }
    }

    inline uint32_t get_duty_cycle(uint8_t resolutionBits) const {
        uint64_t tmp = value_ * uint64_t(1 << resolutionBits);
        tmp /= 10000000;
        return uint32_t(tmp);
    }

    inline float get_percentage() const { return float(value_) / float(10000000); }
};

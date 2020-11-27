#pragma once

/*
	PWMSpindle.h

	This is a full featured TTL PWM spindle This does not include speed/power
	compensation. Use the Laser class for that.

	Part of Grbl_ESP32
	2020 -	Bart Dring

	Grbl is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	Grbl is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	You should have received a copy of the GNU General Public License
	along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

*/
#include "Spindle.h"

namespace Spindles {
    // This adds support for PWM
    class PWM : public Spindle {
    public:
        PWM() = default;

        PWM(const PWM&) = delete;
        PWM(PWM&&)      = delete;
        PWM& operator=(const PWM&) = delete;
        PWM& operator=(PWM&&) = delete;

        void             init() override;
        virtual uint32_t set_rpm(uint32_t rpm) override;
        void             set_state(SpindleState state, uint32_t rpm) override;
        SpindleState     get_state() override;
        void             stop() override;
        void             config_message() override;

        virtual ~PWM() {}

    protected:
        int32_t  _current_pwm_duty;
        uint32_t _min_rpm;
        uint32_t _max_rpm;
        uint32_t _pwm_off_value;
        uint32_t _pwm_min_value;
        uint32_t _pwm_max_value;
        uint8_t  _output_pin;
        uint8_t  _enable_pin;
        uint8_t  _direction_pin;
        uint8_t  _pwm_chan_num;
        uint32_t _pwm_freq;
        uint32_t _pwm_period;  // how many counts in 1 period
        uint8_t  _pwm_precision;
        bool     _piecewide_linear;
        bool     _off_with_zero_speed;
        bool     _invert_pwm;
        //uint32_t _pwm_gradient; // Precalulated value to speed up rpm to PWM conversions.

        virtual void set_dir_pin(bool Clockwise);
        virtual void set_output(uint32_t duty);
        virtual void set_enable_pin(bool enable_pin);
        virtual void deinit();

        virtual void get_pins_and_settings();
        uint8_t      calc_pwm_precision(uint32_t freq);
    };
}

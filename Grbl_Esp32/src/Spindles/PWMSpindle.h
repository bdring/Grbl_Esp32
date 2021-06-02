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

        // PWM(Pin&& output, Pin&& enable, Pin&& direction, uint32_t minRpm, uint32_t maxRpm) :
        //     _min_rpm(minRpm), _max_rpm(maxRpm), _output_pin(std::move(output)), _enable_pin(std::move(enable)),
        //     _direction_pin(std::move(direction)) {}

        PWM(const PWM&) = delete;
        PWM(PWM&&)      = delete;
        PWM& operator=(const PWM&) = delete;
        PWM& operator=(PWM&&) = delete;

        void         init() override;
        virtual void set_rpm(uint32_t rpm) override;
        void         set_state(SpindleState state, uint32_t rpm) override;
        SpindleState get_state() override;
        void         stop() override;
        void         config_message() override;
        uint32_t     limitRPM(uint32_t RPM);
        uint32_t     RPMtoPWM(uint32_t rpm);
        // Configuration handlers:
        void validate() const override { Spindle::validate(); }

        void handle(Configuration::HandlerBase& handler) override {
            handler.handle("min_rpm", _min_rpm);
            handler.handle("max_rpm", _max_rpm);

            handler.handle("pwm_freq", _pwm_freq);
            handler.handle("pwm_off", _pwm_off_setting);
            handler.handle("pwm_min", _pwm_min_setting);
            handler.handle("pwm_max", _pwm_max_setting);
            handler.handle("invert_pwm", _invert_pwm);
            handler.handle("output_pin", _output_pin);
            handler.handle("enable_pin", _enable_pin);
            handler.handle("direction_pin", _direction_pin);
            handler.handle("enable_off_with_zero_speed", _off_with_zero_speed);

            Spindle::handle(handler);
        }

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override { return "PWM"; }

        virtual ~PWM() {}

    protected:
        uint32_t _pwm_off_setting;  // do we need these 3?
        uint32_t _pwm_min_setting;
        uint32_t _pwm_max_setting;

        int32_t  _current_pwm_duty;
        uint32_t _min_rpm;
        uint32_t _max_rpm;
        uint32_t _pwm_off;
        uint32_t _pwm_min;
        uint32_t _pwm_max;
        Pin      _output_pin;
        Pin      _enable_pin;
        Pin      _direction_pin;
        uint8_t  _pwm_chan_num;
        uint32_t _pwm_freq;
        uint32_t _pwm_period;  // how many counts in 1 period
        uint8_t  _pwm_precision;
        bool     _off_with_zero_speed;
        bool     _invert_pwm;

        virtual void set_direction(bool Clockwise);
        virtual void set_output(uint32_t duty);
        virtual void set_enable(bool enable);
        virtual void deinit();

        virtual void get_pins_and_settings();
        uint8_t      calc_pwm_precision(uint32_t freq);
    };
}

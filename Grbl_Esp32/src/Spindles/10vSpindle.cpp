/*
    10vSpindle.cpp


    This is basically a PWM spindle with some changes, so a separate forward and
    reverse signal can be sent.

    The direction pins will act as enables for the 2 directions. There is usually
    a min RPM with VFDs, that speed will remain even if speed is 0. You
    must turn off both direction pins when enable is off.


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
#include "10vSpindle.h"

namespace Spindles {
    void _10v::init() {
        get_pins_and_settings();  // these gets the standard PWM settings, but many need to be changed for BESC

        // a couple more pins not inherited from PWM Spindle
#ifdef SPINDLE_FORWARD_PIN
        _forward_pin = SPINDLE_FORWARD_PIN;
#else
        _forward_pin = Pin::UNDEFINED;
#endif

#ifdef SPINDLE_REVERSE_PIN
        _reverse_pin = SPINDLE_REVERSE_PIN;
#else
        _reverse_pin = Pin::UNDEFINED;
#endif

        if (_output_pin == Pin::UNDEFINED) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Warning: Spindle output pin not defined");
            return;  // We cannot continue without the output pin
        }

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(_output_pin, _pwm_chan_num);                    // attach the PWM to the pin

        pinMode(_enable_pin, OUTPUT);
        pinMode(_direction_pin, OUTPUT);
        pinMode(_forward_pin, OUTPUT);
        pinMode(_reverse_pin, OUTPUT);

        set_rpm(0);

        config_message();

        is_reversable = true;  // these VFDs are always reversable
        use_delays    = true;
    }

    // prints the startup message of the spindle config
    void _10v::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "0-10V spindle Out:%s Enbl:%s, Dir:%s, Fwd:%s, Rev:%s, Freq:%dHz Res:%dbits",
                       pinName(_output_pin).c_str(),
                       pinName(_enable_pin).c_str(),
                       pinName(_direction_pin).c_str(),
                       pinName(_forward_pin).c_str(),
                       pinName(_reverse_pin).c_str(),
                       _pwm_freq,
                       _pwm_precision);
    }

    uint32_t _10v::set_rpm(uint32_t rpm) {
        uint32_t pwm_value;

        if (_output_pin == Pin::UNDEFINED) {
            return rpm;
        }

        // apply speed overrides
        rpm = rpm * sys.spindle_speed_ovr / 100;  // Scale by spindle speed override value (percent)

        // apply limits limits
        if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
            rpm = _max_rpm;
        } else if (rpm != 0 && rpm <= _min_rpm) {
            rpm = _min_rpm;
        }
        sys.spindle_speed = rpm;

        // determine the pwm value
        if (rpm == 0) {
            pwm_value = _pwm_off_value;
        } else {
            pwm_value = map_uint32_t(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
        }

        set_output(pwm_value);
        return rpm;
    }

    /*
	void _10v::set_state(SpindleState state, uint32_t rpm) {
		if (sys.abort) {
            return;   // Block during abort.
        }

		if (state == SpindleState::Disable) { // Halt or set spindle direction and rpm.
			sys.spindle_speed = 0;
			stop();
		} else {
			set_dir_pin(state == SpindleState:Cw);
			set_rpm(rpm);
		}

		set_enable_pin(state != SpindleState::Disable);

		sys.report_ovr_counter = 0; // Set to report change immediately
	}
	*/

    SpindleState _10v::get_state() {
        if (_current_pwm_duty == 0 || _output_pin == Pin::UNDEFINED) {
            return SpindleState::Disable;
        }
        if (_direction_pin != Pin::UNDEFINED) {
            return digitalRead(_direction_pin) ? SpindleState::Cw : SpindleState::Ccw;
        }
        return SpindleState::Cw;
    }

    void _10v::stop() {
        // inverts are delt with in methods
        set_enable_pin(false);
        set_output(_pwm_off_value);
    }

    void _10v::set_enable_pin(bool enable) {
        //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Spindle::_10v::set_enable_pin");
        if (_off_with_zero_speed && sys.spindle_speed == 0) {
            enable = false;
        }

        if (spindle_enable_invert->get()) {
            enable = !enable;
        }

        digitalWrite(_enable_pin, enable);

        // turn off anything that acts like an enable
        if (!enable) {
            digitalWrite(_direction_pin, enable);
            digitalWrite(_forward_pin, enable);
            digitalWrite(_reverse_pin, enable);
        }
    }

    void _10v::set_dir_pin(bool Clockwise) {
        //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Spindle::_10v::set_dir_pin");
        digitalWrite(_direction_pin, Clockwise);
        digitalWrite(_forward_pin, Clockwise);
        digitalWrite(_reverse_pin, !Clockwise);
    }
}

/*
    PWMSpindle.cpp

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
#include "PWMSpindle.h"
#include "soc/ledc_struct.h"

// ======================= PWM ==============================
/*
    This gets called at startup or whenever a spindle setting changes
    If the spindle is running it will stop and need to be restarted with M3Snnnn
*/

//#include "grbl.h"

namespace Spindles {
    void PWM::init() {
        get_pins_and_settings();

        if (_output_pin == UNDEFINED_PIN) {
            return;  // We cannot continue without the output pin
        }

        if (_output_pin >= I2S_OUT_PIN_BASE) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: Spindle output pin %s cannot do PWM", pinName(_output_pin).c_str());
            return;
        }

        _current_state    = SpindleState::Disable;
        _current_pwm_duty = 0;
        use_delays        = true;

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(_output_pin, _pwm_chan_num);                    // attach the PWM to the pin
        pinMode(_enable_pin, OUTPUT);
        pinMode(_direction_pin, OUTPUT);

        config_message();
    }

    // Get the GPIO from the machine definition
    void PWM::get_pins_and_settings() {
        // setup all the pins

#ifdef SPINDLE_OUTPUT_PIN
        _output_pin = SPINDLE_OUTPUT_PIN;
#else
        _output_pin       = UNDEFINED_PIN;
#endif

        _invert_pwm = spindle_output_invert->get();

#ifdef SPINDLE_ENABLE_PIN
        _enable_pin = SPINDLE_ENABLE_PIN;
#else
        _enable_pin       = UNDEFINED_PIN;
#endif

        _off_with_zero_speed = spindle_enbl_off_with_zero_speed->get();

#ifdef SPINDLE_DIR_PIN
        _direction_pin = SPINDLE_DIR_PIN;
#else
        _direction_pin    = UNDEFINED_PIN;
#endif

        if (_output_pin == UNDEFINED_PIN) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: SPINDLE_OUTPUT_PIN not defined");
            return;  // We cannot continue without the output pin
        }

        is_reversable = (_direction_pin != UNDEFINED_PIN);

        _pwm_freq      = spindle_pwm_freq->get();
        _pwm_precision = calc_pwm_precision(_pwm_freq);  // detewrmine the best precision
        _pwm_period    = (1 << _pwm_precision);

        if (spindle_pwm_min_value->get() > spindle_pwm_min_value->get()) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: Spindle min pwm is greater than max. Check $35 and $36");
        }

        // pre-caculate some PWM count values
        _pwm_off_value = (_pwm_period * spindle_pwm_off_value->get() / 100.0);
        _pwm_min_value = (_pwm_period * spindle_pwm_min_value->get() / 100.0);
        _pwm_max_value = (_pwm_period * spindle_pwm_max_value->get() / 100.0);

#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
        _min_rpm          = RPM_MIN;
        _max_rpm          = RPM_MAX;
        _piecewide_linear = true;
#else
        _min_rpm          = rpm_min->get();
        _max_rpm          = rpm_max->get();
        _piecewide_linear = false;
#endif
        // The pwm_gradient is the pwm duty cycle units per rpm
        // _pwm_gradient = (_pwm_max_value - _pwm_min_value) / (_max_rpm - _min_rpm);

        _pwm_chan_num = 0;  // Channel 0 is reserved for spindle use

        _spinup_delay   = spindle_delay_spinup->get() * 1000.0;
        _spindown_delay = spindle_delay_spindown->get() * 1000.0;
    }

    uint32_t PWM::set_rpm(uint32_t rpm) {
        uint32_t pwm_value;

        if (_output_pin == UNDEFINED_PIN) {
            return rpm;
        }

        // apply override
        rpm = rpm * sys.spindle_speed_ovr / 100;  // Scale by spindle speed override value (uint8_t percent)

        // apply limits
        if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
            rpm = _max_rpm;
        } else if (rpm != 0 && rpm <= _min_rpm) {
            rpm = _min_rpm;
        }

        sys.spindle_speed = rpm;

        if (_piecewide_linear) {
            //pwm_value = piecewise_linear_fit(rpm); TODO
            pwm_value = 0;
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: Linear fit not implemented yet.");

        } else {
            if (rpm == 0) {
                pwm_value = _pwm_off_value;
            } else {
                pwm_value = map_uint32_t(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
            }
        }

        set_enable_pin(gc_state.modal.spindle != SpindleState::Disable);
        set_output(pwm_value);

        return 0;
    }

    void PWM::set_state(SpindleState state, uint32_t rpm) {
        if (sys.abort) {
            return;  // Block during abort.
        }

        if (state == SpindleState::Disable) {  // Halt or set spindle direction and rpm.
            sys.spindle_speed = 0;
            stop();
            if (use_delays && (_current_state != state)) {
                delay(_spinup_delay);
            }
        } else {
            set_dir_pin(state == SpindleState::Cw);
            set_rpm(rpm);
            set_enable_pin(state != SpindleState::Disable);  // must be done after setting rpm for enable features to work
            if (use_delays && (_current_state != state)) {
                delay(_spindown_delay);
            }
        }

        _current_state = state;

        sys.report_ovr_counter = 0;  // Set to report change immediately
    }

    SpindleState PWM::get_state() {
        if (_current_pwm_duty == 0 || _output_pin == UNDEFINED_PIN) {
            return SpindleState::Disable;
        }
        if (_direction_pin != UNDEFINED_PIN) {
            return digitalRead(_direction_pin) ? SpindleState::Cw : SpindleState::Ccw;
        }
        return SpindleState::Cw;
    }

    void PWM::stop() {
        // inverts are delt with in methods
        set_enable_pin(false);
        set_output(_pwm_off_value);
    }

    // prints the startup message of the spindle config
    void PWM::config_message() {
        grbl_msg_sendf(CLIENT_ALL,
                       MsgLevel::Info,
                       "PWM spindle Output:%s, Enbl:%s, Dir:%s, Freq:%dHz, Res:%dbits",
                       pinName(_output_pin).c_str(),
                       pinName(_enable_pin).c_str(),
                       pinName(_direction_pin).c_str(),
                       _pwm_freq,
                       _pwm_precision);
    }

    void PWM::set_output(uint32_t duty) {
        if (_output_pin == UNDEFINED_PIN) {
            return;
        }

        // to prevent excessive calls to ledcWrite, make sure duty hass changed
        if (duty == _current_pwm_duty) {
            return;
        }

        _current_pwm_duty = duty;

        if (_invert_pwm) {
            duty = (1 << _pwm_precision) - duty;
        }

        //ledcWrite(_pwm_chan_num, duty);

        // This was ledcWrite, but this is called from an ISR
        // and ledcWrite uses RTOS features not compatible with ISRs
        LEDC.channel_group[0].channel[0].duty.duty        = duty << 4;
        bool on                                           = !!duty;
        LEDC.channel_group[0].channel[0].conf0.sig_out_en = on;
        LEDC.channel_group[0].channel[0].conf1.duty_start = on;
        LEDC.channel_group[0].channel[0].conf0.clk_en     = on;
    }

    void PWM::set_enable_pin(bool enable) {
        // static bool prev_enable = false;

        // if (prev_enable == enable) {
        //     return;
        // }

        // prev_enable = enable;

        if (_enable_pin == UNDEFINED_PIN) {
            return;
        }

        if (_off_with_zero_speed && sys.spindle_speed == 0) {
            enable = false;
        }

        if (spindle_enable_invert->get()) {
            enable = !enable;
        }

        digitalWrite(_enable_pin, enable);
    }

    void PWM::set_dir_pin(bool Clockwise) { digitalWrite(_direction_pin, Clockwise); }

    /*
		Calculate the highest precision of a PWM based on the frequency in bits

		80,000,000 / freq = period
		determine the highest precision where (1 << precision) < period
	*/
    uint8_t PWM::calc_pwm_precision(uint32_t freq) {
        uint8_t precision = 0;

        // increase the precision (bits) until it exceeds allow by frequency the max or is 16
        while ((1 << precision) < (uint32_t)(80000000 / freq) && precision <= 16) {
            precision++;
        }

        return precision - 1;
    }

    void PWM::deinit() {
        stop();
#ifdef SPINDLE_OUTPUT_PIN
        gpio_reset_pin(SPINDLE_OUTPUT_PIN);
        pinMode(SPINDLE_OUTPUT_PIN, INPUT);
#endif
#ifdef SPINDLE_ENABLE_PIN
        gpio_reset_pin(SPINDLE_ENABLE_PIN);
        pinMode(SPINDLE_ENABLE_PIN, INPUT);
#endif

#ifdef SPINDLE_DIR_PIN
        gpio_reset_pin(SPINDLE_DIR_PIN);
        pinMode(SPINDLE_DIR_PIN, INPUT);
#endif
    }
}

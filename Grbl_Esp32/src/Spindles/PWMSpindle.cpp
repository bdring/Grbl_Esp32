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
#include "../Logging.h"

// ======================= PWM ==============================
/*
    This gets called at startup or whenever a spindle setting changes
    If the spindle is running it will stop and need to be restarted with M3Snnnn
*/

//#include "grbl.h"

namespace Spindles {
    void PWM::init() {
        if (_pwm_freq == 0) {
            log_error("Spindle PWM frequency is 0.");
            return;
        }

        get_pins_and_settings();

        if (_output_pin.undefined()) {
            log_warn("Spindle output pin not defined");
            return;  // We cannot continue without the output pin
        }

        if (!_output_pin.capabilities().has(Pin::Capabilities::PWM)) {
            log_warn("Spindle output pin %s cannot do PWM" << _output_pin.name().c_str());
            return;
        }

        _current_state    = SpindleState::Disable;
        _current_pwm_duty = 0;
        use_delays        = true;

        auto outputNative = _output_pin.getNative(Pin::Capabilities::PWM);

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(outputNative, _pwm_chan_num);                   // attach the PWM to the pin

        _enable_pin.setAttr(Pin::Attr::Output);
        _direction_pin.setAttr(Pin::Attr::Output);

        use_delays = true;

        config_message();
    }

    // Get the GPIO from the machine definition
    void PWM::get_pins_and_settings() {
        // setup all the pins

        is_reversable = _direction_pin.defined();

        _pwm_precision = calc_pwm_precision(_pwm_freq);  // determine the best precision
        _pwm_period    = (1 << _pwm_precision);

        if (_pwm_min_setting > _pwm_max_setting) {
            log_warn("Spindle min PWM is greater than max.");
        }

        // pre-calculate some PWM count values
        _pwm_off = uint32_t(_pwm_period * _pwm_off_setting / 100.0);
        _pwm_min = uint32_t(_pwm_period * _pwm_min_setting / 100.0);
        _pwm_max = uint32_t(_pwm_period * _pwm_max_setting / 100.0);

        _pwm_chan_num = 0;  // Channel 0 is reserved for spindle use
    }

    void IRAM_ATTR PWM::set_rpm(uint32_t rpm) {
        sys.spindle_speed = rpm = limitRPM(overrideRPM(rpm));

        set_enable(gc_state.modal.spindle != SpindleState::Disable);
        set_output(RPMtoPWM(rpm));
    }

    void PWM::set_state(SpindleState state, uint32_t rpm) {
        if (sys.abort) {
            return;  // Block during abort.
        }

        if (state == SpindleState::Disable) {  // Halt or set spindle direction and rpm.
            sys.spindle_speed = 0;
            stop();
            if (use_delays && (_current_state != state)) {
                delay(_spindown_delay);
            }
        } else {
            set_direction(state == SpindleState::Cw);
            set_rpm(rpm);
            set_enable(state != SpindleState::Disable);  // must be done after setting rpm for enable features to work
            if (use_delays && (_current_state != state)) {
                delay(_spinup_delay);
            }
        }

        _current_state = state;

        sys.report_ovr_counter = 0;  // Set to report change immediately
    }

    SpindleState PWM::get_state() {
        if (_current_pwm_duty == 0 || _output_pin.undefined()) {
            return SpindleState::Disable;
        }
        if (_direction_pin.defined()) {
            return _direction_pin.read() ? SpindleState::Cw : SpindleState::Ccw;
        }
        return SpindleState::Cw;
    }

    void PWM::stop() {
        set_enable(false);
        set_output(_pwm_off);
    }

    // prints the startup message of the spindle config
    void PWM::config_message() {
        log_info("PWM spindle Output:" << _output_pin.name().c_str() << ", Enbl:" << _enable_pin.name().c_str() << ", Dir:"
                                       << _direction_pin.name().c_str() << ", Freq:" << _pwm_freq << "Hz, Res:" << _pwm_precision << "bits"

        );
    }

    void IRAM_ATTR PWM::set_output(uint32_t duty) {
        if (_output_pin.undefined()) {
            return;
        }

        // to prevent excessive calls to ledcWrite, make sure duty has changed
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
    }

    void PWM::set_enable(bool enable) {
        if (_enable_pin.undefined()) {
            return;
        }

        if (_off_with_zero_speed && sys.spindle_speed == 0) {
            enable = false;
        }

        _enable_pin.write(enable);
    }

    void PWM::set_direction(bool Clockwise) { _direction_pin.write(Clockwise); }

    /*
		Calculate the highest precision of a PWM based on the frequency in bits

		80,000,000 / freq = period
		determine the highest precision where (1 << precision) < period
	*/
    uint8_t PWM::calc_pwm_precision(uint32_t freq) {
        uint8_t precision = 0;
        if (freq == 0) {
            return precision;
        }

        // increase the precision (bits) until it exceeds allow by frequency the max or is 16
        while ((1 << precision) < (uint32_t)(80000000 / freq) && precision <= 16) {
            precision++;
        }

        return precision - 1;
    }

    void PWM::deinit() {
        stop();
        ledcDetachPin(_output_pin.getNative(Pin::Capabilities::PWM));
        _output_pin.setAttr(Pin::Attr::Input);
        _enable_pin.setAttr(Pin::Attr::Input);
        _direction_pin.setAttr(Pin::Attr::Input);
    }

    uint32_t IRAM_ATTR PWM::limitRPM(uint32_t rpm) { return rpm ? constrain(rpm, _min_rpm, _max_rpm) : 0; }

    uint32_t IRAM_ATTR PWM::RPMtoPWM(uint32_t rpm) {
#ifdef PIECEWISE_LINEAR
        // This is a num_segments generalization of the code after the endif.
        // This two-segment array gives the same results as the old code
        //    [{0, _pwm_off}, {_min_rpm, _pwm_min}, {_max_rpm, _pwm_max}]
        if (rpm < rpm_in[0]) {
            return pwm_out[0];
        }
        int i;
        for (i = 0; i < num_segments; i++) {
            if (rpm >= rpm_in[i]) {
                break;
            }
        }
        if (i == num_segments) {
            return pwm_out[num_segments];
        }
        _min_rpm = rpm_in[i];
        _max_rpm = rpm_in[i + 1];
        _pwm_min = pwm_out[i];
        _pwm_max = pwm_out[i + 1];
#endif
        if (rpm == 0 || rpm < _min_rpm) {
            return _pwm_off;
        }
        return (rpm - _min_rpm) * (_pwm_max - _pwm_min) / (_max_rpm - _min_rpm) + _pwm_min;
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<PWM> registration("PWM");
    }
}

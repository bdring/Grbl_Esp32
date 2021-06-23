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

#include <soc/ledc_struct.h>
#include <driver/ledc.h>

#include "../System.h"  // sys.report_ovr_counter
#include "../GCode.h"   // gc_state.modal
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
            log_error(name() << " PWM frequency is 0.");
            return;
        }

        get_pins_and_settings();
        setupSpeeds(_pwm_freq);

        if (_output_pin.undefined()) {
            log_warn(name() << " output pin not defined");
            return;  // We cannot continue without the output pin
        }

        if (!_output_pin.capabilities().has(Pin::Capabilities::PWM)) {
            log_warn(name() << " output pin " << _output_pin.name().c_str() << " cannot do PWM");
            return;
        }

        _current_state    = SpindleState::Disable;
        _current_pwm_duty = 0;

        auto outputNative = _output_pin.getNative(Pin::Capabilities::PWM);

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(outputNative, _pwm_chan_num);                   // attach the PWM to the pin

        _enable_pin.setAttr(Pin::Attr::Output);
        _direction_pin.setAttr(Pin::Attr::Output);

        if (_speeds.size() == 0) {
            // The default speed map for a PWM spindle is linear from 0=0% to 10000=100%
            _speeds.push_back({ 0, 0 });
            _speeds.push_back({ 10000, 100 });
        }
        setupSpeeds(_pwm_period);
        config_message();
    }

    // Get the GPIO from the machine definition
    void PWM::get_pins_and_settings() {
        // setup all the pins

        is_reversable = _direction_pin.defined();

        _pwm_precision = calc_pwm_precision(_pwm_freq);  // determine the best precision
        _pwm_period    = (1 << _pwm_precision);

        _pwm_chan_num = 0;  // Channel 0 is reserved for spindle use
    }

    void PWM::setSpeedfromISR(uint32_t dev_speed) {
        set_enable(gc_state.modal.spindle != SpindleState::Disable);
        set_output(dev_speed);
    }

    // XXX this is the same as OnOff::setState so it might be possible to combine them
    void PWM::setState(SpindleState state, SpindleSpeed speed) {
        if (sys.abort) {
            return;  // Block during abort.
        }

        // We always use mapSpeed() with the unmodified input speed so it sets
        // sys.spindle_speed correctly.
        uint32_t dev_speed = mapSpeed(speed);
        if (state == SpindleState::Disable) {  // Halt or set spindle direction and speed.
            if (_zero_speed_with_disable) {
                dev_speed = offSpeed();
            }
        } else {
            // XXX this could wreak havoc if the direction is changed without first
            // spinning down.
            set_direction(state == SpindleState::Cw);
        }
        // set_output must go first because of the way enable is used for level
        // converters on some boards.
        set_output(dev_speed);
        set_enable(state != SpindleState::Disable);
        spinDelay(state, speed);

        sys.report_ovr_counter = 0;  // Set to report change immediately
    }

    // prints the startup message of the spindle config
    void PWM::config_message() {
        log_info(name() << " spindle Output:" << _output_pin.name().c_str() << ", Ena:" << _enable_pin.name().c_str()
                        << ", Dir:" << _direction_pin.name().c_str() << ", Freq:" << _pwm_freq << "Hz, Res:" << _pwm_precision << "bits"

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

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<PWM> registration("PWM");
    }
}

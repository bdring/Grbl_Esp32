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
        if (_output_pin.undefined()) {
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Warning: Spindle output pin not defined");
            return;  // We cannot continue without the output pin
        }

        auto outputPin = _output_pin.getNative(Pin::Capabilities::PWM);

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(outputPin, _pwm_chan_num);                      // attach the PWM to the pin

        _enable_pin.setAttr(Pin::Attr::Output);
        _direction_pin.setAttr(Pin::Attr::Output);
        _forward_pin.setAttr(Pin::Attr::Output);
        _reverse_pin.setAttr(Pin::Attr::Output);

        set_rpm(0);

        config_message();

        is_reversable = true;  // these VFDs are always reversable
        use_delays    = true;
    }

    // prints the startup message of the spindle config
    void _10v::config_message() {
        grbl_msg_sendf(CLIENT_ALL,
                       MsgLevel::Info,
                       "0-10V spindle Out:%s Enbl:%s, Dir:%s, Fwd:%s, Rev:%s, Freq:%dHz Res:%dbits",
                       _output_pin.name().c_str(),
                       _enable_pin.name().c_str(),
                       _direction_pin.name().c_str(),
                       _forward_pin.name().c_str(),
                       _reverse_pin.name().c_str(),
                       _pwm_freq,
                       _pwm_precision);
    }

    // This appears identical to the code in PWMSpindle.cpp but
    // it uses the 10v versions of set_enable and set_output
    void IRAM_ATTR _10v::set_rpm(uint32_t rpm) {
        sys.spindle_speed = rpm = limitRPM(overrideRPM(rpm));

        set_enable(gc_state.modal.spindle != SpindleState::Disable);
        set_output(RPMtoPWM(rpm));
    }

    void _10v::stop() {
        set_enable(false);
        set_output(_pwm_off);
    }

    void _10v::set_enable(bool enable) {
        if (_off_with_zero_speed && sys.spindle_speed == 0) {
            enable = false;
        }

        _enable_pin.write(enable);

        // turn off anything that acts like an enable
        if (!enable) {
            _direction_pin.write(enable);
            _forward_pin.write(enable);
            _reverse_pin.write(enable);
        }
    }

    void _10v::set_direction(bool Clockwise) {
        _direction_pin.write(Clockwise);
        _forward_pin.write(Clockwise);
        _reverse_pin.write(!Clockwise);
    }

    void _10v::deinit() {
        _enable_pin.setAttr(Pin::Attr::Input);
        _direction_pin.setAttr(Pin::Attr::Input);
        _forward_pin.setAttr(Pin::Attr::Input);
        _reverse_pin.setAttr(Pin::Attr::Input);
        ledcDetachPin(_output_pin.getNative(Pin::Capabilities::PWM));
        _output_pin.setAttr(Pin::Attr::Input);
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<_10v> registration("10V");
    }
}

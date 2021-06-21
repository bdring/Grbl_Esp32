/*
    BESCSpindle.cpp

    This a special type of PWM spindle for RC type Brushless DC Speed
    controllers. They use a short pulse for off and a longer pulse for
    full on. The pulse is always a small portion of the full cycle.
    Some BESCs have a special turn on procedure. This may be a one time
    procedure or must be done every time. The user must do that via gcode.

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

    Important ESC Settings
    50 Hz this is a typical frequency for an ESC
    Some ESCs can handle higher frequencies, but there is no advantage to changing it.

    Determine the typical min and max pulse length of your ESC
    _min_pulse_secs is typically 1ms (0.001 sec) or less
    _max_pulse_secs is typically 2ms (0.002 sec) or more

*/
#include "BESCSpindle.h"

namespace Spindles {
    void BESC::init() {
        if (_output_pin.undefined()) {
            info_all("Warning: BESC output pin not defined");
            return;  // We cannot continue without the output pin
        }

        is_reversable = _direction_pin.defined();
        _pwm_chan_num = 0;  // Channel 0 is reserved for spindle use

        // override some settings in the PWM base class to what is required for a BESC
        _pwm_freq      = besc_pwm_freq;
        _pwm_precision = 16;
        _pwm_period    = (1 << _pwm_precision);

        auto outputNative = _output_pin.getNative(Pin::Capabilities::PWM);
        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(outputNative, _pwm_chan_num);                   // attach the PWM to the pin

        _enable_pin.setAttr(Pin::Attr::Output);

        // 1000000 is us/sec
        const uint32_t besc_pulse_period_us = 1000000 / besc_pwm_freq;

        float _min_pulse_percent = 100.0 * _min_pulse_us / besc_pulse_period_us;
        float _max_pulse_percent = 100.0 * _max_pulse_us / besc_pulse_period_us;

        uint32_t max_speed = 20000;  // Default value if none given in speeds:
        if (_speeds.size() != 0) {
            log_info("Overriding PWM speed map for BESC");
            // Extract the maximum speed from the provide speed map
            max_speed = maxSpeed();
        }

        // BESC PWM typically represents 0 speed as a 1ms pulse and max speed as a 2ms pulse
        _speeds.clear();
        _speeds.push_back({ 0, _min_pulse_percent });
        _speeds.push_back({ max_speed, _max_pulse_percent });

        setupSpeeds(_pwm_period);

        stop();
        config_message();
    }

    // prints the startup message of the spindle config
    void BESC::config_message() {
        info_all("BESC spindle on Pin:%s Min:%dus Max:%dus Freq:%dHz Res:%dbits",
                 _output_pin.name().c_str(),
                 _min_pulse_us,
                 _max_pulse_us,
                 _pwm_freq,
                 _pwm_precision);
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<BESC> registration("BESC");
    }
}

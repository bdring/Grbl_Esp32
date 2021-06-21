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
    BESC_MIN_PULSE_SECS is typically 1ms (0.001 sec) or less
    BESC_MAX_PULSE_SECS is typically 2ms (0.002 sec) or more

*/
#include "BESCSpindle.h"

namespace Spindles {
    void BESC::init() {
        get_pins_and_settings();  // these gets the standard PWM settings, but many need to be changed for BESC

        if (_output_pin.undefined()) {
            info_all("Warning: BESC output pin not defined");
            return;  // We cannot continue without the output pin
        }

        // override some settings to what is required for a BESC
        _pwm_freq      = (uint32_t)BESC_PWM_FREQ;
        _pwm_precision = 16;

        auto outputPin = _output_pin.getNative(Pin::Capabilities::PWM);

        ledcSetup(_pwm_chan_num, (double)_pwm_freq, _pwm_precision);  // setup the channel
        ledcAttachPin(outputPin, _pwm_chan_num);                      // attach the PWM to the pin

        _enable_pin.setAttr(Pin::Attr::Output);

        stop();

#ifdef LATER
        // XXX these need to be folded into the speed map
        _pwm_off = BESC_MIN_PULSE_CNT;
        _pwm_min = _pwm_off;
        _pwm_max = BESC_MAX_PULSE_CNT;
#endif

        config_message();
    }

    // prints the startup message of the spindle config
    void BESC::config_message() {
        info_all("BESC spindle on Pin:%s Min:%0.2fms Max:%0.2fms Freq:%dHz Res:%dbits",
                 _output_pin.name().c_str(),
                 BESC_MIN_PULSE_SECS * 1000.0,  // convert to milliseconds
                 BESC_MAX_PULSE_SECS * 1000.0,  // convert to milliseconds
                 _pwm_freq,
                 _pwm_precision);
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<BESC> registration("BESC");
    }
}

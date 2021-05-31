/*
    DacSpindle.cpp

    This uses the Analog DAC in the ESP32 to generate a voltage
    proportional to the GCode S value desired. Some spindle uses
    a 0-5V or 0-10V value to control the spindle. You would use
    an Op Amp type circuit to get from the 0.3.3V of the ESP32 to that voltage.

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
#include "DacSpindle.h"

namespace Spindles {
    // ======================================== Dac ======================================
    void Dac::init() {
        get_pins_and_settings();

        if (_output_pin.undefined()) {
            return;
        }

        _pwm_min = 0;    // not actually PWM...DAC counts
        _pwm_max = 255;  // not actually PWM...DAC counts
        _gpio_ok = true;

        if (!_output_pin.capabilities().has(Pin::Capabilities::DAC)) {  // DAC can only be used on these pins
            _gpio_ok = false;
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "DAC spindle pin invalid %s (pin 25 or 26 only)", _output_pin.name().c_str());
            return;
        }

        _enable_pin.setAttr(Pin::Attr::Output);
        _direction_pin.setAttr(Pin::Attr::Output);

        is_reversable = _direction_pin.defined();
        use_delays    = true;

        config_message();
    }

    void Dac::config_message() {
        grbl_msg_sendf(CLIENT_ALL,
                       MsgLevel::Info,
                       "DAC spindle Output:%s, Enbl:%s, Dir:%s, Res:8bits",
                       _output_pin.name().c_str(),
                       _enable_pin.name().c_str(),
                       _direction_pin.name().c_str());
    }

    uint32_t Dac::set_rpm(uint32_t rpm) {
        sys.spindle_speed = rpm = limitRPM(overrideRPM(rpm));

        set_output(RPMtoPWM(rpm));
        return rpm;
    }

    void Dac::set_output(uint32_t duty) {
        if (_gpio_ok) {
            auto outputNative = _output_pin.getNative(Pin::Capabilities::DAC);

            dacWrite(outputNative, static_cast<uint8_t>(duty));
        }
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<Dac> registration("DAC");
    }
}

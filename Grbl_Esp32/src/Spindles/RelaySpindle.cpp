/*
    RelaySpindle.cpp

    This is used for a basic on/off spindle All S Values above 0
    will turn the spindle on.

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
#include "RelaySpindle.h"

// ========================= Relay ==================================

namespace Spindles {
    /*
    This is a sub class of PWM but is a digital rather than PWM output
    */
    void Relay::init() {
        get_pins_and_settings();

        if (_output_pin.undefined()) {
            return;
        }

        _output_pin.setAttr(Pin::Attr::Output);
        _enable_pin.setAttr(Pin::Attr::Output);
        _direction_pin.setAttr(Pin::Attr::Output);

        is_reversable = _direction_pin.defined();
        use_delays    = true;

        config_message();
    }

    // prints the startup message of the spindle config
    void Relay ::config_message() {
        info_all(
            "Relay spindle Output:%s, Enbl:%s, Dir:%s", _output_pin.name().c_str(), _enable_pin.name().c_str(), _direction_pin.name().c_str());
    }

    void IRAM_ATTR Relay::set_rpm(uint32_t rpm) {
        sys.spindle_speed = rpm = overrideRPM(rpm);
        set_output(rpm != 0);
    }

    void Relay::set_output(uint32_t duty) {
#ifdef INVERT_SPINDLE_PWM
        duty = (duty == 0);  // flip duty
#endif
        _output_pin.write(duty > 0);  // anything greater
    }

    // Configuration registration
    namespace {
        SpindleFactory::InstanceBuilder<Relay> registration("Relay");
    }
}

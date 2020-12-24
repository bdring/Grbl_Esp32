#pragma once

/*
    UserOutput.h

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

#include "Grbl.h"

namespace UserOutput {
    class DigitalOutput {
    public:
        DigitalOutput();
        DigitalOutput(uint8_t number, uint8_t pin);

        bool set_level(bool isOn);

    protected:
        void init();
        void config_message();

        uint8_t _number = UNDEFINED_PIN;
        uint8_t _pin    = UNDEFINED_PIN;
    };

    class AnalogOutput {
    public:
        AnalogOutput();
        AnalogOutput(uint8_t number, uint8_t pin, float pwm_frequency);
        bool     set_level(uint32_t numerator);
        uint32_t denominator() { return 1UL << _resolution_bits; };

    protected:
        void init();
        void config_message();

        uint8_t  _number      = UNDEFINED_PIN;
        uint8_t  _pin         = UNDEFINED_PIN;
        uint8_t  _pwm_channel = -1;  // -1 means invalid or not setup
        float    _pwm_frequency;
        uint8_t  _resolution_bits;
        uint32_t _current_value;
    };
}

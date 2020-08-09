#pragma once

/*
	10vSpindle.h

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

#include "PWMSpindle.h"

namespace Spindles {
    class _10vSpindle : public PWMSpindle {
    public:
        void     init();
        void     config_message();
        uint32_t set_rpm(uint32_t rpm);
        uint8_t  _forward_pin;
        uint8_t  _reverse_pin;

        //void set_state(uint8_t state, uint32_t rpm);

        uint8_t get_state();
        void    stop();

    protected:
        void set_enable_pin(bool enable_pin);
        void set_spindle_dir_pin(bool Clockwise);
    };
}

#pragma once

/*
	NullSpindle.h

	This is used when you don't want to use a spindle No I/O will be used
	and most methods don't do anything

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
#include "Spindle.h"

namespace Spindles {
    // This is a dummy spindle that has no I/O.
    // It is used to ignore spindle commands when no spinde is desired
    class NullSpindle : public Spindle {
    public:
        void     init();
        uint32_t set_rpm(uint32_t rpm);
        void     set_state(uint8_t state, uint32_t rpm);
        uint8_t  get_state();
        void     stop();
        void     config_message();
    };
}

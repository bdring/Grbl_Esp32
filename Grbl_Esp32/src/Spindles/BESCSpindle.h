#pragma once

/*
	BESCSpindle.h

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

#include "PWMSpindle.h"

namespace Spindles {
    class BESC : public PWM {
    public:
        BESC() = default;

        BESC(const BESC&) = delete;
        BESC(BESC&&)      = delete;
        BESC& operator=(const BESC&) = delete;
        BESC& operator=(BESC&&) = delete;

        void     init() override;
        void     config_message() override;
        uint32_t set_rpm(uint32_t rpm) override;

        virtual ~BESC() {}
    };
}

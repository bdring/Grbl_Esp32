#pragma once

/*
	RelaySpindle.h

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
#include "PWMSpindle.h"

namespace Spindles {
    // This is for an on/off spindle all RPMs above 0 are on
    class Relay : public PWM {
    public:
        Relay() = default;

        Relay(const Relay&) = delete;
        Relay(Relay&&)      = delete;
        Relay& operator=(const Relay&) = delete;
        Relay& operator=(Relay&&) = delete;

        void     init() override;
        void     config_message() override;
        uint32_t set_rpm(uint32_t rpm) override;

        virtual ~Relay() {}

    protected:
        void set_output(uint32_t duty);
    };
}

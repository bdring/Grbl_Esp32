#pragma once

/*
	TickleMeLaser.h

	This is similar to the Laser except that it allows for a tickle pulse 
  to meet the requirements to keep RF CO2 lasers (like Synrad tubes) 
  primed for operation.

	Add on Spindle Code for Grbl_ESP32
	2020 -	Taylor Fry

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
#include "Laser.h"

namespace Spindles {
    // this is the same as a PWM spindle but the M4 compensation is supported.
    class Laser : public PWM {
    public:
        Laser() = default;

        Laser(const Laser&) = delete;
        Laser(Laser&&)      = delete;
        Laser& operator=(const Laser&) = delete;
        Laser& operator=(Laser&&) = delete;

        bool isRateAdjusted() override;
        void config_message() override;

        virtual ~Laser() {}
    };
}

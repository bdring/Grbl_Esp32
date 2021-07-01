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
    class TickleMeLaser : public Laser {
    public:
        TickleMeLaser() = default;

        TickleMeLaser(const TickleMeLaser&) = delete;
        TickleMeLaser(TickleMeLaser&&)      = delete;
        TickleMeLaser& operator=(const TickleMeLaser&) = delete;
        TickleMeLaser& operator=(TickleMeLaser&&) = delete;

        uint32_t set_rpm(uint32_t rpm) override;

        virtual ~TickleMeLaser()  {}
    };
}

#pragma once

/*
    TrinamicBase.h

    Part of Grbl_ESP32
    2021 - Stefan de Bruijn

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

#include <cstdint>
#include "StandardStepper.h"

#ifndef TRINAMIC_RUN_MODE
#    define TRINAMIC_RUN_MODE TrinamicMode ::StealthChop
#endif

#ifndef TRINAMIC_HOMING_MODE
#    define TRINAMIC_HOMING_MODE TRINAMIC_RUN_MODE
#endif

namespace Motors {

    enum class TrinamicMode : uint8_t {
        None        = 0,  // not for machine defs!
        StealthChop = 1,
        CoolStep    = 2,
        StallGuard  = 3,
    };

    class TrinamicBase : public StandardStepper {
    protected:
        uint32_t calc_tstep(float speed, float percent);

        uint16_t     _driver_part_number;  // example: use 2130 for TMC2130
        TrinamicMode _homing_mode;
        float        _r_sense;
        bool         _has_errors;
        bool         _disabled;

        float _run_current         = 0.25;
        float _hold_current        = 0.25;
        int   _microsteps          = 256;
        int   _stallguard          = 0;
        bool  _stallguardDebugMode = false;

        TrinamicMode _mode = TrinamicMode::None;

        uint8_t get_next_index();

        // Linked list of Trinamic driver instances, used by the
        // StallGuard reporting task.
        static TrinamicBase* List;
        TrinamicBase*        link;
        static void          readSgTask(void*);

        const double TRINAMIC_FCLK = 12700000.0;  // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

    public:
        TrinamicBase(uint16_t partNumber) : StandardStepper(), _driver_part_number(partNumber), _homing_mode(TRINAMIC_HOMING_MODE) {}

        void group(Configuration::HandlerBase& handler) override {
            handler.item("r_sense", _r_sense);
            handler.item("run_current", _run_current);
            handler.item("hold_current", _hold_current);
            handler.item("microsteps", _microsteps);
            handler.item("stallguard", _stallguard);
            handler.item("stallguardDebugMode", _stallguardDebugMode);

            StandardStepper::group(handler);
        }
    };

}

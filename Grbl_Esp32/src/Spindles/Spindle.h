#pragma once

/*
    Spindle.h

    Header file for a Spindle Class
    See Spindle.cpp for more details

    Part of Grbl_ESP32

    2020 -	Bart Dring This file was modified for use on the ESP32
                    CPU. Do not use this with Grbl for atMega328P

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

    See SpindleClass.cpp for more info and references

*/

#include <cstdint>

#include "../GCode.h"     // for SpindleState
#include "../Report.h"    // for MsgLevel
#include "../Serial.h"    // for CLIENT_ALL
#include "../Protocol.h"  // for protocol_buffer_synchronize

#include "../Configuration/GenericFactory.h"

// ===============  No floats! ===========================
// ================ NO FLOATS! ==========================

namespace Spindles {
    // This is the base class. Do not use this as your spindle
    class Spindle : public Configuration::Configurable {
    public:
        Spindle() = default;

        Spindle(const Spindle&) = delete;
        Spindle(Spindle&&)      = delete;
        Spindle& operator=(const Spindle&) = delete;
        Spindle& operator=(Spindle&&) = delete;

        virtual void         init()                = 0;  // not in constructor because this also gets called when $$ settings change
        virtual void         set_rpm(uint32_t rpm) = 0;
        virtual void         set_state(SpindleState state, uint32_t rpm) = 0;
        virtual SpindleState get_state()                                 = 0;
        virtual void         stop()                                      = 0;
        virtual void         config_message()                            = 0;
        virtual bool         isRateAdjusted();
        virtual void         sync(SpindleState state, uint32_t rpm);

        virtual IRAM_ATTR void setRPMfromISR(uint32_t rpm) { set_rpm(rpm); }

        void                      spinDown() { set_state(SpindleState::Disable, 0); }
        static uint32_t IRAM_ATTR overrideRPM(uint32_t rpm) { return rpm * sys.spindle_speed_ovr / 100; }

        bool                  is_reversable;
        bool                  use_delays;  // will SpinUp and SpinDown delays be used.
        volatile SpindleState _current_state = SpindleState::Disable;

        uint32_t _spinup_delay;
        uint32_t _spindown_delay;

        // Name is required for the configuration factory to work.
        virtual const char* name() const = 0;

        // Configuration handlers:
        void validate() const override {
            // TODO: Validate spinup/spindown delay?
        }

        void group(Configuration::HandlerBase& handler) override {
            handler.item("spinup_delay_ms", _spinup_delay);
            handler.item("spindown_delay_ms", _spindown_delay);
        }

        // Virtual base classes require a virtual destructor.
        virtual ~Spindle() {}
    };
    using SpindleFactory = Configuration::GenericFactory<Spindle>;
}

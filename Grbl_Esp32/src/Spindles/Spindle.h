#pragma once

/*
    Spindle.h

    Header file for a Spindle Class
    See SpindleClass.cpp for more details

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

#define SPINDLE_STATE_DISABLE 0  // Must be zero.
#define SPINDLE_STATE_CW bit(0)
#define SPINDLE_STATE_CCW bit(1)

#define SPINDLE_TYPE_NONE 0
#define SPINDLE_TYPE_PWM 1
#define SPINDLE_TYPE_RELAY 2
#define SPINDLE_TYPE_LASER 3
#define SPINDLE_TYPE_DAC 4
#define SPINDLE_TYPE_HUANYANG 5
#define SPINDLE_TYPE_BESC 6
#define SPINDLE_TYPE_10V 7

#include "../Grbl.h"
#include <driver/dac.h>
#include <driver/uart.h>

// ===============  No floats! ===========================
// ================ NO FLOATS! ==========================

namespace Spindles {
    // This is the base class. Do not use this as your spindle
    class Spindle {
    public:
        Spindle() = default;

        Spindle(const Spindle&) = delete;
        Spindle(Spindle&&)      = delete;
        Spindle& operator=(const Spindle&) = delete;
        Spindle& operator=(Spindle&&) = delete;

        virtual void     init()                = 0;  // not in constructor because this also gets called when $$ settings change
        virtual uint32_t set_rpm(uint32_t rpm) = 0;
        virtual void     set_state(uint8_t state, uint32_t rpm) = 0;
        virtual uint8_t  get_state()                            = 0;
        virtual void     stop()                                 = 0;
        virtual void     config_message()                       = 0;
        virtual bool     isRateAdjusted();
        virtual void     sync(uint8_t state, uint32_t rpm);

        virtual ~Spindle() {}

        bool    is_reversable;
        bool    use_delays;  // will SpinUp and SpinDown delays be used.
        uint8_t _current_state;

        static void select();
    };

}

extern Spindles::Spindle* spindle;

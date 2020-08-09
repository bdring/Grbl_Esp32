#pragma once

/*
	HuanyangSpindle.h

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
    class HuanyangSpindle : public Spindle {
    private:
        uint16_t ModRTU_CRC(char* buf, int len);

        bool set_mode(uint8_t mode, bool critical);

        bool get_pins_and_settings();

        uint32_t _current_rpm;
        uint8_t  _txd_pin;
        uint8_t  _rxd_pin;
        uint8_t  _rts_pin;
        uint8_t  _state;
        bool     _task_running;

    public:
        HuanyangSpindle() : _task_running(false) {}

        HuanyangSpindle(const HuanyangSpindle&) = delete;
        HuanyangSpindle(HuanyangSpindle&&)      = delete;
        HuanyangSpindle& operator=(const HuanyangSpindle&) = delete;
        HuanyangSpindle& operator=(HuanyangSpindle&&) = delete;

        void        init();
        void        config_message();
        void        set_state(uint8_t state, uint32_t rpm);
        uint8_t     get_state();
        uint32_t    set_rpm(uint32_t rpm);
        void        stop();
        static void read_value(uint8_t reg);
        static void add_ModRTU_CRC(char* buf, int full_msg_len);

        virtual ~HuanyangSpindle() {}

    protected:
        uint32_t _min_rpm;
        uint32_t _max_rpm;
    };
}

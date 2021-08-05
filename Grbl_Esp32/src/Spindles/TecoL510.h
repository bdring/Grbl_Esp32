#pragma once

#include "VFDSpindle.h"

/*
    TecoL510.h

    Part of Grbl_ESP32
    2021 -  Jesse Schoch

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

namespace Spindles {
    class L510 : public VFD {
    protected:
        void direction_command(SpindleState mode, ModbusCommand& data) override;
        void set_speed_command(uint32_t rpm, ModbusCommand& data) override;

        response_parser initialization_sequence(int index, ModbusCommand& data) override;
        response_parser get_current_rpm(ModbusCommand& data) override;
        response_parser get_current_direction(ModbusCommand& data) override;

        // what is this, what should it do?
        bool            supports_actual_rpm() const override { return true; }
        bool            safety_polling() const override { return true; }
        response_parser get_status_ok(ModbusCommand& data) override;
        uint16_t        rpm_to_frequency(uint32_t rpm);
        uint32_t        freq_to_rpm(uint16_t);
        //uint32_t set_rpm(uint32_t rpm) override;
        void start_spindle();

    public:
        L510();
        uint16_t _max_freq;
    };
}

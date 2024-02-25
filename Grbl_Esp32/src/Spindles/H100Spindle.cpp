#include "H100Spindle.h"

/*
    H100Spindle.cpp

    This is for a Changrong Electric H100 VFD based spindle to be controlled via RS485 Modbus RTU.

    Part of Grbl_ESP32
    2024 -  Jaka Kordez

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

                         WARNING!!!!
    VFDs are very dangerous. They have high voltages and are very powerful
    Remove power before changing bits.

*/

namespace Spindles {
    H100::H100() : VFD() {}

    void H100::direction_command(SpindleState mode, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // data.msg[0] is omitted (modbus address is filled in later)
        data.msg[1] = 0x06; // 06: write single holding register
        data.msg[2] = 0x02; // 0x200: main control bit
        data.msg[3] = 0x00;

        switch (mode) {
            case SpindleState::Cw:
                data.msg[4] = 0x00;
                data.msg[5] = 0x03;
                break;
            case SpindleState::Ccw:
                data.msg[4] = 0x00;
                data.msg[5] = 0x05;
                break;
            default:                 // SpindleState::Disable
                data.msg[4] = 0x00;
                data.msg[5] = 0x08;
                break;
        }
    }

    void H100::set_speed_command(uint32_t rpm, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // We have to know the max RPM before we can set the current RPM:
        auto max_rpm       = this->_max_rpm;
        auto max_frequency = this->_maxFrequency;

        uint16_t freqFromRPM = (uint16_t(rpm) * uint16_t(max_frequency)) / uint16_t(max_rpm);

#ifdef VFD_DEBUG_MODE
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "For %d RPM the output frequency is set to %d Hz*10", int(rpm), int(freqFromRPM));
#endif

        data.msg[1] = 0x06; // 06: write single holding register
        data.msg[2] = 0x02; // 0x0201: given frequency
        data.msg[3] = 0x01;
        data.msg[4] = uint8_t(freqFromRPM >> 8);
        data.msg[5] = uint8_t(freqFromRPM & 0xFF);
    }

    VFD::response_parser H100::initialization_sequence(int index, ModbusCommand& data) {
        this->_min_rpm = 0;
        this->_max_rpm = 24000;

        return nullptr;
    }

    VFD::response_parser H100::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 04 0000 0001
        data.msg[1] = 0x04; // 0x04: read input register
        data.msg[2] = 0x00; // 0x0000: register address
        data.msg[3] = 0x00;
        data.msg[4] = 0x00; // 0x0001: read 1 byte
        data.msg[5] = 0x01;

        //  Recv: 01 04 02 xx xx
        //                 ---- = 1500
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t freq = (uint16_t(response[3]) << 8) | uint16_t(response[4]);

            auto h100 = static_cast<H100*>(vfd);

            uint16_t rpm = freq * uint16_t(vfd->_max_rpm) / uint16_t(h100->_maxFrequency);

            // Set current RPM value? Somewhere?
            vfd->_sync_rpm = rpm;
            return true;
        };
    }

    VFD::response_parser H100::get_current_direction(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 0210 0001
        data.msg[1] = 0x03; // 0x03: read holding register
        data.msg[2] = 0x02; // 0x0210: in forward/reverse rotation
        data.msg[3] = 0x10;
        data.msg[4] = 0x00; // 0x0001: read 1 byte
        data.msg[5] = 0x01;

        // TODO: What are we going to do with this? Update sys.spindle_speed? Update vfd state?
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool { 
            uint8_t status = response[4];

            if (status & 0x9) {
                // Operation: ON
                if (status & 0x2) {
                    // Direction: Reverse
                }
                else {
                    // Direction: Forward
                }
            }
            else {
                // Operation: OFF
            }

            return true; 
        };
    }
}

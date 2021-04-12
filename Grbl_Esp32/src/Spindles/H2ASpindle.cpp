#include "H2ASpindle.h"

/*
    H2ASpindle.cpp

    This is for the new H2A H2A VFD based spindle via RS485 Modbus.

    Part of Grbl_ESP32
    2020 -  Stefan de Bruijn

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

    The documentation is okay once you get how it works, but unfortunately
    incomplete... See H2ASpindle.md for the remainder of the docs that I
    managed to piece together.
*/

namespace Spindles {
    H2A::H2A() : VFD() {
        _baudrate = 19200;
        _parity   = Uart::Parity::Even;
    }

    void H2A::direction_command(SpindleState mode, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x20;  // Command ID 0x2000
        data.msg[3] = 0x00;
        data.msg[4] = 0x00;
        data.msg[5] = (mode == SpindleState::Ccw) ? 0x02 : (mode == SpindleState::Cw ? 0x01 : 0x06);
    }

    void H2A::set_speed_command(uint32_t rpm, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // We have to know the max RPM before we can set the current RPM:
        auto max_rpm = this->_max_rpm;

        // Speed is in [0..10'000] where 10'000 = 100%.
        // We have to use a 32-bit integer here; typical values are 10k/24k rpm.
        // I've never seen a 400K RPM spindle in my life, and they aren't supported
        // by this modbus protocol anyways... So I guess this is OK.
        uint16_t speed = (uint32_t(rpm) * 10000L) / uint32_t(max_rpm);
        if (speed < 0) {
            speed = 0;
        }
        if (speed > 10000) {
            speed = 10000;
        }

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x10;  // Command ID 0x1000
        data.msg[3] = 0x00;
        data.msg[4] = uint8_t(speed >> 8);  // RPM
        data.msg[5] = uint8_t(speed & 0xFF);
    }

    VFD::response_parser H2A::initialization_sequence(int index, ModbusCommand& data) {
        if (index == -1) {
            // NOTE: data length is excluding the CRC16 checksum.
            data.tx_length = 6;
            data.rx_length = 8;

            // Send: 01 03 B005 0002
            data.msg[1] = 0x03;  // READ
            data.msg[2] = 0xB0;  // B0.05 = Get RPM
            data.msg[3] = 0x05;
            data.msg[4] = 0x00;  // Read 2 values
            data.msg[5] = 0x02;

            //  Recv: 01 03 00 04 5D C0 03 F6
            //                    -- -- = 24000 (val #1)
            return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
                uint16_t rpm  = (uint16_t(response[4]) << 8) | uint16_t(response[5]);
                vfd->_max_rpm = rpm;

                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "H2A spindle is initialized at %d RPM", int(rpm));

                return true;
            };
        } else {
            return nullptr;
        }
    }

    VFD::response_parser H2A::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 8;

        // Send: 01 03 700C 0002
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x70;  // B0.05 = Get RPM
        data.msg[3] = 0x0C;
        data.msg[4] = 0x00;  // Read 2 values
        data.msg[5] = 0x02;

        //  Recv: 01 03 0004 095D 0000
        //                   ---- = 2397 (val #1)
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t rpm = (uint16_t(response[4]) << 8) | uint16_t(response[5]);

            // Set current RPM value? Somewhere?
            vfd->_sync_rpm = rpm;
            return true;
        };
    }

    VFD::response_parser H2A::get_current_direction(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // Send: 01 03 30 00 00 01
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x30;  // Command group ID
        data.msg[3] = 0x00;
        data.msg[4] = 0x00;  // Message ID
        data.msg[5] = 0x01;

        // Receive: 01 03 00 02 00 02
        //                      ----- status

        // TODO: What are we going to do with this? Update sys.spindle_speed? Update vfd state?
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool { return true; };
    }
}

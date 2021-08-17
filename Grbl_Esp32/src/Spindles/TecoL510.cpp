#include "TecoL510.h"

/*
    TecoL510.cpp

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

                         WARNING!!!!
    VFDs are very dangerous. They have high voltages and are very powerful
    Remove power before changing bits.

    See TecoL510.md for details
*/

namespace Spindles {
    L510::L510() : VFD() {
        _baudrate = 9600;
        _parity   = Uart::Parity::None;
        // TODO:  should defaults be set here?  What happens if the motor settings in the VFD are wrong or default?
        // I think they are overloaded with DEFAULT_SPINDLE_RPM_MAX and DEFAULT_SPINDLE_RPM_MIN
        _max_rpm  = 24000;
        _min_rpm  = 6000;
        _max_freq = 40000;
    }

    void L510::direction_command(SpindleState mode, ModbusCommand& data) {
        //  Note: The direction command is always called on M3,M4, and M5
        //      This is where the spindle start/stop should be sent
        

        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x25;  // Command ID 0x2501
        data.msg[3] = 0x01;
        data.msg[4] = 0x00;
        switch (mode) {
            case SpindleState::Disable:
                //data.msg[4] = 0x00;
                data.msg[5] = 0x00;
                break;
            case SpindleState::Cw:
                data.msg[5] = 0x01;
                break;
            case SpindleState::Ccw:
                data.msg[5] = 0x03;
                break;
        }
    }

    void L510::set_speed_command(uint32_t rpm, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        uint16_t freq = rpm_to_frequency(rpm);

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x25;  // Command ID 0x2502
        data.msg[3] = 0x02;
        data.msg[4] = uint8_t(freq >> 8);  // RPM
        data.msg[5] = uint8_t(freq & 0xFF);

#ifdef VFD_DEBUG_MODE2
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "setting speed to: %d", speed);
#endif
    }
    uint16_t L510::rpm_to_frequency(uint32_t rpm) {
        auto     max_rpm  = this->_max_rpm;
        auto     max_freq = this->_max_freq;
        uint16_t freq     = (uint32_t(rpm) * max_freq) / uint32_t(max_rpm);
        if (freq < 0) {
            freq = 0;
        }
        if (freq > max_freq) {
            freq = max_freq;
        }
        return freq;
    }

    uint32_t L510::freq_to_rpm(uint16_t freq) {
        auto     max_rpm  = this->_max_rpm;
        auto     max_freq = this->_max_freq;
        uint32_t rpm      = (freq * max_rpm) / max_freq;
        if (rpm < 0) {
            // sometimes it returns -1 which causes an alarm
            rpm = 0;
        }
        return rpm;
    }

    VFD::response_parser L510::initialization_sequence(int index, ModbusCommand& data) {
        if (index == -1) {
            // NOTE: data length is excluding the CRC16 checksum.
            data.tx_length = 6;
            data.rx_length = 11;

            // read parameters 02-03..02-06

            // Send:
            data.msg[1] = 0x03;  // READ
            data.msg[2] = 0x02;  // 0x0203 = Get  max rpm
            data.msg[3] = 0x03;
            data.msg[4] = 0x00;  // Read 4 values
            data.msg[5] = 0x04;

            //  Recv: ??

            return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
                uint32_t rpm = (response[3] << 8) | response[4];

                // NOTE:  the frequency is stored xxx.x but the input command is frequency * 100;
                uint16_t freq = ((uint16_t)response[9] << 8) | (uint16_t)response[10];
                freq          = freq * 10;
                auto l510     = static_cast<L510*>(vfd);

                l510->_max_rpm  = rpm;
                l510->_max_freq = freq;

                grbl_msg_sendf(
                    CLIENT_SERIAL, MsgLevel::Info, "L510 initialized: spindle max_rpm %d max_freq %d", vfd->_max_rpm, l510->_max_freq);

                return true;
            };
        } else {
            return nullptr;
        }
    }
    VFD::response_parser L510::get_status_ok(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send:
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x25;  // 0x2520 = Get state
        data.msg[3] = 0x20;
        data.msg[4] = 0x00;  // Read 1 value
        data.msg[5] = 0x01;

        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t vfd_state = ((uint16_t)response[3] << 8) | (uint16_t)response[4];

            if (bitRead(vfd_state, 3)) {
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "L510 Fault detected");
                return false;
            }
            return true;
        };
    }

    VFD::response_parser L510::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 700C 0002
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x25;  // 0x2524 = Get output frequency
        data.msg[3] = 0x24;
        data.msg[4] = 0x00;  // Read 1 value
        data.msg[5] = 0x01;

        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t freq = ((uint16_t)response[3] << 8) | (uint16_t)response[4];

            auto l510 = static_cast<L510*>(vfd);

            vfd->_sync_rpm = l510->freq_to_rpm(freq);
            return true;
        };
    }

    VFD::response_parser L510::get_current_direction(ModbusCommand& data) {
        // does this run ever??
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 30 00 00 01
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x25;  // 0x2520
        data.msg[3] = 0x20;
        data.msg[4] = 0x00;  // Message ID
        data.msg[5] = 0x01;

        // Receive: 01 03 00 02 00 02
        //                      ----- status

        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t got = (uint16_t(response[3]) << 8) | uint16_t(response[4]);
            bool     dir = bitRead(got, 1);
            return true;
        };
    }
}

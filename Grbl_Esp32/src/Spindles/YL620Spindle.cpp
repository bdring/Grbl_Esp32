#include "YL620Spindle.h"

/*
    YL620Spindle.cpp

    This is for a Yalang YL620/YL620-A VFD based spindle to be controlled via RS485 Modbus RTU.

    Part of Grbl_ESP32
    2021 -  Marco Wagner

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

    =============================================================================================================

    Configuration required for the YL620

    Parameter number        Description                     Value
    -------------------------------------------------------------------------------
    P00.00                  Main frequency                  400.00Hz (match to your spindle)
    P00.01                  Command source                  3
    
    P03.00                  RS485 Baud rate                 3 (9600)
    P03.01                  RS485 address                   1
    P03.02                  RS485 protocol                  2
    P03.08                  Frequency given lower limit     100.0Hz (match to your spindle cooling-type)

    ===============================================================================================================

    RS485 communication is standard Modbus RTU

    Therefore, the following operation codes are relevant:
    0x03:   read single holding register
    0x06:   write single holding register

    Holding register address                Description
    ---------------------------------------------------------------------------
    0x0000                                  main frequency
    0x0308                                  frequency given lower limit

    0x2000                                  command register (further information below)
    0x2001                                  Modbus485 frequency command (x0.1Hz => 2500 = 250.0Hz)

    0x200A                                  Target frequency
    0x200B                                  Output frequency
    0x200C                                  Output current


    Command register at holding address 0x2000
    --------------------------------------------------------------------------
    bit 1:0             b00: No function
                        b01: shutdown command
                        b10: start command
                        b11: Jog command
    bit 3:2             reserved
    bit 5:4             b00: No function
                        b01: Forward command
                        b10: Reverse command
                        b11: change direction
    bit 7:6             b00: No function
                        b01: reset an error flag
                        b10: reset all error flags
                        b11: reserved
*/

namespace Spindles {
    YL620::YL620() : VFD() {}

    void YL620::direction_command(SpindleState mode, ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // data.msg[0] is omitted (modbus address is filled in later)
        data.msg[1] = 0x06;  // 06: write output register
        data.msg[2] = 0x20;  // 0x2000: command register address
        data.msg[3] = 0x00;

        data.msg[4] = 0x00;  // High-Byte of command always 0x00
        switch (mode) {
            case SpindleState::Cw:
                data.msg[5] = 0x12;  // Start in forward direction
                break;
            case SpindleState::Ccw:
                data.msg[5] = 0x22;  // Start in reverse direction
                break;
            default:                 // SpindleState::Disable
                data.msg[5] = 0x01;  // Disable spindle
                break;
        }
    }

    void YL620::set_speed_command(uint32_t rpm, ModbusCommand& data) {
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

        data.msg[1] = 0x06;
        data.msg[2] = 0x20;
        data.msg[3] = 0x01;
        data.msg[4] = uint8_t(freqFromRPM >> 8);
        data.msg[5] = uint8_t(freqFromRPM & 0xFF);
    }

    VFD::response_parser YL620::initialization_sequence(int index, ModbusCommand& data) {
        if (index == -1) {
            // NOTE: data length is excluding the CRC16 checksum.
            data.tx_length = 6;
            data.rx_length = 5;

            data.msg[1] = 0x03;
            data.msg[2] = 0x03;
            data.msg[3] = 0x08;
            data.msg[4] = 0x00;
            data.msg[5] = 0x01;

            //  Recv: 01 03 02 03 E8 xx xx
            //                 -- -- = 1000
            return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
                auto yl620           = static_cast<YL620*>(vfd);
                yl620->_minFrequency = (uint16_t(response[3]) << 8) | uint16_t(response[4]);

#ifdef VFD_DEBUG_MODE
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "YL620 allows minimum frequency of %d Hz", int(yl620->_minFrequency));
#endif

                return true;
            };
        } else if (index == -2) {
            // NOTE: data length is excluding the CRC16 checksum.
            data.tx_length = 6;
            data.rx_length = 5;

            data.msg[1] = 0x03;
            data.msg[2] = 0x00;
            data.msg[3] = 0x00;
            data.msg[4] = 0x00;
            data.msg[5] = 0x01;

            //  Recv: 01 03 02 0F A0 xx xx
            //                 -- -- = 4000
            return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
                auto yl620           = static_cast<YL620*>(vfd);
                yl620->_maxFrequency = (uint16_t(response[3]) << 8) | uint16_t(response[4]);

                vfd->_min_rpm = uint32_t(yl620->_minFrequency) * uint32_t(vfd->_max_rpm) /
                                uint32_t(yl620->_maxFrequency);  //   1000 * 24000 / 4000 =   6000 RPM.

#ifdef VFD_DEBUG_MODE
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "YL620 allows maximum frequency of %d Hz", int(yl620->_maxFrequency));
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "Configured maxRPM of %d RPM results in minRPM of %d RPM",
                               int(vfd->_max_rpm),
                               int(vfd->_min_rpm));
#endif

                return true;
            };
        } else {
            return nullptr;
        }
    }

    VFD::response_parser YL620::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 200B 0001
        data.msg[1] = 0x03;
        data.msg[2] = 0x20;
        data.msg[3] = 0x0B;
        data.msg[4] = 0x00;
        data.msg[5] = 0x01;

        //  Recv: 01 03 02 05 DC xx xx
        //                 ---- = 1500
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t freq = (uint16_t(response[3]) << 8) | uint16_t(response[4]);

            auto yl620 = static_cast<YL620*>(vfd);

            uint16_t rpm = freq * uint16_t(vfd->_max_rpm) / uint16_t(yl620->_maxFrequency);

            // Set current RPM value? Somewhere?
            vfd->_sync_rpm = rpm;
            return true;
        };
    }

    VFD::response_parser YL620::get_current_direction(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 20 00 00 01
        data.msg[1] = 0x03;
        data.msg[2] = 0x20;
        data.msg[3] = 0x00;
        data.msg[4] = 0x00;
        data.msg[5] = 0x01;

        // Receive: 01 03 02 00 0A xx xx
        //                   ----- status is in 00 0A bit 5:4

        // TODO: What are we going to do with this? Update sys.spindle_speed? Update vfd state?
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool { return true; };
    }
}

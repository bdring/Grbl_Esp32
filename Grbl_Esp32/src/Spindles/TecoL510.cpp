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
        _max_rpm = 24000;
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
        switch(mode){
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
        grbl_msg_sendf(CLIENT_SERIAL,MsgLevel::Info,"sending dir: state:  %d",mode);
    }

    /*
    void L510::start_command(ModbusCommand& data){
        data.tx_length = 6;
        data.rx_length = 6;
        data.msg[1] = 0x06;
        data.msg[2] = 0x25;
        data.msg[3] = 0x01;
        data.msg[4] = 0x00;
        data.msg[5] = 0x01;

    }
    */

    void L510::set_speed_command(uint32_t rpm, ModbusCommand& data) {

                // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // We have to know the max RPM before we can set the current RPM:
        auto max_rpm = this->_max_rpm;
        auto max_freq = this->_max_freq;

        // Assuming max frequncy is 400Hz
        // Speed is in [0..40,000] 
        uint16_t speed = (uint16_t(rpm) * max_freq) / uint32_t(max_rpm);
        if (speed < 0) {
            speed = 0;
        }
        if (speed > 40000) {
            speed = 40000;
        }

        data.msg[1] = 0x06;  // WRITE
        data.msg[2] = 0x25;  // Command ID 0x2502
        data.msg[3] = 0x02;
        data.msg[4] = uint8_t(speed >> 8);  // RPM
        data.msg[5] = uint8_t(speed & 0xFF);

        grbl_msg_sendf(CLIENT_SERIAL,MsgLevel::Info,"setting speed to: %d",speed);
    }
    uint16_t L510::rpm_to_frequency(uint32_t rpm){
        auto max_rpm = this->_max_rpm;
        uint16_t freq = (uint32_t(rpm) * 40000L) / uint32_t(max_rpm);
        return freq;
    }

    uint32_t L510::freq_to_rpm(uint16_t freq){
        auto max_rpm = this->_max_rpm;
        auto max_freq = this->_max_freq;
        uint32_t rpm = (freq*max_rpm)/max_freq;
    }

    VFD::response_parser L510::initialization_sequence(int index, ModbusCommand& data) {
        if (index == -1) {
            // NOTE: data length is excluding the CRC16 checksum.
            data.tx_length = 6;
            data.rx_length = 5;

            
            // Send: 
            data.msg[1] = 0x03;  // READ
            data.msg[2] = 0x02;  // 0x0203 = Get  max rpm
            data.msg[3] = 0x03;
            data.msg[4] = 0x00;  // Read 1 value
            data.msg[5] = 0x01;
            


            //  Recv: ??

            return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
                uint32_t rpm  = (response[3] << 8) | response[4];
                //TODO remove hardcoding
                vfd->_max_rpm = rpm;
                //vfd->_max_rpm = 24000;

                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "L510 spindle hardcoded 24000 %d ", vfd->_max_rpm);

                return true;
            };
        } else {
            return nullptr;
        }
    }

    VFD::response_parser L510::get_current_rpm(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 5;

        // Send: 01 03 700C 0002
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x25;  // 0x2523 = Get RPM
        data.msg[3] = 0x23;
        data.msg[4] = 0x00;  // Read 1 value
        data.msg[5] = 0x01;

        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool {
            uint16_t freq = (response[3] << 8) | response[4];
            grbl_msg_sendf(CLIENT_SERIAL,MsgLevel::Info,"current frequency: %d",freq);

            auto l510 = static_cast<L510*>(vfd);

            vfd->_sync_rpm = l510->freq_to_rpm(freq);
            return true;
        };
    }

    VFD::response_parser L510::get_current_direction(ModbusCommand& data) {
        // NOTE: data length is excluding the CRC16 checksum.
        data.tx_length = 6;
        data.rx_length = 6;

        // Send: 01 03 30 00 00 01
        data.msg[1] = 0x03;  // READ
        data.msg[2] = 0x25;  // 0x2520
        data.msg[3] = 0x20;
        data.msg[4] = 0x00;  // Message ID
        data.msg[5] = 0x01;

        // Receive: 01 03 00 02 00 02
        //                      ----- status

        // TODO: this doesn't seem to do anything in H2A
        return [](const uint8_t* response, Spindles::VFD* vfd) -> bool { 
            uint16_t got = (uint16_t(response[4]) << 8) | uint16_t(response[5]);
            bool dir = bitRead(got,1);
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "L510 dir %d", int(dir));
            return true; 
            };
    }
    /*
    void start_spindle(){
        if(!spindle_started){
            // send start to VFD
           ModbusCommand start_cmd;
           start_cmd[0] = VFD_RS485_ADDR;
           start_command(start_cmd);
           if (xQueueSend(vfd_cmd_queue, &start_cmd, 0) != pdTRUE) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "VFD Queue Full");
           }

        }
    }
    */

    /*
    uint32_t LF510::set_rpm(uint32_t rpm){
        if (!vfd_ok) {
            return 0;
        }
        // Hack to start spindle running
        SpindleState sstate = get_state();
        if(sstate != SpindleState::Disabled){
            start_spindle();
        }
        // apply override
        rpm = rpm * sys.spindle_speed_ovr / 100;  // Scale by spindle speed override value (uint8_t percent)

        if (rpm != 0 && (rpm < _min_rpm || rpm > _max_rpm)) {
            // NOTE: Don't add a info message here; this method is called from the stepper_pulse_func ISR method, so
            // emitting debug information could crash the ESP32.

            rpm = constrain(rpm, _min_rpm, _max_rpm);
        }

        // apply limits
        // if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        //     rpm = _max_rpm;
        // } else if (rpm != 0 && rpm <= _min_rpm) {
        //     rpm = _min_rpm;
        // }

        sys.spindle_speed = rpm;

        if (rpm == _current_rpm) {  // prevent setting same RPM twice
            return rpm;
        }

#ifdef VFD_DEBUG_MODE2
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Setting spindle speed to %d rpm (%d, %d)", int(rpm), int(_min_rpm), int(_max_rpm));
#endif

        _current_rpm = rpm;

        // TODO add the speed modifiers override, linearization, etc.

        ModbusCommand rpm_cmd;
        rpm_cmd.msg[0] = VFD_RS485_ADDR;

        set_speed_command(rpm, rpm_cmd);

        // Sometimes sync_rpm is retained between different set_speed_command's. We don't want that - we want 
        // spindle sync to kick in after we set the speed. This forces that.
        _sync_rpm = UINT32_MAX;

        rpm_cmd.critical = (rpm == 0);

        if (xQueueSend(vfd_cmd_queue, &rpm_cmd, 0) != pdTRUE) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "VFD Queue Full");
        }

        return rpm;
    }
    */
}
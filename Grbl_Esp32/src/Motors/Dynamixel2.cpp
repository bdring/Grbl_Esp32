/*
    Dynamixel2.cpp

    This allows an Dynamixel sero to be used like any other motor. Servos
    do have limitation in travel and speed, so you do need to respect that.

    Protocol 2

    Part of Grbl_ESP32

    2020 -	Bart Dring

    https://emanual.robotis.com/docs/en/dxl/protocol2/

    Grbl_ESP32 is free software: you can redistribute it and/or modify
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

#include "Dynamixel2.h"

namespace Motors {
    bool    Motors::Dynamixel2::uart_ready         = false;
    uint8_t Motors::Dynamixel2::ids[MAX_N_AXIS][2] = { { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } };

    Dynamixel2::Dynamixel2(uint8_t axis_index, uint8_t id, uint8_t tx_pin, uint8_t rx_pin, uint8_t rts_pin) :
        Servo(axis_index), _id(id), _tx_pin(tx_pin), _rx_pin(rx_pin), _rts_pin(rts_pin) {
        if (_tx_pin == UNDEFINED_PIN || _rx_pin == UNDEFINED_PIN || _rts_pin == UNDEFINED_PIN) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Error. Missing pin definitions");
            _has_errors = true;
        } else {
            _has_errors = false;   // The motor can be used
        }
    }

    void Dynamixel2::init() {
        init_uart(_id, _axis_index, _dual_axis_index);  // static and only allows one init

        read_settings();

        config_message();  // print the config

        if (!test()) {  // ping the motor
            _has_errors = true;
            return;
        }

        set_disable(true);                              // turn off torque so we can set EEPROM registers
        set_operating_mode(DXL_CONTROL_MODE_POSITION);  // set it in the right control mode

        // servos will blink in axis order for reference
        LED_on(true);
        vTaskDelay(100);
        LED_on(false);

        startUpdateTask();
    }

    void Dynamixel2::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Dynamixel Servo ID:%d Count(%5.0f,%5.0f) %s",
                       reportAxisNameMsg(_axis_index, _dual_axis_index),
                       _id,
                       _dxl_count_min,
                       _dxl_count_max,
                       reportAxisLimitsMsg(_axis_index));
    }

    bool Dynamixel2::test() {
        uint16_t len = 3;

        _dxl_tx_message[DXL_MSG_INSTR] = DXL_INSTR_PING;

        dxl_finish_message(_id, _dxl_tx_message, len);

        len = dxl_get_response(PING_RSP_LEN);  // wait for and get response

        if (len == PING_RSP_LEN) {
            uint16_t model_num = _dxl_rx_message[10] << 8 | _dxl_rx_message[9];
            if (model_num == 1060) {
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "%s Dynamixel Detected ID %d Model XL430-W250 F/W Rev %x",
                               reportAxisNameMsg(_axis_index, _dual_axis_index),
                               _id,
                               _dxl_rx_message[11]);
            } else {
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "%s Dynamixel Detected ID %d M/N %d F/W Rev %x",
                               reportAxisNameMsg(_axis_index, _dual_axis_index),
                               _id,
                               model_num,
                               _dxl_rx_message[11]);
            }

        } else {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "%s Dynamixel Servo ID %d Ping failed", reportAxisNameMsg(_axis_index, _dual_axis_index),
_id);
            return false;
        }

        return true;
    }

    void Dynamixel2::read_settings() {
        _dxl_count_min = DXL_COUNT_MIN;
        _dxl_count_max = DXL_COUNT_MAX;

        if (bitnum_istrue(dir_invert_mask->get(), _axis_index))  // normal direction
            swap(_dxl_count_min, _dxl_count_min);
    }

    // sets the PWM to zero. This allows most servos to be manually moved
    void Dynamixel2::set_disable(bool disable) {
        uint8_t param_count = 1;

        if (_disabled == disable)
            return;

        _disabled = disable;

        dxl_write(DXL_ADDR_TORQUE_EN, param_count, !_disabled);
    }

    void Dynamixel2::set_operating_mode(uint8_t mode) {
        uint8_t param_count = 1;
        dxl_write(DXL_OPERATING_MODE, param_count, mode);
    }

    void Dynamixel2::update() {
        if (_has_errors) {
            return;
        }

        if (_disabled) {
            dxl_read_position();
        } else {
            dxl_bulk_goal_position();  // call the static method that updates all at once
        }
    }

    /*
        Static

        This will be called by each axis, but only the first call will setup the serial port.
        It will store the IDs and Axes in an array for later group processing

    */
    void Dynamixel2::init_uart(uint8_t id, uint8_t axis_index, uint8_t dual_axis_index) {
        ids[axis_index][dual_axis_index] = id;  // learn all the ids

        if (uart_ready)
            return;  // UART already setup

        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel UART TX:%d RX:%d RTS:%d", DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS);

        uart_driver_delete(UART_NUM_2);

        // setup the comm port as half duplex
        uart_config_t uart_config = {
            .baud_rate           = DYNAMIXEL_BAUD_RATE,
            .data_bits           = UART_DATA_8_BITS,
            .parity              = UART_PARITY_DISABLE,
            .stop_bits           = UART_STOP_BITS_1,
            .flow_ctrl           = UART_HW_FLOWCTRL_DISABLE,
            .rx_flow_ctrl_thresh = 122,
        };

        // Configure UART parameters
        uart_param_config(UART_NUM_2, &uart_config);
        uart_set_pin(UART_NUM_2, DYNAMIXEL_TXD, DYNAMIXEL_RXD, DYNAMIXEL_RTS, UART_PIN_NO_CHANGE);
        uart_driver_install(UART_NUM_2, DYNAMIXEL_BUF_SIZE * 2, 0, 0, NULL, 0);
        uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX);

        uart_ready = true;
    }

    void Dynamixel2::set_location() {}

    // This motor will not do a standard home to a limit switch (maybe future)
    // If it is in the homing mask it will a quick move to $<axis>/Home/Mpos
    bool Dynamixel2::set_homing_mode(bool isHoming) {
        if (_has_errors) {
            return false;
        }
        sys_position[_axis_index] =
            axis_settings[_axis_index]->home_mpos->get() * axis_settings[_axis_index]->steps_per_mm->get();  // convert to steps

        set_disable(false);
        set_location();  // force the PWM to update now
        return false;  // Cannot do conventional homing
    }

    void Dynamixel2::dxl_goal_position(int32_t position) {
        uint8_t param_count = 4;

        dxl_write(DXL_GOAL_POSITION,
                  param_count,
                  (position & 0xFF),
                  (position & 0xFF00) >> 8,
                  (position & 0xFF0000) >> 16,
                  (position & 0xFF000000) >> 24);
    }

    uint32_t Dynamixel2::dxl_read_position() {
        uint8_t data_len = 4;

        dxl_read(DXL_PRESENT_POSITION, data_len);

        data_len = dxl_get_response(15);

        if (data_len == 15) {
            uint32_t dxl_position = _dxl_rx_message[9] | (_dxl_rx_message[10] << 8) | (_dxl_rx_message[11] << 16) |
                                    (_dxl_rx_message[12] << 24);

            read_settings();

            int32_t pos_min_steps = lround(limitsMinPosition(_axis_index) * axis_settings[_axis_index]->steps_per_mm->get());
            int32_t pos_max_steps = lround(limitsMaxPosition(_axis_index) * axis_settings[_axis_index]->steps_per_mm->get());

            int32_t temp = map(dxl_position, DXL_COUNT_MIN, DXL_COUNT_MAX, pos_min_steps, pos_max_steps);

            sys_position[_axis_index] = temp;

            plan_sync_position();

            return dxl_position;
        } else {
            return 0;
        }
    }

    void Dynamixel2::dxl_read(uint16_t address, uint16_t data_len) {
        uint8_t msg_len = 3 + 4;

        _dxl_tx_message[DXL_MSG_INSTR]     = DXL_READ;
        _dxl_tx_message[DXL_MSG_START]     = (address & 0xFF);            // low-order address value
        _dxl_tx_message[DXL_MSG_START + 1] = ((address & 0xFF00) >> 8);   // High-order address value
        _dxl_tx_message[DXL_MSG_START + 2] = (data_len & 0xFF);           // low-order data length value
        _dxl_tx_message[DXL_MSG_START + 3] = ((data_len & 0xFF00) >> 8);  // high-order address value

        dxl_finish_message(_id, _dxl_tx_message, msg_len);
    }

    void Dynamixel2::LED_on(bool on) {
        uint8_t param_count = 1;

        if (on)
            dxl_write(DXL_ADDR_LED_ON, param_count, 1);
        else
            dxl_write(DXL_ADDR_LED_ON, param_count, 0);
    }

    // wait for and get the servo response
    uint16_t Dynamixel2::dxl_get_response(uint16_t length) {
        length = uart_read_bytes(UART_NUM_2, _dxl_rx_message, length, DXL_RESPONSE_WAIT_TICKS);
        return length;
    }

    void Dynamixel2::dxl_write(uint16_t address, uint8_t paramCount, ...) {
        _dxl_tx_message[DXL_MSG_INSTR]     = DXL_WRITE;
        _dxl_tx_message[DXL_MSG_START]     = (address & 0xFF);           // low-order address value
        _dxl_tx_message[DXL_MSG_START + 1] = ((address & 0xFF00) >> 8);  // High-order address value

        uint8_t msg_offset = 1;  // this is the offset from DXL_MSG_START in the message

        va_list valist;

        /* Initializing arguments  */
        va_start(valist, paramCount);

        for (int x = 0; x < paramCount; x++) {
            msg_offset++;
            _dxl_tx_message[DXL_MSG_START + msg_offset] = (uint8_t)va_arg(valist, int);
        }
        va_end(valist);  // Cleans up the list

        dxl_finish_message(_id, _dxl_tx_message, msg_offset + 4);

        uint16_t len = 11;  // response length
        len          = dxl_get_response(len);

        if (len == 11) {
            uint8_t err = _dxl_rx_message[8];
            switch (err) {
                case 1:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write fail error", _id);
                    break;
                case 2:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write instruction error", _id);
                    break;
                case 3:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write access error", _id);
                    break;
                case 4:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write data range error", _id);
                    break;
                case 5:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write data length error", _id);
                    break;
                case 6:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write data limit error", _id);
                    break;
                case 7:
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Write access error", _id);
                    break;
                default:
                    break;
            }
        } else {
            // timeout
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Dynamixel Servo ID %d Timeout", _id);
        }
    }

    /*
        Static

        This will sync all the motors in one command
        It looks for IDs in the array of axes

    */
    void Dynamixel2::dxl_bulk_goal_position() {
        char  tx_message[100];  // outgoing to dynamixel
        float position_min, position_max;
        float dxl_count_min, dxl_count_max;

        uint16_t msg_index = DXL_MSG_INSTR;  // index of the byte in the message we are currently filling
        uint32_t dxl_position;
        uint8_t  count = 0;
        uint8_t  current_id;

        tx_message[msg_index]   = DXL_SYNC_WRITE;
        tx_message[++msg_index] = DXL_GOAL_POSITION & 0xFF;           // low order address
        tx_message[++msg_index] = (DXL_GOAL_POSITION & 0xFF00) >> 8;  // high order address
        tx_message[++msg_index] = 4;                                  // low order data length
        tx_message[++msg_index] = 0;                                  // high order data length

        auto n_axis = number_axis->get();
        for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
            for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
                current_id = ids[axis][gang_index];
                if (current_id != 0) {
                    count++;  // keep track of the count for the message length

                    //determine the location of the axis
                    float target = system_convert_axis_steps_to_mpos(sys_position, axis);  // get the axis machine position in mm

                    dxl_count_min = DXL_COUNT_MIN;
                    dxl_count_max = DXL_COUNT_MAX;

                    if (bitnum_istrue(dir_invert_mask->get(), axis))  // normal direction
                        swap(dxl_count_min, dxl_count_max);

                    // map the mm range to the servo range
                    dxl_position = (uint32_t)mapConstrain(target, limitsMinPosition(axis), limitsMaxPosition(axis), dxl_count_min, dxl_count_max);

                    tx_message[++msg_index] = current_id;                         // ID of the servo
                    tx_message[++msg_index] = dxl_position & 0xFF;                // data
                    tx_message[++msg_index] = (dxl_position & 0xFF00) >> 8;       // data
                    tx_message[++msg_index] = (dxl_position & 0xFF0000) >> 16;    // data
                    tx_message[++msg_index] = (dxl_position & 0xFF000000) >> 24;  // data
                }
            }
        }
        dxl_finish_message(DXL_BROADCAST_ID, tx_message, (count * 5) + 7);
    }

    /*
    Static

    This is a helper function to complete and send the message
    The body of the message should be in msg, at the correct location
    before calling this function.
    This function will add the header, length bytes and CRC
    It will then send the message
*/
    void Dynamixel2::dxl_finish_message(uint8_t id, char* msg, uint16_t msg_len) {
        //uint16_t msg_len;
        uint16_t crc = 0;
        // header
        msg[DXL_MSG_HDR1] = 0xFF;
        msg[DXL_MSG_HDR2] = 0xFF;
        msg[DXL_MSG_HDR3] = 0xFD;
        //
        // reserved
        msg[DXL_MSG_RSRV] = 0x00;
        msg[DXL_MSG_ID]   = id;
        // length
        msg[DXL_MSG_LEN_L] = msg_len & 0xFF;
        msg[DXL_MSG_LEN_H] = (msg_len & 0xFF00) >> 8;

        // the message should already be here

        crc = dxl_update_crc(crc, msg, 5 + msg_len);

        msg[msg_len + 5] = crc & 0xFF;  // CRC_L
        msg[msg_len + 6] = (crc & 0xFF00) >> 8;

        uart_flush(UART_NUM_2);
        uart_write_bytes(UART_NUM_2, msg, msg_len + 7);
    }

    // from http://emanual.robotis.com/docs/en/dxl/crc/
    uint16_t Dynamixel2::dxl_update_crc(uint16_t crc_accum, char* data_blk_ptr, uint8_t data_blk_size) {
        uint16_t i, j;
        uint16_t crc_table[256] = {
            0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011, 0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
            0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
            0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
            0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
            0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
            0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
            0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
            0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132, 0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
            0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
            0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
            0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
            0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
            0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
            0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
            0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252, 0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
            0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
        };

        for (j = 0; j < data_blk_size; j++) {
            i         = ((uint16_t)(crc_accum >> 8) ^ data_blk_ptr[j]) & 0xFF;
            crc_accum = (crc_accum << 8) ^ crc_table[i];
        }

        return crc_accum;
    }
}

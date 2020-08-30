/*
    Dynamixel2.cpp

    This allows an Dynamixel sero to be used like any other motor. Servos
    do have limitation in travel and speed, so you do need to respect that.

    Part of Grbl_ESP32

    2020 -	Bart Dring

    Servos have a limited travel, so they map the travel across a range in
    the current work coordinatee system. The servo can only travel as far
    as the range, but the internal axis value can keep going.

    Range: The range is specified in the machine definition file with...
    #define X_SERVO_RANGE_MIN       0.0
    #define X_SERVO_RANGE_MAX       5.0

    Direction: The direction can be changed using the $3 setting for the axis

    Homing: During homing, the servo will move to one of the endpoints. The
    endpoint is determined by the $23 or $HomingDirInvertMask setting for the axis.
    Do not define a homing cycle for the axis with the servo.
    You do need at least 1 homing cycle.  TODO: Fix this

    Calibration. You can tweak the endpoints using the $10n or nStepsPerMm and
    $13n or $xMaxTravel setting, where n is the axis.
    The value is a percent. If you secify a percent outside the
    the range specified by the values below, it will be reset to 100.0 (100% ... no change)
    The calibration adjusts in direction of positive momement, so a value above 100% moves
    towards the higher axis value.

    #define SERVO_CAL_MIN
    #define SERVO_CAL_MAX

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
    Dynamixel2::Dynamixel2() {}

    Dynamixel2::Dynamixel2(uint8_t axis_index, uint8_t id, uint8_t tx_pin, uint8_t rx_pin, uint8_t rts_pin, float min, float max) {
        type_id               = DYNAMIXEL2;
        this->axis_index      = axis_index % MAX_AXES;
        this->dual_axis_index = axis_index < MAX_AXES ? 0 : 1;  // 0 = primary 1 = ganged
        _position_min         = min;
        _position_max         = max;
        _id                   = id;
        _tx_pin               = tx_pin;
        _rx_pin               = rx_pin;
        _rts_pin              = rts_pin;
        init();
    }

    void Dynamixel2::init() {
        read_settings();
        is_active = true;  // as opposed to NullMotors, this is a real motor
        set_axis_name();

        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Dynamixel UART TX:%d RX:%d RTS:%d", _tx_pin, _rx_pin, _rts_pin);

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
        uart_set_pin(UART_NUM_2, _tx_pin, _rx_pin, _rts_pin, UART_PIN_NO_CHANGE);
        uart_driver_install(UART_NUM_2, DYNAMIXEL_BUF_SIZE * 2, 0, 0, NULL, 0);
        uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX);
        config_message();
        test();
    }

    void Dynamixel2::config_message() {
        grbl_msg_sendf(
            CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Axis Dynamixel Servo ID:%d Min:%5.3fmm Max:%5.3fmm", _axis_name, _id, _position_min, _position_max);
    }

    bool Dynamixel2::test() {
        uint16_t len = 3;

        dxl_tx_message[DXL_MSG_INSTR] = DXL_INSTR_PING;

        dxl_finish_message(dxl_tx_message, _id, len);

        len = dxl_get_response(PING_RSP_LEN);  // wait for and get response

        if (len == PING_RSP_LEN) {
            uint16_t model_num = dxl_rx_message[10] << 8 | dxl_rx_message[9];
            if (model_num == 1060) {
                grbl_msg_sendf(CLIENT_SERIAL,
                               MSG_LEVEL_INFO,
                               "%s Axis Dynamixel Detected ID %d Model XL430-W250 F/W Rev %x",
                               _axis_name,
                               _id,
                               dxl_rx_message[11]);
            } else {
                grbl_msg_sendf(CLIENT_SERIAL,
                               MSG_LEVEL_INFO,
                               "%s Axis Dynamixel Detected ID %d M/N %d F/W Rev %x",
                               _axis_name,
                               _id,
                               dxl_rx_message[11]);               
            }

        } else {
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Axis Dynamixel Dynamixel Servo ID %d Ping failed", _axis_name, _id);
        }
    }

    // sets the PWM to zero. This allows most servos to be manually moved
    void Dynamixel2::set_disable(bool disable) {}

    void Dynamixel2::update() { set_location(); }

    void Dynamixel2::set_location() {}

    void Dynamixel2::read_settings() { _get_calibration(); }

    // this should change to use its own settings.
    void Dynamixel2::_get_calibration() {}

    // wait for and get the servo response
    uint16_t Dynamixel2::dxl_get_response(uint16_t length) {
        length = uart_read_bytes(UART_NUM_2, dxl_rx_message, length, DXL_RESPONSE_WAIT_TICKS);
        return length;
    }

    /*
    This is a helper function to complete and send the message
    The body of the message should be in msg, at the correct location
    before calling this function.
    This function will add the header, length bytes and CRC
    It will then send the message
*/
    void Dynamixel2::dxl_finish_message(char* msg, uint8_t servo_id, uint16_t msg_len) {
        //uint16_t msg_len;
        uint16_t crc = 0;
        // header
        msg[DXL_MSG_HDR1] = 0xFF;
        msg[DXL_MSG_HDR2] = 0xFF;
        msg[DXL_MSG_HDR3] = 0xFD;
        //
        // reserved
        msg[DXL_MSG_RSRV] = 0x00;
        msg[DXL_MSG_ID]   = servo_id;
        // length
        msg[DXL_MSG_LEN_L] = msg_len & 0xFF;
        msg[DXL_MSG_LEN_H] = (msg_len & 0xFF00) >> 8;

        // the message should already be here

        crc = dxl_update_crc(crc, msg, 5 + msg_len);

        msg[msg_len + 5] = crc & 0xFF;  // CRC_L
        msg[msg_len + 6] = (crc & 0xFF00) >> 8;

        // debug
        /*
    grbl_sendf(CLIENT_SERIAL, "[MSG: TX:");
    for (uint8_t index = 0; index < msg_len + 7; index++) {
        grbl_sendf(CLIENT_SERIAL, " 0x%02X", msg[index]);
    }
    grbl_sendf(CLIENT_SERIAL, "]\r\n");
	*/

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

#pragma once

/*
    Dynamixel2.h

    For control of Dynamixel Servos using Protocol 2.0

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

#define DYNAMIXEL_BUF_SIZE 127
#define DYNAMIXEL_BAUD_RATE 1000000

#define DXL_RESPONSE_WAIT_TICKS 20  // how long to wait for a response

// protocol 2 byte positions
#define DXL_MSG_HDR1 0
#define DXL_MSG_HDR2 1
#define DXL_MSG_HDR3 2
#define DXL_MSG_RSRV 3  // reserved byte
#define DXL_MSG_ID 4
#define DXL_MSG_LEN_L 5
#define DXL_MSG_LEN_H 6
#define DXL_MSG_INSTR 7
#define DXL_MSG_START 8

#define DXL_BROADCAST_ID 0xFE

// protocol 2 instruction numbers
#define DXL_INSTR_PING 0x01
#define PING_RSP_LEN 14
#define DXL_READ 0x02
#define DXL_WRITE 0x03
#define DXL_SYNC_WRITE 0x83
#define DXL_BULK_WRITE 0x93

// protocol 2 register locations
#define DXL_OPERATING_MODE 11
#define DXL_MOVING_THRESHOLD 24
#define DXL_ADDR_TORQUE_EN 64
#define DXL_ADDR_LED_ON 65
#define DXL_GOAL_POSITION 116     // 0x74
#define DXL_PRESENT_POSITION 132  // 0x84

// control modes
#define DXL_CONTROL_MODE_VELOCITY 1
#define DXL_CONTROL_MODE_POSITION 3
#define DXL_CONTROL_MODE_EXT_POSITION 4
#define DXL_CONTROL_MODE_PWM 16

#ifndef DXL_COUNT_MIN
#    define DXL_COUNT_MIN 1024
#endif

#ifndef DXL_COUNT_MAX
#    define DXL_COUNT_MAX 3072
#endif

#ifndef DYNAMIXEL_FULL_MOVE_TIME
#    define DYNAMIXEL_FULL_MOVE_TIME 1000  // time in milliseconds to do a full DYNAMIXEL_FULL_MOVE_TIME
#endif

#include "Motor.h"

namespace Motors {
    class Dynamixel2 : public Motor {
    public:
        Dynamixel2();
        Dynamixel2(uint8_t axis_index, uint8_t address, uint8_t tx_pin, uint8_t rx_pin, uint8_t rts_pin);

        virtual void config_message();
        virtual void init();
        virtual void set_disable(bool disable);
        virtual void update();

        static bool    uart_ready;
        static uint8_t ids[MAX_N_AXIS][2];

        void set_homing_mode(uint8_t homing_mask, bool isHoming) override;
        void read_settings();

    protected:
        void set_location();

        uint8_t _id;
        char    dxl_tx_message[50];  // outgoing to dynamixel
        uint8_t dxl_rx_message[50];  // received from dynamixel

        bool     test();
        uint16_t dxl_get_response(uint16_t length);
        uint32_t dxl_read_position();
        void     dxl_read(uint16_t address, uint16_t data_len);
        void     dxl_write(uint16_t address, uint8_t paramCount, ...);
        void     dxl_goal_position(int32_t position);  // set one motor
        void     set_operating_mode(uint8_t mode);
        void     LED_on(bool on);

        static void     init_uart(uint8_t id, uint8_t axis_index, uint8_t dual_axis_index);
        static void     dxl_finish_message(uint8_t id, char* msg, uint16_t msg_len);
        static uint16_t dxl_update_crc(uint16_t crc_accum, char* data_blk_ptr, uint8_t data_blk_size);
        static void     dxl_bulk_goal_position();  // set all motorsd init_uart(uint8_t id, uint8_t axis_index, uint8_t dual_axis_index);

        bool _disabled;

        float _homing_position;

        float _dxl_count_min;
        float _dxl_count_max;

        uint8_t     _tx_pin;
        uint8_t     _rx_pin;
        uint8_t     _rts_pin;
        uart_port_t _uart_num;
    };
}

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

#define DYNAMIXEL_BUF_SIZE            127
#define DYNAMIXEL_BAUD_RATE           1000000

#include "Motor.h"

namespace Motors {
    class Dynamixel2 : public Motor {
    public:
        Dynamixel2();
        Dynamixel2(uint8_t axis_index, uint8_t address, uint8_t tx_pin, uint8_t rx_pin, uint8_t rts_pin, float min, float max);
        virtual void config_message();
        virtual void init();
        virtual void set_disable(bool disable);
        virtual void update();
        void         read_settings();

        static bool uart_ready;

    protected:
        void set_location();
        void _get_calibration();

        static bool init_uart(uint8_t tx_pin, uint8_t rx_pin, uint8_t rts_pin);

        bool     _disabled;

        float _position_min;
        float _position_max;  // position in millimeters
        float _homing_position;

        float _pwm_pulse_min;
        float _pwm_pulse_max;

        uint8_t _tx_pin;          
        uint8_t _rx_pin;           
        uint8_t _rts_pin;
        uart_port_t _uart_num;

        uint8_t _address;        
    };
}

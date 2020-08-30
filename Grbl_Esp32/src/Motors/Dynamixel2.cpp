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

    Dynamixel2::Dynamixel2(uint8_t axis_index, uint8_t address, uint8_t tx_pin, uint8_t rx_pin, uint8_t rts_pin, float min, float max) {
        type_id               = DYNAMIXEL2;
        this->axis_index      = axis_index % MAX_AXES;
        this->dual_axis_index = axis_index < MAX_AXES ? 0 : 1;  // 0 = primary 1 = ganged
        _position_min         = min;
        _position_max         = max;
        _address              = address;
        _tx_pin               = tx_pin;
        _rx_pin               = rx_pin;
        _rts_pin              = rts_pin;
        init();
    }

    void Dynamixel2::init() {
        read_settings();
        is_active = true;  // as opposed to NullMotors, this is a real motor
        set_axis_name();

        /*
            _uart_num = sys_get_next_uart_num();

            if (_uart_num == UART_NUM_MAX) {
                grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "No avialable UART for Dynamixel Motors");
                return;
            }
            */

        _uart_num = UART_NUM_2;

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
        uart_param_config(_uart_num, &uart_config);
        uart_set_pin(_uart_num, _tx_pin, _rx_pin, _rts_pin, UART_PIN_NO_CHANGE);
        uart_driver_install(_uart_num, DYNAMIXEL_BUF_SIZE * 2, 0, 0, NULL, 0);
        uart_set_mode(_uart_num, UART_MODE_RS485_HALF_DUPLEX);

        config_message();
    }

    void Dynamixel2::config_message() {}

    // sets the PWM to zero. This allows most servos to be manually moved
    void Dynamixel2::set_disable(bool disable) {}

    void Dynamixel2::update() { set_location(); }

    void Dynamixel2::set_location() {}

    void Dynamixel2::read_settings() { _get_calibration(); }

    // this should change to use its own settings.
    void Dynamixel2::_get_calibration() {}
}

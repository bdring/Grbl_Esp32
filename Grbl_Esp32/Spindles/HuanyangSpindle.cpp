/*
    HuanyangSpindle.cpp

    This is for a Huanyang VFD based spindle via RS485 Modbus.

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

    // VFD frequencies are in Hz. Multiple by 60 for RPM

    // before using spindle, VFD must be setup for RS485 and match your spindle
    PD001   2   RS485 Control of run commands
    PD002   2   RS485 Control of operating frequency
    PD005   400 Maximum frequency Hz
    PD011   120 Min Speed (Recommend Aircooled=120 Water=100)
    PD014   10  Acceleration time (Test to optimize)
    PD015   10  Deceleration time (Test to optimize)
    PD023   1   Reverse run enabled
    PD142   3.7 Max current Amps (0.8kw=3.7 1.5kw=7.0)
    PD163   1   RS485 Address: 1
    PD164   1   RS485 Baud rate: 9600
    PD165   3   RS485 Mode: RTU, 8N1

    Some references....
    Manual: http://www.hy-electrical.com/bf/inverter.pdf
    Reference: https://github.com/Smoothieware/Smoothieware/blob/edge/src/modules/tools/spindle/HuanyangSpindleControl.cpp
    Refernece: https://gist.github.com/Bouni/803492ed0aab3f944066
    VFD settings: https://www.hobbytronics.co.za/Content/external/1159/Spindle_Settings.pdf

    TODO
        Returning errors to Grbl and handling them in Grbl.
        What happens if the VFD does not respond
        Add periodic pinging of VFD in run mode to see if it is still at correct RPM
*/

#include "driver/uart.h"

#define HUANYANG_ADDR           0x01
#define HUANYANG_UART_PORT      UART_NUM_2      // hard coded for this port right now
#define ECHO_TEST_CTS           UART_PIN_NO_CHANGE // CTS pin is not used
#define HUANYANG_BAUD_RATE      9600   // PD164 setting
#define HUANYANG_BUF_SIZE       127
#define RESPONSE_WAIT_TICKS     80 // how long to wait for a response

void HuanyangSpindle :: init() {

    // fail if numbers are not defined
    if (!get_pins_and_settings()) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Huanyang spindle errors");
        return;
    }

    uart_driver_delete(HUANYANG_UART_PORT); // this allows us to init() more than once if settings have changed.

    uart_config_t uart_config = {
        .baud_rate = HUANYANG_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    uart_param_config(HUANYANG_UART_PORT, &uart_config);

    uart_set_pin(HUANYANG_UART_PORT,
                 _txd_pin,
                 _rxd_pin,
                 _rts_pin,
                 UART_PIN_NO_CHANGE);

    uart_driver_install(HUANYANG_UART_PORT,
                        HUANYANG_BUF_SIZE * 2,
                        0,
                        0,
                        NULL,
                        0);

    uart_set_mode(HUANYANG_UART_PORT, UART_MODE_RS485_HALF_DUPLEX);

    is_reversable = true;

    config_message();
}

// Checks for all the required pin definitions
// It returns a message for each missing pin
// Returns true if all pins are defined.
bool HuanyangSpindle :: get_pins_and_settings() {
    bool pins_ok = true;

#ifdef HUANYANG_TXD_PIN
    _txd_pin = HUANYANG_TXD_PIN;
#else
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Missing HUANYANG_TXD_PIN");
    pins_ok = false;
#endif

#ifdef HUANYANG_RXD_PIN
    _rxd_pin = HUANYANG_RXD_PIN;
#else
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "No HUANYANG_RXD_PIN");
    pins_ok = false;
#endif

#ifdef HUANYANG_RTS_PIN
    _rts_pin = HUANYANG_RTS_PIN;
#else
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "No HUANYANG_RTS_PIN");
    pins_ok = false;
#endif

    return pins_ok;
}

void HuanyangSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Huanyang Spindle Tx:%d Rx:%d RTS:%d", _txd_pin, _rxd_pin, _rts_pin);
}

/*
    ADDR    CMD     LEN     DATA    CRC
    0x01    0x03    0x01    0x01    0x31 0x88                   Start spindle clockwise
    0x01    0x03    0x01    0x08    0xF1 0x8E                   Stop spindle
    0x01    0x03    0x01    0x11    0x30 0x44                   Start spindle counter-clockwise
*/
void HuanyangSpindle :: set_state(uint8_t state, float rpm) {
    if (sys.abort)
        return;   // Block during abort.

    _state = state; // store locally for faster get_state()

    if (!set_mode(state)) {  // try to set state. If it fails there is no need to try to set RPM
        system_set_exec_alarm(EXEC_ALARM_SPINDLE_CONTROL);
        return;
    }

    if (state == SPINDLE_DISABLE) {
        sys.spindle_speed = 0.0;
        return;
    }

    set_rpm(rpm);
    sys.report_ovr_counter = 0; // Set to report change immediately
}

bool HuanyangSpindle :: set_mode(uint8_t mode) {
    char msg[6] = {HUANYANG_ADDR, 0x03, 0x01, 0x00, 0x00, 0x00};

    if (mode == SPINDLE_ENABLE_CW)
        msg[3] = 0x01;
    else if (mode == SPINDLE_ENABLE_CCW)
        msg[3] = 0x11;
    else    //SPINDLE_DISABLE
        msg[3] = 0x08;

    add_ModRTU_CRC(msg, sizeof(msg));

    //report_hex_msg(msg, "To VFD:", sizeof(msg));  // TODO for debugging comment out

    uart_write_bytes(HUANYANG_UART_PORT, msg, sizeof(msg));

    return get_response(6);
}


bool HuanyangSpindle :: get_response(uint16_t length) {
    uint8_t rx_message[10];

    uint16_t read_length = uart_read_bytes(HUANYANG_UART_PORT, rx_message, length, RESPONSE_WAIT_TICKS);
    if (read_length < length) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RS485 Unresponsive");
        return false;
    }
    // check CRC?
    // Check address?

    return true;
}

/*
    ADDR    CMD     LEN     DATA        CRC
    0x01    0x05    0x02    0x09 0xC4   0xBF 0x0F               Write Frequency (0x9C4 = 2500 = 25.00HZ)
*/
float HuanyangSpindle :: set_rpm(float rpm) {

    // TODO add in all the speed modifiers, like override and linearization

    char msg[7] = {HUANYANG_ADDR, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00};

    // add data (rpm) bytes
    uint16_t data = uint16_t(rpm / 60.0 * 100.0); // send Hz * 10  (Ex:1500 RPM = 25Hz .... Send 2500)
    msg[3] = (data & 0xFF00) >> 8;
    msg[4] = (data & 0xFF);

    add_ModRTU_CRC(msg, sizeof(msg));

    //report_hex_msg(msg, "To VFD:", sizeof(msg));  // TODO for debugging comment out

    uart_write_bytes(HUANYANG_UART_PORT, msg, sizeof(msg));
    get_response(6);

    return rpm;
}

void HuanyangSpindle ::stop() {
    set_mode(SPINDLE_DISABLE);
}

// state is cached rather than read right now to prevent delays
uint8_t HuanyangSpindle :: get_state() {
    return _state;
}

// Calculate the CRC on all of the byte except the last 2
// It then added the CRC to those last 2 bytes
// full_msg_len This is the length of the message including the 2 crc bytes
// Source: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
void HuanyangSpindle :: add_ModRTU_CRC(char* buf, int full_msg_len) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < full_msg_len - 2; pos++) {
        crc ^= (uint16_t)buf[pos];          // XOR byte into least sig. byte of crc
        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {      // If the LSB is set
                crc >>= 1;                      // Shift right and XOR 0xA001
                crc ^= 0xA001;
            } else                          // Else LSB is not set
                crc >>= 1;                    // Just shift right
        }
    }
    // add the calculated Crc to the message
    buf[full_msg_len - 1] = (crc & 0xFF00) >> 8;
    buf[full_msg_len - 2] = (crc & 0xFF);
}

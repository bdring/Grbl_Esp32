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

                         WARNING!!!!
    VFDs are very dangerous. They have high voltages and are very powerful
    Remove power before changing bits.

    VFD frequencies are in Hz. Multiply by 60 for RPM

    before using spindle, VFD must be setup for RS485 and match your spindle
    PD001   2   RS485 Control of run commands
    PD002   2   RS485 Control of operating frequency
    PD005   400 Maximum frequency Hz (Typical for spindles)
    PD011   120 Min Speed (Recommend Aircooled=120 Water=100)
    PD014   10  Acceleration time (Test to optimize)
    PD015   10  Deceleration time (Test to optimize)
    PD023   1   Reverse run enabled
    PD142   3.7 Max current Amps (0.8kw=3.7 1.5kw=7.0, 2.2kw=??)
    PD163   1   RS485 Address: 1 (Typical. OK to change...see below)
    PD164   1   RS485 Baud rate: 9600 (Typical. OK to change...see below)
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
#include "SpindleClass.h"

#include "driver/uart.h"

#define HUANYANG_UART_PORT      UART_NUM_2      // hard coded for this port right now
#define ECHO_TEST_CTS           UART_PIN_NO_CHANGE // CTS pin is not used
#define HUANYANG_BUF_SIZE       127
#define RESPONSE_WAIT_TICKS     50 // how long to wait for a response
#define HUANYANG_MAX_MSG_SIZE   16   // more than enough for a modbus message

// OK to change these
// #define them in your machine definition file if you want different values
#ifndef HUANYANG_ADDR
    #define HUANYANG_ADDR           0x01
#endif

#ifndef HUANYANG_BAUD_RATE
    #define HUANYANG_BAUD_RATE      9600   // PD164 setting
#endif

// communication task and queue stuff
typedef struct {
    uint8_t tx_length;
    uint8_t rx_length;
    bool critical;
    char msg[HUANYANG_MAX_MSG_SIZE];
} hy_command_t;

QueueHandle_t hy_cmd_queue;

static TaskHandle_t vfd_cmdTaskHandle = 0;

// The communications task
void vfd_cmd_task(void* pvParameters) {
    hy_command_t next_cmd;
    uint8_t rx_message[HUANYANG_MAX_MSG_SIZE];

    while (true) {
        if (xQueueReceive(hy_cmd_queue, &next_cmd, 0) == pdTRUE) {

            uart_flush(HUANYANG_UART_PORT);
            uart_write_bytes(HUANYANG_UART_PORT, next_cmd.msg, next_cmd.tx_length);

            uint16_t read_length = uart_read_bytes(HUANYANG_UART_PORT, rx_message, next_cmd.rx_length, RESPONSE_WAIT_TICKS);

            if (read_length < next_cmd.rx_length) {
                grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RS485 Unresponsive");
                if (next_cmd.critical)
                    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Critical Spindle RS485 Unresponsive");
                // TODO Do something with this error
                system_set_exec_alarm(EXEC_ALARM_SPINDLE_CONTROL);
            } else {
                
            }
        } else {
            // TODO: Should we ping the spindle here to make sure it does not go off line?
            // But there is virtually no real documentation on how to do this.
        }
        vTaskDelay(100); // TODO: What is the best value here?
    }
}

// ================== Class methods ==================================

void HuanyangSpindle :: init() {

    if (! _task_running) { // init can happen many times, we only want to start one task
        hy_cmd_queue = xQueueCreate(5, sizeof(hy_command_t));
        xTaskCreatePinnedToCore(vfd_cmd_task,      // task
                                "vfd_cmdTaskHandle", // name for task
                                2048,   // size of task stack
                                NULL,   // parameters
                                1, // priority
                                &vfd_cmdTaskHandle,
                                0 // core
                               );
        _task_running = true;
    }

    // fail if required items are not defined
    if (!get_pins_and_settings()) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Huanyang spindle errors");
        return;
    }

    // this allows us to init() again later.
    // If you change certain settings, init() gets called agian
    uart_driver_delete(HUANYANG_UART_PORT);

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

    is_reversable = true; // these VFDs are always reversable
    use_delays = true;

    //
    _current_rpm = 0;
    _state = SPINDLE_DISABLE;

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
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "Huanyang Spindle Tx:%s Rx:%s RTS:%s",
                   pinName(_txd_pin).c_str(),
                   pinName(_rxd_pin).c_str(),
                   pinName(_rts_pin).c_str());

}

/*
    ADDR    CMD     LEN     DATA    CRC
    0x01    0x03    0x01    0x01    0x31 0x88                   Start spindle clockwise
    0x01    0x03    0x01    0x08    0xF1 0x8E                   Stop spindle
    0x01    0x03    0x01    0x11    0x30 0x44                   Start spindle counter-clockwise


    0x01    0x04    0x03    0x00    0x00 0x00   0xF0 0x4E       Read Frequency
*/
void HuanyangSpindle :: set_state(uint8_t state, uint32_t rpm) {
    if (sys.abort)
        return;   // Block during abort.

    bool critical = (sys.state == STATE_CYCLE || state != SPINDLE_DISABLE);
    
    if (_current_state != state) { // already at the desired state. This function gets called a lot.
        set_mode(state, critical); // critical if we are in a job
        set_rpm(rpm);
        if (state == SPINDLE_DISABLE) {
            sys.spindle_speed = 0;
            if (_current_state != state)
                mc_dwell(spindle_delay_spindown->get());
        } else {
            if (_current_state != state)
                mc_dwell(spindle_delay_spinup->get());
        }
    } else {
        if (_current_rpm != rpm)
            set_rpm(rpm);
    }

    _current_state = state; // store locally for faster get_state()

    sys.report_ovr_counter = 0; // Set to report change immediately

    return;
}

bool HuanyangSpindle :: set_mode(uint8_t mode, bool critical) {
    hy_command_t mode_cmd;

    mode_cmd.tx_length = 6;
    mode_cmd.rx_length = 6;

    mode_cmd.msg[0] = HUANYANG_ADDR;
    mode_cmd.msg[1] = 0x03;
    mode_cmd.msg[2] = 0x01;

    if (mode == SPINDLE_ENABLE_CW)
        mode_cmd.msg[3] = 0x01;
    else if (mode == SPINDLE_ENABLE_CCW)
        mode_cmd.msg[3] = 0x11;
    else    //SPINDLE_DISABLE
        mode_cmd.msg[3] = 0x08;

    add_ModRTU_CRC(mode_cmd.msg, mode_cmd.rx_length);

    mode_cmd.critical = critical;
    ;
    if (xQueueSend(hy_cmd_queue, &mode_cmd, 0) != pdTRUE)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "VFD Queue Full");

    return true;
}

/*
    ADDR    CMD     LEN     DATA        CRC
    0x01    0x05    0x02    0x09 0xC4   0xBF 0x0F               Write Frequency (0x9C4 = 2500 = 25.00HZ)
*/
uint32_t HuanyangSpindle :: set_rpm(uint32_t rpm) {
    hy_command_t rpm_cmd;

    if (rpm == _current_rpm) // prevent setting same RPM twice
        return rpm;

    _current_rpm = rpm;

    // TODO add the speed modifiers override, linearization, etc.

    rpm_cmd.tx_length = 7;
    rpm_cmd.rx_length = 6;

    rpm_cmd.msg[0] = HUANYANG_ADDR;
    rpm_cmd.msg[1] = 0x05;
    rpm_cmd.msg[2] = 0x02;

    uint16_t data = (uint16_t)(rpm * 100 / 60); // send Hz * 10  (Ex:1500 RPM = 25Hz .... Send 2500)

    rpm_cmd.msg[3] = (data & 0xFF00) >> 8;
    rpm_cmd.msg[4] = (data & 0xFF);

    add_ModRTU_CRC(rpm_cmd.msg, rpm_cmd.tx_length);

    rpm_cmd.critical = (sys.state == STATE_CYCLE);

    if (xQueueSend(hy_cmd_queue, &rpm_cmd, 0) != pdTRUE)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "VFD Queue Full");

    return rpm;
}

// 0x01    0x04    0x03    0x00    0x00 0x00   0xF0 0x4E       Read Frequency
// This appears to read the control register and will return an RPM running or not.
uint16_t HuanyangSpindle :: read_rpm() {
    hy_command_t read_cmd;
    uint8_t rx_message[HUANYANG_MAX_MSG_SIZE];

    read_cmd.tx_length = 8;
    read_cmd.rx_length = 6;

    read_cmd.msg[0] = HUANYANG_ADDR;
    read_cmd.msg[1] = 0x04;
    read_cmd.msg[2] = 0x03;

    read_cmd.msg[3] = 0x00;

    read_cmd.msg[4] = 0x00;
    read_cmd.msg[5] = 0x00;

    add_ModRTU_CRC(read_cmd.msg, read_cmd.tx_length);

    uart_flush(HUANYANG_UART_PORT);
    uart_write_bytes(HUANYANG_UART_PORT, read_cmd.msg, read_cmd.tx_length);

    uint16_t read_length = uart_read_bytes(HUANYANG_UART_PORT, rx_message, read_cmd.rx_length, RESPONSE_WAIT_TICKS);

    if (read_length < read_cmd.rx_length)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Read RPM Spindle RS485 Unresponsive");
    else {
        uint32_t hz = ((uint32_t)rx_message[4] << 8) + rx_message[5];
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "speed %d", (hz * 60) / 100);

        report_hex_msg(rx_message, "Rx:", read_length);
    }

    return 0;
}

void HuanyangSpindle ::stop() {
    set_mode(SPINDLE_DISABLE, false);
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

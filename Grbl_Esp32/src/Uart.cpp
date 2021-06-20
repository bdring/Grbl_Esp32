/*
 * UART driver that accesses the ESP32 hardware FIFOs directly.
 */

#include "Grbl.h"

#include "esp_system.h"
#include "soc/uart_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/dport_reg.h"
#include "soc/rtc.h"

Uart::Uart(int uart_num) : _uart_num(uart_port_t(uart_num)), _pushback(-1) {}

void Uart::begin(unsigned long baudrate, Data dataBits, Stop stopBits, Parity parity) {
    //    uart_driver_delete(_uart_num);
    uart_config_t conf;
    conf.baud_rate           = baudrate;
    conf.data_bits           = uart_word_length_t(dataBits);
    conf.parity              = uart_parity_t(parity);
    conf.stop_bits           = uart_stop_bits_t(stopBits);
    conf.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
    conf.rx_flow_ctrl_thresh = 0;
    conf.use_ref_tick        = false;
    if (uart_param_config(_uart_num, &conf) != ESP_OK) {
        return;
    };
    uart_driver_install(_uart_num, 256, 0, 0, NULL, 0);
}

int Uart::available() {
    size_t size = 0;
    uart_get_buffered_data_len(_uart_num, &size);
    return size + (_pushback >= 0);
}

int Uart::peek() {
    _pushback = read();
    return _pushback;
}

int Uart::read(TickType_t timeout) {
    if (_pushback >= 0) {
        int ret   = _pushback;
        _pushback = -1;
        return ret;
    }
    uint8_t c;
    int     res = uart_read_bytes(_uart_num, &c, 1, timeout);
    return res != 1 ? -1 : c;
}
int Uart::read() {
    return read(0);
}

size_t Uart::readBytes(char* buffer, size_t length, TickType_t timeout) {
    bool pushback = _pushback >= 0;
    if (pushback && length) {
        *buffer++ = _pushback;
        _pushback = -1;
        --length;
    }
    int res = uart_read_bytes(_uart_num, (uint8_t*)buffer, length, timeout);
    // The Stream class version of readBytes never returns -1,
    // so if uart_read_bytes returns -1, we change that to 0
    return pushback + (res >= 0 ? res : 0);
}
size_t Uart::readBytes(char* buffer, size_t length) {
    return readBytes(buffer, length, (TickType_t)0);
}
size_t Uart::write(uint8_t c) {
    return uart_write_bytes(_uart_num, (char*)&c, 1);
}

size_t Uart::write(const uint8_t* buffer, size_t length) {
    return uart_write_bytes(_uart_num, (const char*)buffer, length);
}

size_t Uart::write(const char* text) {
    return uart_write_bytes(_uart_num, text, strlen(text));
}

bool Uart::setHalfDuplex() {
    return uart_set_mode(_uart_num, UART_MODE_RS485_HALF_DUPLEX) != ESP_OK;
}
bool Uart::setPins(int tx_pin, int rx_pin, int rts_pin, int cts_pin) {
    return uart_set_pin(_uart_num, tx_pin, rx_pin, rts_pin, cts_pin) != ESP_OK;
}
bool Uart::flushTxTimed(TickType_t ticks) {
    return uart_wait_tx_done(_uart_num, ticks) != ESP_OK;
}

Uart Uart0(0);

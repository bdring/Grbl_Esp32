/*
 * UART driver that accesses the ESP32 hardware FIFOs directly.
 */

#include "Grbl.h"
#include "Uart.h"

#include "esp_system.h"
#include "soc/uart_reg.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_sig_map.h"
#include "soc/dport_reg.h"
#include "soc/rtc.h"

void Uart::flushTx() {
    while (READ_PERI_REG(UART_STATUS_REG(_uart_num)) & (UART_TXFIFO_CNT_M | UART_ST_UTX_OUT)) {}
}

void Uart::setBaudRate(uint32_t baud_rate) {
    uint32_t clk_div        = ((getApbFrequency() << 4) / baud_rate);
    uint32_t clk_div_regval = ((clk_div >> 4) << UART_CLKDIV_S) | ((clk_div & UART_CLKDIV_FRAG_V) << UART_CLKDIV_FRAG_S);
    WRITE_PERI_REG(UART_CLKDIV_REG(_uart_num), clk_div_regval);
}

Uart::Uart(int uart_num) : _uart_num(uart_num) {}

void Uart::begin(unsigned long baudrate, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms) {
    if (rxPin == -1 && txPin == -1) {
        return;
    }

    switch (_uart_num) {
        case 0:
            DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_UART_CLK_EN);
            DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_UART_RST);
            break;
        case 1:
            DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_UART1_CLK_EN);
            DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_UART1_RST);
            break;
        case 2:
            DPORT_SET_PERI_REG_MASK(DPORT_PERIP_CLK_EN_REG, DPORT_UART2_CLK_EN);
            DPORT_CLEAR_PERI_REG_MASK(DPORT_PERIP_RST_EN_REG, DPORT_UART2_RST);
            break;
        default:
            break;
            // Can't happen
    }

    flushTx();
    setBaudRate(baudrate);

    // For now we hardcode the config as 8N1

#define UART_8_DATABITS 3  // 0:5bits, 1:6bits, 2:7bits, 3:8bits
#define UART_1_STOP_BIT 1  // 1:1bit, 2:1.5bits, 3:2bits
    uint32_t uart_conf0 = READ_PERI_REG(UART_CONF0_REG(_uart_num));
    uart_conf0 &= ~(UART_STOP_BIT_NUM_M | UART_BIT_NUM_M | UART_PARITY_EN_M);
    uart_conf0 |= (UART_8_DATABITS << UART_BIT_NUM_S) | (UART_1_STOP_BIT << UART_STOP_BIT_NUM_S) | (0 << UART_PARITY_EN_S);
    WRITE_PERI_REG(UART_CONF0_REG(_uart_num), uart_conf0);

    // tx_idle_num : idle interval after tx FIFO is empty(unit: the time it takes to send one bit under current baudrate)
    // Setting it to 0 prevents line idle time/delays when sending messages with small intervals
    uint32_t tx_idle_conf = READ_PERI_REG(UART_IDLE_CONF_REG(_uart_num));
    tx_idle_conf &= ~UART_TX_IDLE_NUM;
    tx_idle_conf |= 0 << UART_TX_IDLE_NUM_S;  // Does nothing but here for documentation
    WRITE_PERI_REG(UART_IDLE_CONF_REG(_uart_num), tx_idle_conf);

#define UART_RXD_IDX(u) ((u == 0) ? U0RXD_IN_IDX : ((u == 1) ? U1RXD_IN_IDX : ((u == 2) ? U2RXD_IN_IDX : 0)))
    if (rxPin != -1) {
        pinMode(rxPin, INPUT);
        pinMatrixInAttach(rxPin, UART_RXD_IDX(_uart_num), false);  // false is not inverted
    }

#define UART_TXD_IDX(u) ((u == 0) ? U0TXD_OUT_IDX : ((u == 1) ? U1TXD_OUT_IDX : ((u == 2) ? U2TXD_OUT_IDX : 0)))
    if (txPin != -1) {
        pinMode(txPin, OUTPUT);
        pinMatrixOutAttach(txPin, UART_TXD_IDX(_uart_num), false, false);
    }
#if 0
#endif
}

const int RxFIFOLen = 128;
int       Uart::available() {
    uint32_t val     = READ_PERI_REG(UART_MEM_RX_STATUS_REG(_uart_num));
    uint32_t rd_addr = (val >> UART_MEM_RX_RD_ADDR_S) & UART_MEM_RX_RD_ADDR_V;
    uint32_t wr_addr = (val >> UART_MEM_RX_WR_ADDR_S) & UART_MEM_RX_WR_ADDR_V;
    uint32_t cnt     = wr_addr - rd_addr;
    return cnt & (RxFIFOLen - 1);
}

int Uart::read() {
    if (available()) {
        return READ_PERI_REG(UART_FIFO_REG(_uart_num));
    }
    return -1;
}

uint32_t Uart::txFIFOCnt() {
    uint32_t val = READ_PERI_REG(UART_STATUS_REG(_uart_num));
    return (val >> UART_TXFIFO_CNT_S) & UART_TXFIFO_CNT_V;
}

size_t Uart::write(uint8_t c) {
    while (txFIFOCnt() == 0x7f) {}
    // Access the FIFO via AHB instead of DPORT because of erratum 3.16 in
    // https://espressif.com/sites/default/files/documentation/eco_and_workarounds_for_bugs_in_esp32_en.pdf
    WRITE_PERI_REG(UART_FIFO_AHB_REG(_uart_num), c);
    return 1;
}

size_t Uart::write(const char* text) {
    size_t  i = 0;
    uint8_t c;
    for (i = 0; (c = (uint8_t)*text++) != '\0'; i++) {
        write(c);
    }
    return i;
}

Uart Uart0(0);

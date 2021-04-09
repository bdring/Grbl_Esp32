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

#ifdef DIRECT_UART
void Uart::flushTx() {
    while (READ_PERI_REG(UART_STATUS_REG(_uart_num)) & (UART_TXFIFO_CNT_M | UART_ST_UTX_OUT)) {}
}
#endif

void Uart::setBaudRate(uint32_t baud_rate) {
#ifdef DIRECT_UART
    uint32_t clk_div        = ((getApbFrequency() << 4) / baud_rate);
    uint32_t clk_div_regval = ((clk_div >> 4) << UART_CLKDIV_S) | ((clk_div & UART_CLKDIV_FRAG_V) << UART_CLKDIV_FRAG_S);
    WRITE_PERI_REG(UART_CLKDIV_REG(_uart_num), clk_div_regval);
#else
    uart_set_baudrate(_uart_num, baud_rate);
#endif
}

Uart::Uart(int uart_num) : _uart_num(uart_port_t(uart_num)) {}

void Uart::begin(unsigned long baudrate, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms) {
#ifdef DIRECT_UART
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
            return;
    }

    flushTx();
    setBaudRate(baudrate);

    // The config value is the actual number that goes into the register.
    // Names like SERIAL_8N1 are defined in esp32-hal-uart.h in the
    // Arduino framework code.
    if ((config & UART_STOP_BIT_NUM_M) == (3 << UART_STOP_BIT_NUM_S)) {
        // The stop bit field is 1 for 1 stop bit, 2 for 1.5, and 3 for 2 stop bits.
        // If the config specifies two stop bits, we change it to one
        // stop bit and enable a delay in the RS485 register.  Espressif
        // UART drivers contain this workaround and attribute it to some
        // hardware bug, but the bug is not listed in the errata document.
        config = (config & ~UART_STOP_BIT_NUM_M) | (1 << UART_STOP_BIT_NUM_S);
        DPORT_SET_PERI_REG_MASK(UART_RS485_CONF_REG(_uart_num), UART_DL1_EN_M);
    } else {
        DPORT_CLEAR_PERI_REG_MASK(UART_RS485_CONF_REG(_uart_num), UART_DL1_EN_M);
    }
    WRITE_PERI_REG(UART_CONF0_REG(_uart_num), config);

    // tx_idle_num : idle interval after tx FIFO is empty(unit: the time it takes to send one bit under current baudrate)
    // Setting it to 0 prevents line idle time/delays when sending messages with small intervals
    uint32_t tx_idle_conf = READ_PERI_REG(UART_IDLE_CONF_REG(_uart_num));
    tx_idle_conf &= ~UART_TX_IDLE_NUM_M;
    tx_idle_conf |= 0 << UART_TX_IDLE_NUM_S;  // Does nothing but here for documentation
    WRITE_PERI_REG(UART_IDLE_CONF_REG(_uart_num), tx_idle_conf);

#    define UART_RXD_IDX(u) ((u == 0) ? U0RXD_IN_IDX : ((u == 1) ? U1RXD_IN_IDX : ((u == 2) ? U2RXD_IN_IDX : 0)))
    if (rxPin != -1) {
        pinMode(rxPin, INPUT);
        pinMatrixInAttach(rxPin, UART_RXD_IDX(_uart_num), false);  // false is not inverted
    }

#    define UART_TXD_IDX(u) ((u == 0) ? U0TXD_OUT_IDX : ((u == 1) ? U1TXD_OUT_IDX : ((u == 2) ? U2TXD_OUT_IDX : 0)))
    if (txPin != -1) {
        pinMode(txPin, OUTPUT);
        pinMatrixOutAttach(txPin, UART_TXD_IDX(_uart_num), false, false);
    }
#else
    //    uart_driver_delete(_uart_num);
    uart_config_t conf;
    conf.baud_rate           = baudrate;
    conf.data_bits           = UART_DATA_8_BITS;
    conf.parity              = UART_PARITY_DISABLE;
    conf.stop_bits           = UART_STOP_BITS_1;
    conf.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
    conf.rx_flow_ctrl_thresh = 0;
    conf.use_ref_tick        = false;
    if (uart_param_config(_uart_num, &conf) != ESP_OK) {
        return;
    };
    uart_driver_install(_uart_num, 256, 0, 0, NULL, 0);
#endif
}

int Uart::available() {
#ifdef DIRECT_UART
    // RxFIFOLen be a power of two otherwise the & computation below will not work.
    // The hardware only allows power-of-two lengths anyway.
    const int RxFIFOLen = 128;
    uint32_t  val       = READ_PERI_REG(UART_MEM_RX_STATUS_REG(_uart_num));
    uint32_t  rd_addr   = (val >> UART_MEM_RX_RD_ADDR_S) & UART_MEM_RX_RD_ADDR_V;
    uint32_t  wr_addr   = (val >> UART_MEM_RX_WR_ADDR_S) & UART_MEM_RX_WR_ADDR_V;
    uint32_t  cnt       = wr_addr - rd_addr;
    return cnt & (RxFIFOLen - 1);
#else
    size_t size = 0;
    uart_get_buffered_data_len(_uart_num, &size);
    return size;
#endif
}

int Uart::read() {
#ifdef DIRECT_UART
    if (available()) {
        return READ_PERI_REG(UART_FIFO_REG(_uart_num));
    }
    return -1;
#else
    uint8_t c;
    int     res = uart_read_bytes(_uart_num, &c, 1, 0);
    return res != 1 ? -1 : c;
#endif
}

#ifdef DIRECT_UART
uint32_t Uart::txFIFOCnt() {
    uint32_t val = READ_PERI_REG(UART_STATUS_REG(_uart_num));
    return (val >> UART_TXFIFO_CNT_S) & UART_TXFIFO_CNT_V;
}
#endif

size_t Uart::write(uint8_t c) {
#ifdef DIRECT_UART
    while (txFIFOCnt() == 0x7f) {}
    // Access the FIFO via AHB instead of DPORT because of erratum 3.16 in
    // https://espressif.com/sites/default/files/documentation/eco_and_workarounds_for_bugs_in_esp32_en.pdf
    WRITE_PERI_REG(UART_FIFO_AHB_REG(_uart_num), c);
    return 1;
#else
    return uart_write_bytes(_uart_num, (char*)&c, 1);
#endif
}

size_t Uart::write(const char* text) {
#ifdef DIRECT_UART
    size_t  i = 0;
    uint8_t c;
    for (i = 0; (c = (uint8_t)*text++) != '\0'; i++) {
        write(c);
    }
    return i;
#else
    return uart_write_bytes(_uart_num, text, strlen(text));
#endif
}

Uart Uart0(0);

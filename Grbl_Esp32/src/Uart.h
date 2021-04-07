#pragma once

class Uart {
private:
    uint32_t _uart_num;
    void     flushTx();
    void     setBaudRate(uint32_t baud_rate);
    uint32_t rxFIFOCnt();
    uint32_t txFIFOCnt();

public:
    Uart(int uart_num);
    void   begin(unsigned long baud, uint32_t config, int8_t rxPin, int8_t txPin, bool invert, unsigned long timeout_ms = 20000UL);
    int    available(void);
    int    read(void);
    size_t write(uint8_t data);
    size_t write(const char* text);
};

extern Uart Uart0;

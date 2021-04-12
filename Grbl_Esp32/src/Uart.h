#pragma once

#ifdef DIRECT_UART
#else
#    include <driver/uart.h>
#endif

class Uart : public Stream {
private:
    uart_port_t _uart_num;
    int         _pushback;

public:
    static const int DataBits5;
    static const int DataBits6;
    static const int DataBits7;
    static const int DataBits8;

    static const int StopBits1;
    static const int StopBits1_5;
    static const int StopBits2;

    static const int ParityNone;
    static const int ParityEven;
    static const int ParityOdd;

    Uart(int uart_num);
    bool          setHalfDuplex();
    bool          setPins(int tx_pin, int rx_pin, int rts_pin = -1, int cts_pin = -1);
    void          begin(unsigned long baud, int dataBits, int stopBits, int parity);
    int           available(void) override;
    int           read(void) override;
    int           read(TickType_t timeout);
    size_t        readBytes(char* buffer, size_t length, TickType_t timeout);
    size_t        readBytes(uint8_t* buffer, size_t length, TickType_t timeout) { return readBytes((char*)buffer, length, timeout); }
    size_t        readBytes(char* buffer, size_t length) override;
    int           peek(void) override;
    size_t        write(uint8_t data);
    size_t        write(const uint8_t* buffer, size_t length);
    inline size_t write(const char* buffer, size_t size) { return write((uint8_t*)buffer, size); }
    size_t        write(const char* text);
    void          flush() { uart_flush(_uart_num); }
    bool          flushTxTimed(TickType_t ticks);
};

extern Uart Uart0;

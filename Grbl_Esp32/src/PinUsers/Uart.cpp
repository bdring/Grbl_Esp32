#include "Uart.h"

#include "../Pin.h"
#include "../Pins/PinOptionsParser.h"
#include "../Assert.h"
#include "LimitedResource.h"

#include <driver/uart.h>

namespace PinUsers {
    // Native UART in the ESP32
    class NativeUart : public UartDetail {
        static LimitedResource<2>& UartResources() {
            // The ESP32 chip has three UART controllers (UART0, UART1, and UART2), but
            // UART0 is used for Serial0. In other words, we can only make 1 and 2 available.

            static LimitedResource<2> instances_;
            return instances_;
        }

        Pin tx_;
        Pin rx_;
        Pin rts_;

        int           bufferSize_;
        uart_config_t uartConfig_;
        uart_port_t   uartPort_;
        uart_mode_t   uartMode_;

    public:
        NativeUart(Pin tx, Pin rx, Pin rts, Pins::PinOptionsParser& options, Pins::PinOptionsParser& userOptions) :
            tx_(tx), rx_(rx), rts_(rts), uartPort_(UART_NUM_MAX) {
            // Validate if claiming the resources will err:
            Assert(tx.capabilities().has(Pin::Capabilities::Native | Pin::Capabilities::UART | Pin::Capabilities::Output));
            Assert(rx.capabilities().has(Pin::Capabilities::Native | Pin::Capabilities::UART | Pin::Capabilities::Input));
            Assert(rts.capabilities().has(Pin::Capabilities::Native | Pin::Capabilities::UART | Pin::Capabilities::Output));

            // Iterate options:
            uartConfig_ = { 0 };
            bufferSize_ = 128;

            // 115200 8N1 = default.
            uartConfig_.baud_rate           = 115200;
            uartConfig_.data_bits           = UART_DATA_8_BITS;
            uartConfig_.flow_ctrl           = UART_HW_FLOWCTRL_DISABLE;
            uartConfig_.parity              = UART_PARITY_DISABLE;
            uartConfig_.rx_flow_ctrl_thresh = 122;
            uartConfig_.stop_bits           = UART_STOP_BITS_1;

            uartMode_ = UART_MODE_UART;

            for (auto opt : options) {
                if (opt.is("flowControl")) {
                    auto val = opt.value();
                    if (!strcmp(val, "RTS")) {
                        uartConfig_.flow_ctrl = UART_HW_FLOWCTRL_RTS;
                    } else if (!strcmp(val, "CTS")) {
                        uartConfig_.flow_ctrl = UART_HW_FLOWCTRL_CTS;
                    } else if (!strcmp(val, "CTS+RTS")) {
                        uartConfig_.flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS;
                    } else {
                        Assert(false, "Flow control option passed is invalid: %s", val);
                    }
                } else if (opt.is("bufsize")) {
                    bufferSize_ = opt.iValue();
                } else if (opt.is("mode")) {
                    auto val = opt.value();
                    if (!strcmp(val, "UART")) {
                        uartMode_ = UART_MODE_UART;
                    } else if (!strcmp(val, "RS485")) {
                        uartMode_ = UART_MODE_RS485_HALF_DUPLEX;
                    } else if (!strcmp(val, "IRDA")) {
                        uartMode_ = UART_MODE_IRDA;
                    } else {
                        Assert(false, "Unsupported mode passed to uart mode: %s", val);
                    }
                }
            }

            for (auto opt : userOptions) {
                if (opt.is("baud")) {
                    uartConfig_.baud_rate = opt.iValue();
                } else if (opt.is("data")) {
                    /* 8N1 etc */
                    auto modeStr = opt.value();
                    Assert(::strlen(modeStr) == 3, "Uart mode should be [bits][parity mode][parity bits], like 8N1");
                    Assert(::isdigit(modeStr[0]), "Expected first character of uart mode to be a number");
                    Assert(::isdigit(modeStr[2]), "Expected third character of uart mode to be a number");

                    int dataBits = modeStr[0] - '0';
                    Assert(dataBits >= 5 && dataBits <= 8, "Uart mode data bits should be in the range [5,8]");
                    uartConfig_.data_bits = static_cast<uart_word_length_t>(UART_DATA_5_BITS + (dataBits - 5));

                    int numberStop = modeStr[2] - '0';
                    if (numberStop == 1) {
                        uartConfig_.stop_bits = UART_STOP_BITS_1;
                    } else if (numberStop == 2) {
                        uartConfig_.stop_bits = UART_STOP_BITS_2;
                    } else {
                        Assert(false, "Number of stop bits in uart mode should be 1 or 2");
                    }

                    char type = modeStr[1];
                    if (type == 'N' || type == 'n') {
                        uartConfig_.parity = UART_PARITY_DISABLE;
                    } else if (type == 'E' || type == 'e') {
                        uartConfig_.parity = UART_PARITY_EVEN;
                    } else if (type == 'O' || type == 'o') {
                        uartConfig_.parity = UART_PARITY_ODD;
                    } else {
                        Assert(false, "Parity mode in uart mode should be odd (O), even (E) or none (N)");
                    }
                }
            }

            int uartIndex = UartResources().tryClaim();
            Assert(uartIndex >= 0);
            uartPort_ = static_cast<uart_port_t>(uartIndex + 1);

            // We have everything we need. Set it up:

            auto txn  = tx_.getNative(Pin::Capabilities::UART | Pin::Capabilities::Output);
            auto rxn  = rx_.getNative(Pin::Capabilities::UART | Pin::Capabilities::Output);
            auto rtsn = rts_.getNative(Pin::Capabilities::UART | Pin::Capabilities::Output);

            try {
                Assert(uart_param_config(uartPort_, &uartConfig_) == ESP_OK, "Uart parameter set failed.");
                Assert(uart_set_pin(uartPort_, txn, rxn, rtsn, UART_PIN_NO_CHANGE) == ESP_OK, "Uart parameter set failed.");
                Assert(uart_driver_install(uartPort_, bufferSize_ * 2, 0, 0, NULL, 0) == ESP_OK, "Uart driver install failed.");

                if (uart_set_mode(uartPort_, uartMode_) != ESP_OK) {
                    uart_driver_delete(uartPort_);
                    Assert(false, "UART set mode failed");
                }
            } catch (const AssertionFailed& ex) {
                UartResources().release(int(uartPort_) - 1);
                uartPort_ = UART_NUM_MAX;

                throw;
            }
        }

        int write(const uint8_t* ptr, int bytes) override {
            // Flush the UART and write the data:
            uart_flush(uartPort_);
            return uart_write_bytes(uartPort_, reinterpret_cast<const char*>(ptr), bytes);
        }

        int read(uint8_t* ptr, int bytes, int ticksToWait) override {
            // Read the response
            return uart_read_bytes(uartPort_, ptr, bytes, ticksToWait);
        }

        ~NativeUart() override {
            // Tear down the uart, give back all resources.
            if (uartPort_ != UART_NUM_MAX) {
                uart_driver_delete(uartPort_);

                UartResources().release(int(uartPort_) - 1);
            }
        }
    };

    Uart::Uart(Pin tx, Pin rx, Pin rts, String config, String userConfig) {
        Pins::PinOptionsParser configParser(config.begin(), config.end());
        Pins::PinOptionsParser userConfigParser(userConfig.begin(), userConfig.end());

        // Decide on the RX pin what to do:
        if (rx.capabilities().has(Pin::Capabilities::Native)) {
            this->_detail = new NativeUart(tx, rx, rts, configParser, userConfigParser);
        } else {
            Assert(false, "Pin is not supported for UART.");
        }
    }
}

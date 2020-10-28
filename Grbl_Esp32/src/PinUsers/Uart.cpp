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

        uart_port_t uartPort_;

    public:
        NativeUart(Pin tx, Pin rx, Pin rts, Pins::PinOptionsParser& options, Pins::PinOptionsParser& userOptions) :
            tx_(tx), rx_(rx), rts_(rts), uartPort_(UART_NUM_MAX) {
            // Validate if claiming the resources will err:
            Assert(tx.capabilities().has(Pin::Capabilities::Native | Pin::Capabilities::UART | Pin::Capabilities::Output));
            Assert(rx.capabilities().has(Pin::Capabilities::Native | Pin::Capabilities::UART | Pin::Capabilities::Input));
            Assert(rts.capabilities().has(Pin::Capabilities::Native | Pin::Capabilities::UART | Pin::Capabilities::Output));

            // Iterate options:
            uart_config_t uart_config = { 0 };
            int           bufferSize  = 128;
            for (auto opt : options) {
                // if (opt.is("baud")) { uart_config.baud_rate = opt.iValue(); }
                // if (opt.is("mode")) { mode = opt.value(); /* 8N1 etc */ }
                // if (opt.is("bufsize")) { mode = opt.iValue(); /* 128 etc */ }
                // if (opt.is("protocol")) { protocol= opt.value(); /* RS485 etc */ }
                // etc...
            }
            for (auto opt : userOptions) {
                // if (opt.is("baud")) { baudRate = opt.valueInt(); }
                // if (opt.is("mode")) { mode = opt.valueString(); /* 8N1 etc */ }
                // etc...
            }

            int uartIndex = UartResources().tryClaim();
            Assert(uartIndex >= 0);
            uartPort_ = static_cast<uart_port_t>(uartIndex + 1);

            // We have everything we need. Set it up:

            auto txn  = tx_.getNative(Pin::Capabilities::UART | Pin::Capabilities::Output);
            auto rxn  = rx_.getNative(Pin::Capabilities::UART | Pin::Capabilities::Output);
            auto rtsn = rts_.getNative(Pin::Capabilities::UART | Pin::Capabilities::Output);

            Assert(uart_param_config(uartPort_, &uart_config) == ESP_OK, "Uart parameter set failed.");
            Assert(uart_set_pin(uartPort_, txn, rxn, rtsn, UART_PIN_NO_CHANGE) == ESP_OK, "Uart parameter set failed.");
            Assert(uart_driver_install(uartPort_, bufferSize * 2, 0, 0, NULL, 0) == ESP_OK, "Uart driver install failed.");

            // TODO FIXME: We should set the UART mode somewhere better suited than here:
            if (uart_set_mode(uartPort_, UART_MODE_RS485_HALF_DUPLEX) != ESP_OK) {
                uart_driver_delete(uartPort_);

                UartResources().release(int(uartPort_) - 1);
                uartPort_ = UART_NUM_MAX;

                Assert(false, "UART set mode failed");
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

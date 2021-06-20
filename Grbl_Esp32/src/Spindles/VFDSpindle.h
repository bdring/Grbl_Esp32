#pragma once

/*
    VFDSpindle.h

    Part of Grbl_ESP32
    2020 -	Bart Dring
    2020 -  Stefan de Bruijn

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
*/
#include "Spindle.h"

#include "../Uart.h"

// #define VFD_DEBUG_MODE

namespace Spindles {
    extern Uart _uart;

    class VFD : public Spindle {
    private:
        static const int VFD_RS485_MAX_MSG_SIZE = 16;  // more than enough for a modbus message
        static const int MAX_RETRIES            = 5;   // otherwise the spindle is marked 'unresponsive'

        bool set_mode(SpindleState mode, bool critical);
        bool get_pins_and_settings();

        int _txd_pin;
        int _rxd_pin;
        int _rts_pin;

        uint32_t _current_rpm  = 0;
        bool     _task_running = false;
        bool     vfd_ok        = true;

        static QueueHandle_t vfd_cmd_queue;
        static TaskHandle_t  vfd_cmdTaskHandle;
        static void          vfd_cmd_task(void* pvParameters);

        static uint16_t ModRTU_CRC(uint8_t* buf, int msg_len);

    protected:
        struct ModbusCommand {
            bool critical;  // TODO SdB: change into `uint8_t critical : 1;`: We want more flags...

            uint8_t tx_length;
            uint8_t rx_length;
            uint8_t msg[VFD_RS485_MAX_MSG_SIZE];
        };

        // Commands:
        virtual void direction_command(SpindleState mode, ModbusCommand& data) = 0;
        virtual void set_speed_command(uint32_t rpm, ModbusCommand& data)      = 0;

        // Commands that return the status. Returns nullptr if unavailable by this VFD (default):
        using response_parser = bool (*)(const uint8_t* response, VFD* spindle);

        virtual response_parser initialization_sequence(int index, ModbusCommand& data) { return nullptr; }
        virtual response_parser get_current_rpm(ModbusCommand& data) { return nullptr; }
        virtual response_parser get_current_direction(ModbusCommand& data) { return nullptr; }
        virtual response_parser get_status_ok(ModbusCommand& data) = 0;
        virtual bool            supports_actual_rpm() const { return false; }
        virtual bool            safety_polling() const { return true; }

        // The constructor sets these
        int          _baudrate;
        Uart::Data   _dataBits;
        Uart::Stop   _stopBits;
        Uart::Parity _parity;

    public:
        VFD();
        VFD(const VFD&) = delete;
        VFD(VFD&&)      = delete;
        VFD& operator=(const VFD&) = delete;
        VFD& operator=(VFD&&) = delete;

        // TODO FIXME: Have to make these public because of the parsers.
        // Should hide them and use a member function.
        volatile uint32_t _min_rpm;
        volatile uint32_t _max_rpm;
        volatile uint32_t _sync_rpm;
        volatile bool     _syncing;

        void         init();
        void         config_message();
        void         set_state(SpindleState state, uint32_t rpm);
        SpindleState get_state();
        uint32_t     set_rpm(uint32_t rpm);
        void         stop();

        virtual ~VFD() {}
    };
}

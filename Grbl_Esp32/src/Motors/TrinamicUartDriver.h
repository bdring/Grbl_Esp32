#pragma once

/*
    TrinamicUartDriver.h

    Part of Grbl_ESP32
    2020 -	The Ant Team
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
*/

#include "Motor.h"
#include "StandardStepper.h"
#include "../Uart.h"

#include <TMCStepper.h>  // https://github.com/teemuatlut/TMCStepper

const float TMC2208_RSENSE_DEFAULT = 0.11f;
const float TMC2209_RSENSE_DEFAULT = 0.11f;

const double TRINAMIC_UART_FCLK = 12700000.0;  // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

// ==== defaults OK to define them in your machine definition ====

#ifndef TRINAMIC_UART_RUN_MODE
#    define TRINAMIC_UART_RUN_MODE TrinamicUartMode ::StealthChop
#endif

#ifndef TRINAMIC_UART_HOMING_MODE
#    define TRINAMIC_UART_HOMING_MODE TRINAMIC_UART_RUN_MODE
#endif

#ifndef TRINAMIC_UART_TOFF_DISABLE
#    define TRINAMIC_UART_TOFF_DISABLE 0
#endif

#ifndef TRINAMIC_UART_TOFF_STEALTHCHOP
#    define TRINAMIC_UART_TOFF_STEALTHCHOP 5
#endif

#ifndef TRINAMIC_UART_TOFF_COOLSTEP
#    define TRINAMIC_UART_TOFF_COOLSTEP 3
#endif

#ifndef TMC_UART
#    define TMC_UART UART_NUM_2
#endif

#ifdef LATER
#    ifndef TMC_UART_RX
#        define TMC_UART_RX UNDEFINED_PIN
#    endif

#    ifndef TMC_UART_TX
#        define TMC_UART_TX UNDEFINED_PIN
#    endif
#endif

extern Uart tmc_serial;

namespace Motors {

    enum class TrinamicUartMode : uint8_t {
        None        = 0,  // not for machine defs!
        StealthChop = 1,
        CoolStep    = 2,
        StallGuard  = 3,
    };

    class TrinamicUartDriver : public StandardStepper {
    private:
        static bool _uart_started;

        uint32_t calc_tstep(float speed, float percent);

        TMC2209Stepper*  tmcstepper;  // all other driver types are subclasses of this one
        TrinamicUartMode _homing_mode;
        uint16_t         _driver_part_number;  // example: use 2209 for TMC2209
        float            _r_sense;
        bool             _has_errors;
        bool             _disabled;

        float _run_current         = 0.25;
        float _hold_current        = 0.25;
        int   _microsteps          = 256;
        int   _stallguard          = 0;
        bool  _stallguardDebugMode = false;

        TrinamicUartMode _mode = TrinamicUartMode::None;
        bool             test();
        void             set_mode(bool isHoming);
        void             trinamic_test_response();
        void             trinamic_stepper_enable(bool enable);

        bool report_open_load(TMC2208_n ::DRV_STATUS_t status);
        bool report_short_to_ground(TMC2208_n ::DRV_STATUS_t status);
        bool report_over_temp(TMC2208_n ::DRV_STATUS_t status);
        bool report_short_to_ps(TMC2208_n ::DRV_STATUS_t status);

        uint8_t get_next_index();

        // Linked list of Trinamic driver instances, used by the
        // StallGuard reporting task. TODO: verify if this is really used/useful.
        static TrinamicUartDriver* List;
        TrinamicUartDriver*        link;
        static void                readSgTask(void*);

    protected:
        void config_message() override;

    public:
        TrinamicUartDriver(uint16_t driver_part_number) : TrinamicUartDriver(driver_part_number, get_next_index()) {}

        TrinamicUartDriver(uint16_t driver_part_number, uint8_t address);

        void init() override;
        void read_settings() override;
        bool set_homing_mode(bool is_homing) override;
        void set_disable(bool disable) override;

        void debug_message();

        bool hw_serial_init();

        uint8_t _addr;

        // Configuration handlers:
        void validate() const override { StandardStepper::validate(); }

        void handle(Configuration::HandlerBase& handler) override {
            handler.handle("r_sense", _r_sense);
            handler.handle("run_current", _run_current);
            handler.handle("hold_current", _hold_current);
            handler.handle("microsteps", _microsteps);
            handler.handle("stallguard", _stallguard);
            handler.handle("stallguardDebugMode", _stallguardDebugMode);

            StandardStepper::handle(handler);
        }

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override { return "trinamic_uart"; }
    };

    class TMC2008 : public TrinamicUartDriver {
    public:
        TMC2008() : TrinamicUartDriver(2008) {}

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override { return "tmc_2008"; }
    };

    class TMC2009 : public TrinamicUartDriver {
    public:
        TMC2009() : TrinamicUartDriver(2009) {}

        // Name of the configurable. Must match the name registered in the cpp file.
        const char* name() const override { return "tmc_2009"; }
    };
}

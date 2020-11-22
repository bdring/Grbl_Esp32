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

#include <TMCStepper.h>  // https://github.com/teemuatlut/TMCStepper

//#define TRINAMIC_MODE_STEALTHCHOP 0  // very quiet
//#define TRINAMIC_MODE_COOLSTEP 1     // everything runs cooler so higher current possible
//#define TRINAMIC_MODE_STALLGUARD 2   // coolstep plus generates stall indication

const float TMC2208_RSENSE_DEFAULT = 0.11f;
const float TMC2209_RSENSE_DEFAULT = 0.11f;

// const int NORMAL_TCOOLTHRS = 0xFFFFF;  // 20 bit is max
// const int NORMAL_THIGH     = 0;

const double TRINAMIC_UART_FCLK = 12700000.0;  // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

// ==== defaults OK to define them in your machine definition ====

#ifndef TRINAMIC_RUN_MODE
#    define TRINAMIC_RUN_MODE TrinamicUartMode ::StealthChop
#endif

#ifndef TRINAMIC_HOMING_MODE
#    define TRINAMIC_HOMING_MODE TRINAMIC_RUN_MODE
#endif

#ifndef TRINAMIC_TOFF_DISABLE
#    define TRINAMIC_TOFF_DISABLE 0
#endif

#ifndef TRINAMIC_TOFF_STEALTHCHOP
#    define TRINAMIC_TOFF_STEALTHCHOP 5
#endif

#ifndef TRINAMIC_TOFF_COOLSTEP
#    define TRINAMIC_TOFF_COOLSTEP 3
#endif

namespace Motors {

    enum class TrinamicUartMode : uint8_t {
        None        = 0,  // not for machine defs!
        StealthChop = 1,
        CoolStep    = 2,
        StallGuard  = 3,
    };

    class TrinamicUartDriver : public StandardStepper {
    public:
        TrinamicUartDriver( uint8_t axis_index, 
                            uint8_t step_pin, 
                            uint8_t dir_pin, 
                            uint8_t  disable_pin, 
                            uint16_t driver_part_number, 
                            float r_senseS, 
                            HardwareSerial *serial) :
            TrinamicUartDriver( axis_index, step_pin, dir_pin, disable_pin, driver_part_number, r_senseS, serial, get_trinamic_driver_uart_address(axis_index)) {}

        
        TrinamicUartDriver( uint8_t axis_index, 
                            uint8_t step_pin, 
                            uint8_t dir_pin, 
                            uint8_t  disable_pin, 
                            uint16_t driver_part_number, 
                            float r_senseS, 
                            HardwareSerial *serial,
                            uint8_t address);

        TrinamicUartDriver( uint8_t axis_index, 
                            gpio_num_t step_pin, 
                            uint8_t dir_pin, 
                            uint8_t  disable_pin, 
                            uint16_t driver_part_number, 
                            float r_sense, 
                            uint16_t SW_RX_pin, 
                            uint16_t SW_TX_pin) : 
            TrinamicUartDriver( axis_index, step_pin, dir_pin, disable_pin, driver_part_number, r_sense, SW_RX_pin, SW_TX_pin, get_trinamic_driver_uart_address(axis_index)) {}

        TrinamicUartDriver( uint8_t axis_index, 
                            gpio_num_t step_pin, 
                            uint8_t dir_pin, 
                            uint8_t  disable_pin, 
                            uint16_t driver_part_number, 
                            float r_sense, 
                            uint16_t SW_RX_pin, 
                            uint16_t SW_TX_pin, 
                            uint8_t addr);

        void config_message();
        void hw_serial_init();
        void sw_serial_init();
        void init();
        void set_mode();
        void read_settings();
        // void set_settings();
        void debug_message();
        bool set_homing_mode(bool is_homing) override;
        void set_disable(bool disable) override;

        uint8_t addr;
        uint16_t SW_RX_pin;
        uint16_t SW_TX_pin;


    private:
        uint32_t calc_tstep(float speed, float percent); //TODO: see if this is useful/used.

        TMC2208Stepper*  tmcstepper;  // all other driver types are subclasses of this one
        TrinamicUartMode _homing_mode;
        uint16_t         _driver_part_number; // example: use 2209 for TMC2209
        float            _r_sense;
        bool             _has_errors;
        bool             _disabled;

        TrinamicUartMode _mode = TrinamicUartMode::None;
        bool test();
        void set_mode(bool isHoming);
        void trinamic_test_response();
        void trinamic_stepper_enable(bool enable);

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
        // void config_message() override;

    };

}
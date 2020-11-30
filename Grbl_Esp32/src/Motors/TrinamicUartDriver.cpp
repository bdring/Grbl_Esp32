/*
    TrinamicUartDriverClass.cpp

    This is used for Trinamic UART controlled stepper motor drivers.

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
#include "TrinamicUartDriver.h"

#include <TMCStepper.h>

namespace Motors {

    /* HW Serial Constructor. */
    TrinamicUartDriver::TrinamicUartDriver( uint8_t axis_index, 
                                              uint8_t step_pin,
                                              uint8_t dir_pin,
                                              uint8_t  disable_pin,
                                              uint16_t driver_part_number,
                                              float r_sense, 
                                              HardwareSerial *serial, 
                                              uint8_t addr): 
        StandardStepper(axis_index, step_pin, dir_pin, disable_pin) {
        _driver_part_number = driver_part_number;
        _has_errors = false;
        _r_sense = r_sense;
        this->addr = addr;
        serial->begin(115200, SERIAL_8N1, -1 , -1);
        serial->setRxBufferSize(128);
        hw_serial_init();
    }

    /* SW Serial Constructor. */
    TrinamicUartDriver :: TrinamicUartDriver( uint8_t axis_index, 
                                            gpio_num_t step_pin,
                                            uint8_t dir_pin,
                                            uint8_t  disable_pin,
                                            uint16_t driver_part_number,
                                            float r_sense, 
                                            uint16_t SW_RX_pin,
                                            uint16_t SW_TX_pin,
                                            uint8_t addr):
        StandardStepper(axis_index, step_pin, dir_pin, disable_pin) {
        _driver_part_number = driver_part_number;
        _has_errors = false;
        _r_sense = r_sense;
        this->SW_RX_pin = SW_RX_pin;
        this->SW_TX_pin = SW_TX_pin;
        this->addr = addr;
        sw_serial_init();
    }

    void TrinamicUartDriver::hw_serial_init() {
        if (_driver_part_number == 2208)
            // TMC 2208 does not use address, this field is 0, differently from 2209
            tmcstepper = new TMC2208Stepper(&Serial2, _r_sense);
        else if (_driver_part_number == 2209)
            tmcstepper = new TMC2209Stepper(&Serial2, _r_sense, addr);
        else {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Trinamic Uart unsupported p/n:%d", _driver_part_number);
            return;
        }
    }
    void TrinamicUartDriver::sw_serial_init() {
    #ifdef SW_SERIAL_MOTORS //TODO: this ifdef is added to avoid error because ESP32 is not define as SW CAPABLE PLATFORM. Alternatively it can be defined as such.
        if (_driver_part_number == 2208)
            // TMC 2208 does not use address, this field is 0, differently from 2209
            tmcstepper = new TMC2208Stepper(SW_RX_pin, SW_TX_pin, _r_sense);
        else if (_driver_part_number == 2209)
            // tmcstepper = new TMC2209Stepper(sw_serial, _r_sense, addr);
            tmcstepper = new TMC2209Stepper(SW_RX_pin, SW_TX_pin, _r_sense, addr);
        else {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Trinamic Uart unsupported p/n:%d", _driver_part_number);
            return;
        }
        
        // Set PDN DISABLE to ensure UART pin is available to use.
        tmcstepper->beginSerial(57600);
        tmcstepper->pdn_disable(true);
        tmcstepper->senddelay(15);
    #endif
    }


    void TrinamicUartDriver :: init() {
        if (_has_errors) {
            return;
        }
        config_message();

        tmcstepper->begin();

        _has_errors = !test();  // Try communicating with motor. Prints an error if there is a problem.
        
        /* If communication with the driver is working, read the 
        main settings, apply new driver settings and then read 
        them back. */
        if(!_has_errors) {  //TODO: verify if this is the right way to set the Irun/IHold and microsteps.
            read_settings();
            set_mode(false);
            // set_settings();
            // read_settings();
        }
        else {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Driver %c is off, cannot set motor.", reportAxisNameMsg(_axis_index, _dual_axis_index));//report_get_axis_letter(axis_index));
        }
    }

    /*
        This is the startup message showing the basic definition. 
    */
    void TrinamicUartDriver::config_message() { //TODO: The RX/TX pin could be added to the msg.
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Trinamic TMC%d Step:%s Dir:%s Disable:%s Address:%d R:%0.3f %s",
                       reportAxisNameMsg(_axis_index, _dual_axis_index),
                       _driver_part_number,
                       pinName(_step_pin).c_str(),
                       pinName(_dir_pin).c_str(),
                       pinName(_disable_pin).c_str(),
                       this->addr,
                       _r_sense,
                       reportAxisLimitsMsg(_axis_index));
    }

    
    bool TrinamicUartDriver::test() {
        if (_has_errors) {
            return false;
        }
        switch (tmcstepper->test_connection()) {
            case 1:
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "%s Trinamic driver test failed. Check connection",
                               reportAxisNameMsg(_axis_index, _dual_axis_index));
                return false;
            case 2:
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "%s Trinamic driver test failed. Check motor power",
                               reportAxisNameMsg(_axis_index, _dual_axis_index));
                return false;
            default:
                // driver responded, so check for other errors from the DRV_STATUS register

                TMC2208_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
                status.sr = tmcstepper->DRV_STATUS();

                bool err = false;

                // look for errors
                if (report_short_to_ground(status)) {
                    err = true;
                }

                if (report_over_temp(status)) {
                    err = true;
                }

                if (report_short_to_ps(status)) {
                    err = true;
                }

                if (err) {
                    return false;
                }

                grbl_msg_sendf(
                    CLIENT_SERIAL, MsgLevel::Info, "%s Trinamic driver test passed", reportAxisNameMsg(_axis_index, _dual_axis_index));
                return true;
        }
    }

    /*
    Read setting and send them to the driver. Called at init() and whenever related settings change
    both are stored as float Amps, but TMCStepper library expects...
    uint16_t run (mA)
    float hold (as a percentage of run)
    */
    void TrinamicUartDriver::read_settings() {
        if (_has_errors) {
            return;
        }

        uint16_t run_i_ma = (uint16_t)(axis_settings[_axis_index]->run_current->get() * 1000.0);
        float    hold_i_percent;

        if (axis_settings[_axis_index]->run_current->get() == 0)
            hold_i_percent = 0;
        else {
            hold_i_percent = axis_settings[_axis_index]->hold_current->get() / axis_settings[_axis_index]->run_current->get();
            if (hold_i_percent > 1.0)
                hold_i_percent = 1.0;
        }
        tmcstepper->microsteps(axis_settings[_axis_index]->microsteps->get());
        tmcstepper->rms_current(run_i_ma, hold_i_percent);
    }

    bool TrinamicUartDriver::set_homing_mode(bool isHoming) {
        set_mode(isHoming);
        return true;
    }

    /*
    There are ton of settings. I'll start by grouping then into modes for now.
    Many people will want quiet and stallgaurd homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing
    */
    void TrinamicUartDriver::set_mode(bool isHoming) {
        if (_has_errors) {
            return;
        }

        TrinamicUartMode newMode = isHoming ? TRINAMIC_UART_HOMING_MODE : TRINAMIC_UART_RUN_MODE;

        if (newMode == _mode) {
            return;
        }
        _mode = newMode;

        switch (_mode) {
            case TrinamicUartMode ::StealthChop:
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "StealthChop");
                // tmcstepper->en_pwm_mode(true); //TODO: check if this is present in TMC2208/09
                tmcstepper->en_spreadCycle(false);
                tmcstepper->pwm_autoscale(true);
                // if (_driver_part_number == 2209) {
                    // tmcstepper->diag1_stall(false); //TODO: check the equivalent in TMC2209
                // }
                break;
            case TrinamicUartMode ::CoolStep:
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Coolstep");
                // tmcstepper->en_pwm_mode(false); //TODO: check if this is present in TMC2208/09
                tmcstepper->en_spreadCycle(true);
                tmcstepper->pwm_autoscale(false);
                if (_driver_part_number == 2209) {
                    // tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS);  // when to turn on coolstep //TODO: add this solving compilation issue.
                    // tmcstepper->THIGH(NORMAL_THIGH); //TODO: this does not exist in TMC2208/09. verify and eventually remove.
                }
                break;
            case TrinamicUartMode ::StallGuard: //TODO: check all configurations for stallguard
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Stallguard");
                // tmcstepper->en_pwm_mode(false); //TODO: check if this is present in TMC2208/09
                tmcstepper->en_spreadCycle(false);
                tmcstepper->pwm_autoscale(false);
                // tmcstepper->TCOOLTHRS(calc_tstep(homing_feed_rate->get(), 150.0));
                // tmcstepper->THIGH(calc_tstep(homing_feed_rate->get(), 60.0));
                // tmcstepper->sfilt(1);
                // tmcstepper->diag1_stall(true);  // stallguard i/o is on diag1
                // tmcstepper->sgt(axis_settings[_axis_index]->stallguard->get());
                break;
            default:
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "TRINAMIC_MODE_UNDEFINED");
        }
    }

    /*
    This is the stallguard tuning info. It is call debug, so it could be generic across all classes.
    */
    void TrinamicUartDriver::debug_message() {
        if (_has_errors) {
            return;
        }
        uint32_t tstep = tmcstepper->TSTEP();

        if (tstep == 0xFFFFF || tstep < 1) {  // if axis is not moving return
            return;
        }
        float feedrate = st_get_realtime_rate();  //* settings.microsteps[axis_index] / 60.0 ; // convert mm/min to Hz

        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Stallguard %d   SG_Val: %04d   Rate: %05.0f mm/min SG_Setting:%d",
                       reportAxisNameMsg(_axis_index, _dual_axis_index),
                    0,//    tmcstepper->stallguard(), // TODO: add this again solving the compilation issues
                    0,//    tmcstepper->sg_result(),
                       feedrate,
                       axis_settings[_axis_index]->stallguard->get());

        TMC2208_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
        status.sr = tmcstepper->DRV_STATUS();

        // these only report if there is a fault condition
        report_open_load(status);
        report_short_to_ground(status);
        report_over_temp(status);
        report_short_to_ps(status);

        // grbl_msg_sendf(CLIENT_SERIAL,
        //                MsgLevel::Info,
        //                "%s Status Register %08x GSTAT %02x",
        //                reportAxisNameMsg(_axis_index, _dual_axis_index),
        //                status.sr,
        //                tmcstepper->GSTAT());
    }


    // calculate a tstep from a rate
    // tstep = TRINAMIC_UART_FCLK / (time between 1/256 steps)
    // This is used to set the stallguard window from the homing speed.
    // The percent is the offset on the window
    uint32_t TrinamicUartDriver::calc_tstep(float speed, float percent) {
        float tstep =
            speed / 60.0 * axis_settings[_axis_index]->steps_per_mm->get() * (float)(256 / axis_settings[_axis_index]->microsteps->get());
        tstep = TRINAMIC_UART_FCLK / tstep * percent / 100.0;

        return static_cast<uint32_t>(tstep);
    }

    // this can use the enable feature over SPI. The dedicated pin must be in the enable mode,
    // but that can be hardwired that way.
    void TrinamicUartDriver::set_disable(bool disable) {
        if (_has_errors) {
            return;
        }

        if (_disabled == disable) {
            return;
        }

        _disabled = disable;

        digitalWrite(_disable_pin, _disabled);

#ifdef USE_TRINAMIC_ENABLE
        if (_disabled) {
            tmcstepper->toff(TRINAMIC_UART_TOFF_DISABLE);
        } else {
            if (_mode == TrinamicUartMode::StealthChop) {
                tmcstepper->toff(TRINAMIC_UART_TOFF_STEALTHCHOP);
            } else {
                tmcstepper->toff(TRINAMIC_UART_TOFF_COOLSTEP);
            }
        }
#endif
        // the pin based enable could be added here.
        // This would be for individual motors, not the single pin for all motors.
    }


    // =========== Reporting functions ========================

    bool TrinamicUartDriver::report_open_load(TMC2208_n ::DRV_STATUS_t status) {
        if (status.ola || status.olb) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Open Load a:%s b:%s",
                           reportAxisNameMsg(_axis_index, _dual_axis_index),
                           status.ola ? "Y" : "N",
                           status.olb ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    bool TrinamicUartDriver::report_short_to_ground(TMC2208_n ::DRV_STATUS_t status) {
        if (status.s2ga || status.s2gb) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Short Coil a:%s b:%s",
                           reportAxisNameMsg(_axis_index, _dual_axis_index),
                           status.s2ga ? "Y" : "N",
                           status.s2gb ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    bool TrinamicUartDriver::report_over_temp(TMC2208_n ::DRV_STATUS_t status) {
        if (status.ot || status.otpw) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Temp Warning:%s Fault:%s",
                           reportAxisNameMsg(_axis_index, _dual_axis_index),
                           status.otpw ? "Y" : "N",
                           status.ot ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    bool TrinamicUartDriver::report_short_to_ps(TMC2208_n ::DRV_STATUS_t status) {
        // check for short to power supply
        if ((status.sr & bit(12)) || (status.sr & bit(13))) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Short vsa:%s vsb:%s",
                           reportAxisNameMsg(_axis_index, _dual_axis_index),
                           (status.sr & bit(12)) ? "Y" : "N",
                           (status.sr & bit(13)) ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

}
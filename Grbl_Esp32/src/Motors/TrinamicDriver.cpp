/*
    TrinamicDriver.cpp
    This is used for Trinamic SPI controlled stepper motor drivers.

    Part of Grbl_ESP32
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
#include "TrinamicDriver.h"

#include <TMCStepper.h>

namespace Motors {
    TrinamicDriver::TrinamicDriver(uint8_t  axis_index,
                                   uint8_t  step_pin,
                                   uint8_t  dir_pin,
                                   uint8_t  disable_pin,
                                   uint8_t  cs_pin,
                                   uint16_t driver_part_number,
                                   float    r_sense,
                                   int8_t   spi_index) {
        type_id               = TRINAMIC_SPI_MOTOR;
        this->axis_index      = axis_index % MAX_AXES;
        this->dual_axis_index = axis_index < 6 ? 0 : 1;  // 0 = primary 1 = ganged
        _driver_part_number   = driver_part_number;
        _r_sense              = r_sense;
        this->step_pin        = step_pin;
        this->dir_pin         = dir_pin;
        this->disable_pin     = disable_pin;
        this->cs_pin          = cs_pin;
        this->spi_index       = spi_index;

        _homing_mode = TRINAMIC_HOMING_MODE;
        _homing_mask = 0;  // no axes homing

        if (_driver_part_number == 2130)
            tmcstepper = new TMC2130Stepper(cs_pin, _r_sense, spi_index);
        else if (_driver_part_number == 5160)
            tmcstepper = new TMC5160Stepper(cs_pin, _r_sense, spi_index);
        else {
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic unsupported p/n:%d", _driver_part_number);
            return;
        }

        set_axis_name();

        init_step_dir_pins();  // from StandardStepper

        digitalWrite(cs_pin, HIGH);
        pinMode(cs_pin, OUTPUT);

        // use slower speed if I2S
        if (cs_pin >= I2S_OUT_PIN_BASE)
            tmcstepper->setSPISpeed(TRINAMIC_SPI_FREQ);

        config_message();

        // init() must be called later, after all TMC drivers have CS pins setup.
    }

    void TrinamicDriver::init() {
        SPI.begin();  // this will get called for each motor, but does not seem to hurt anything

        tmcstepper->begin();
        test();           // Try communicating with motor. Prints an error if there is a problem.
        read_settings();  // pull info from settings
        set_mode(false);

        _homing_mask = 0;
        is_active    = true;  // as opposed to NullMotors, this is a real motor
    }

    /*
    This is the startup message showing the basic definition
*/
    void TrinamicDriver::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MSG_LEVEL_INFO,
                       "%s Axis Trinamic TMC%d Step:%s Dir:%s CS:%s Disable:%s Index:%d",
                       _axis_name,
                       _driver_part_number,
                       pinName(step_pin).c_str(),
                       pinName(dir_pin).c_str(),
                       pinName(cs_pin).c_str(),
                       pinName(disable_pin).c_str(),
                       spi_index);
    }

    bool TrinamicDriver::test() {
        switch (tmcstepper->test_connection()) {
            case 1:
                grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Trinamic driver test failed. Check connection", _axis_name);
                return false;
            case 2:
                grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Trinamic driver test failed. Check motor power", _axis_name);
                return false;
            default:
                // driver responded, so check for other errors from the DRV_STATUS register

                TMC2130_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
                status.sr = tmcstepper->DRV_STATUS();

                bool err = false;
                // look for open loan or short 2 ground on a and b
                if (status.s2ga || status.s2gb) {
                    grbl_msg_sendf(CLIENT_SERIAL,
                                   MSG_LEVEL_INFO,
                                   "%s Motor Short Coil a:%s b:%s",
                                   _axis_name,
                                   status.s2ga ? "Y" : "N",
                                   status.s2gb ? "Y" : "N");
                    err = true;
                }
                // check for over temp or pre-warning
                if (status.ot || status.otpw) {
                    grbl_msg_sendf(CLIENT_SERIAL,
                                   MSG_LEVEL_INFO,
                                   "%s Driver Temp Warning:%s Fault:%s",
                                   _axis_name,
                                   status.otpw ? "Y" : "N",
                                   status.ot ? "Y" : "N");
                    err = true;
                }

                if (err)
                    return false;

                grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Trinamic driver test passed", _axis_name);
                return true;
        }
    }

    /*
    Read setting and send them to the driver. Called at init() and whenever related settings change
    both are stored as float Amps, but TMCStepper library expects...
    uint16_t run (mA)
    float hold (as a percentage of run)
*/
    void TrinamicDriver::read_settings() {
        uint16_t run_i_ma = (uint16_t)(axis_settings[axis_index]->run_current->get() * 1000.0);
        float    hold_i_percent;

        if (axis_settings[axis_index]->run_current->get() == 0)
            hold_i_percent = 0;
        else {
            hold_i_percent = axis_settings[axis_index]->hold_current->get() / axis_settings[axis_index]->run_current->get();
            if (hold_i_percent > 1.0)
                hold_i_percent = 1.0;
        }
        //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Current run %d hold %f", _axis_name, run_i_ma, hold_i_percent);

        tmcstepper->microsteps(axis_settings[axis_index]->microsteps->get());
        tmcstepper->rms_current(run_i_ma, hold_i_percent);
    }

    void TrinamicDriver::set_homing_mode(uint8_t homing_mask, bool isHoming) {
        _homing_mask = homing_mask;
        set_mode(isHoming);
    }

    /*
    There are ton of settings. I'll start by grouping then into modes for now.
    Many people will want quiet and stallgaurd homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing
*/
    void TrinamicDriver::set_mode(bool isHoming) {
        if (isHoming)
            _mode = TRINAMIC_HOMING_MODE;
        else
            _mode = TRINAMIC_RUN_MODE;

        if (_lastMode == _mode)
            return;
        _lastMode = _mode;

        switch (_mode) {
            case TRINAMIC_MODE_STEALTHCHOP:
                //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TRINAMIC_MODE_STEALTHCHOP");
                tmcstepper->en_pwm_mode(true);
                tmcstepper->pwm_autoscale(true);
                tmcstepper->diag1_stall(false);
                break;
            case TRINAMIC_MODE_COOLSTEP:
                //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TRINAMIC_MODE_COOLSTEP");
                tmcstepper->en_pwm_mode(false);
                tmcstepper->pwm_autoscale(false);
                tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS);  // when to turn on coolstep
                tmcstepper->THIGH(NORMAL_THIGH);
                break;
            case TRINAMIC_MODE_STALLGUARD:
                //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TRINAMIC_MODE_STALLGUARD");
                tmcstepper->en_pwm_mode(false);
                tmcstepper->pwm_autoscale(false);
                tmcstepper->TCOOLTHRS(calc_tstep(homing_feed_rate->get(), 150.0));
                tmcstepper->THIGH(calc_tstep(homing_feed_rate->get(), 60.0));
                tmcstepper->sfilt(1);
                tmcstepper->diag1_stall(true);  // stallguard i/o is on diag1
                tmcstepper->sgt(axis_settings[axis_index]->stallguard->get());
                break;
            default: grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TRINAMIC_MODE_UNDEFINED");
        }
    }

    /*
    This is the stallguard tuning info. It is call debug, so it could be generic across all classes.
*/
    void TrinamicDriver::debug_message() {
        uint32_t tstep = tmcstepper->TSTEP();

        if (tstep == 0xFFFFF || tstep < 1)  // if axis is not moving return
            return;
        float feedrate = st_get_realtime_rate();  //* settings.microsteps[axis_index] / 60.0 ; // convert mm/min to Hz

        grbl_msg_sendf(CLIENT_SERIAL,
                       MSG_LEVEL_INFO,
                       "%s Stallguard %d   SG_Val: %04d   Rate: %05.0f mm/min SG_Setting:%d",
                       _axis_name,
                       tmcstepper->stallguard(),
                       tmcstepper->sg_result(),
                       feedrate,
                       axis_settings[axis_index]->stallguard->get());
    }

    // calculate a tstep from a rate
    // tstep = TRINAMIC_FCLK / (time between 1/256 steps)
    // This is used to set the stallguard window from the homing speed.
    // The percent is the offset on the window
    uint32_t TrinamicDriver::calc_tstep(float speed, float percent) {
        float tstep =
            speed / 60.0 * axis_settings[axis_index]->steps_per_mm->get() * (float)(256 / axis_settings[axis_index]->microsteps->get());
        tstep = TRINAMIC_FCLK / tstep * percent / 100.0;

        return (uint32_t)tstep;
    }

    // this can use the enable feature over SPI. The dedicated pin must be in the enable mode,
    // but that can be hardwired that way.
    void TrinamicDriver::set_disable(bool disable) {
        //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%s Axis disable %d", _axis_name, disable);

        digitalWrite(disable_pin, disable);

#ifdef USE_TRINAMIC_ENABLE
        if (disable)
            tmcstepper->toff(TRINAMIC_TOFF_DISABLE);
        else {
            if (_mode == TRINAMIC_MODE_STEALTHCHOP)
                tmcstepper->toff(TRINAMIC_TOFF_STEALTHCHOP);
            else
                tmcstepper->toff(TRINAMIC_TOFF_COOLSTEP);
        }
#endif
        // the pin based enable could be added here.
        // This would be for individual motors, not the single pin for all motors.
    }
}

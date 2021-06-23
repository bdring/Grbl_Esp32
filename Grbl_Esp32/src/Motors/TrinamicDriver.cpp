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

#include "../Machine/MachineConfig.h"
#include "../Report.h"

#include <TMCStepper.h>  // https://github.com/teemuatlut/TMCStepper
#include <atomic>

// This is global so it can be accessed from the override
namespace Motors {
    namespace Details {
        // How this hack works... In short, we just derive from the TMCStepper classes, and add the pin as a field. This is
        // fine of course, as long as we know what CS pin we have to switch. Unfortunately, this is a bit tougher, because
        // the TMC5160Stepper class derives from TMC2130Stepper. Normally, this is solved through multiple inheritance, where
        // we put the pin in the second class. Unfortunately, that requires dynamic_cast down the road, which is not available.
        // So instead, we use the old _pinCS in the TMC2130Stepper class to figure out which class we actually have, and cast
        // away with static_cast.

        class MyTMC2130Stepper : public TMC2130Stepper {
        public:
            MyTMC2130Stepper(Pin& cspin, float rsense, int spiIndex) : TMC2130Stepper(2130, rsense, spiIndex), _cspin(cspin) {}
            Pin& _cspin;
        };

        class MyTMC5160Stepper : public TMC5160Stepper {
        public:
            MyTMC5160Stepper(Pin& cspin, float rsense, int spiIndex) : TMC5160Stepper(5160, rsense, spiIndex), _cspin(cspin) {}
            Pin& _cspin;
        };
    }
}

// Override default functions to use our pin framework
void TMC2130Stepper::switchCSpin(bool state) {
    // We use the _pinCS to figure out which derived class we have
    switch (this->_pinCS) {
        case 2130:
            static_cast<Motors::Details::MyTMC2130Stepper*>(this)->_cspin.write(state);
            break;
        case 5160:
            static_cast<Motors::Details::MyTMC5160Stepper*>(this)->_cspin.write(state);
            break;
    }
}

namespace Motors {
    TrinamicDriver::TrinamicDriver(uint16_t driver_part_number, int8_t spi_index) :
        TrinamicBase(driver_part_number), _spi_index(spi_index) {}

    void TrinamicDriver::init() {
        _has_errors = false;

        if (_driver_part_number == 2130) {
            tmcstepper = new Details::MyTMC2130Stepper(_cs_pin, _r_sense, _spi_index);
        } else if (_driver_part_number == 5160) {
            tmcstepper = new Details::MyTMC5160Stepper(_cs_pin, _r_sense, _spi_index);
        } else {
            info_serial("%s Unsupported Trinamic part number TMC%d", reportAxisNameMsg(axis_index(), dual_axis_index()), _driver_part_number);
            _has_errors = true;  // This motor cannot be used
            return;
        }

        _has_errors = false;

        _cs_pin.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);

        // use slower speed if I2S
        if (_cs_pin.capabilities().has(Pin::Capabilities::I2S)) {
            tmcstepper->setSPISpeed(_spi_freq);
        }

        link = List;
        List = this;

        // init() must be called later, after all TMC drivers have CS pins setup.
        if (_has_errors) {
            return;
        }

        // Display the stepper library version message once, before the first
        // TMC config message.  Link is NULL for the first TMC instance.
        if (!link) {
            info_serial("TMCStepper Library Ver. 0x%06x", TMCSTEPPER_VERSION);
        }

        config_message();

        auto spiConfig = config->_spi;
        if (spiConfig != nullptr) {
            auto ssPin   = _cs_pin.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
            auto mosiPin = spiConfig->_mosi.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
            auto sckPin  = spiConfig->_sck.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
            auto misoPin = spiConfig->_miso.getNative(Pin::Capabilities::Input | Pin::Capabilities::Native);

            SPI.begin(sckPin, misoPin, mosiPin, ssPin);  // this will get called for each motor, but does not seem to hurt anything

            tmcstepper->begin();

            _has_errors = !test();  // Try communicating with motor. Prints an error if there is a problem.

            read_settings();  // pull info from settings
            set_mode(false);

            // After initializing all of the TMC drivers, create a task to
            // display StallGuard data.  List == this for the final instance.
            if (List == this) {
                xTaskCreatePinnedToCore(readSgTask,    // task
                                        "readSgTask",  // name for task
                                        4096,          // size of task stack
                                        this,          // parameters
                                        1,             // priority
                                        NULL,
                                        SUPPORT_TASK_CORE  // must run the task on same core
                );
            }
        } else {
            info_serial("SPI bus is not available; cannot initialize TMC driver.");
            _has_errors = true;
        }
    }

    /*
    This is the startup message showing the basic definition
    */
    void TrinamicDriver::config_message() {
        info_serial("%s Trinamic TMC%d Step:%s Dir:%s CS:%s Disable:%s Index:%d R:%0.3f %s",
                    reportAxisNameMsg(axis_index(), dual_axis_index()),
                    _driver_part_number,
                    _step_pin.name().c_str(),
                    _dir_pin.name().c_str(),
                    _cs_pin.name().c_str(),
                    _disable_pin.name().c_str(),
                    _spi_index,
                    _r_sense,
                    reportAxisLimitsMsg(axis_index()));
    }

    bool TrinamicDriver::test() {
        if (_has_errors) {
            return false;
        }

        switch (tmcstepper->test_connection()) {
            case 1:
                info_serial("%s Trinamic driver test failed. Check connection", reportAxisNameMsg(axis_index(), dual_axis_index()));
                return false;
            case 2:
                info_serial("%s Trinamic driver test failed. Check motor power", reportAxisNameMsg(axis_index(), dual_axis_index()));
                return false;
            default:
                // driver responded, so check for other errors from the DRV_STATUS register

                TMC2130_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
                status.sr = tmcstepper->DRV_STATUS();

                bool err = false;

                // look for errors
                if (report_short_to_ground(status.s2ga, status.s2gb)) {
                    err = true;
                }

                if (report_over_temp(status.ot, status.otpw)) {
                    err = true;
                }

                if (report_short_to_ps(bit_istrue(status.sr, 12), bit_istrue(status.sr, 13))) {
                    err = true;
                }

                // XXX why not report_open_load(status.ola, status.olb) ?

                if (err) {
                    return false;
                }

                info_serial("%s Trinamic driver test passed", reportAxisNameMsg(axis_index(), dual_axis_index()));
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
        if (_has_errors) {
            return;
        }

        uint16_t run_i_ma = (uint16_t)(_run_current * 1000.0);
        float    hold_i_percent;

        if (_run_current == 0) {
            hold_i_percent = 0;
        } else {
            hold_i_percent = _hold_current / _run_current;
            if (hold_i_percent > 1.0) {
                hold_i_percent = 1.0;
            }
        }

        tmcstepper->microsteps(_microsteps);
        tmcstepper->rms_current(run_i_ma, hold_i_percent);

        init_step_dir_pins();
    }

    bool TrinamicDriver::set_homing_mode(bool isHoming) {
        set_mode(isHoming);
        return true;
    }

    /*
    There are ton of settings. I'll start by grouping then into modes for now.
    Many people will want quiet and stallguard homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing
    */
    void TrinamicDriver::set_mode(bool isHoming) {
        if (_has_errors) {
            return;
        }

        TrinamicMode newMode = static_cast<TrinamicMode>(trinamicModes[isHoming ? _homing_mode : _run_mode].value);

        if (newMode == _mode) {
            return;
        }
        _mode = newMode;

        switch (_mode) {
            case TrinamicMode ::StealthChop:
                //info_serial("StealthChop");
                tmcstepper->en_pwm_mode(true);
                tmcstepper->pwm_autoscale(true);
                tmcstepper->diag1_stall(false);
                break;
            case TrinamicMode ::CoolStep:
                //info_serial("Coolstep");
                tmcstepper->en_pwm_mode(false);
                tmcstepper->pwm_autoscale(false);
                tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS);  // when to turn on coolstep
                tmcstepper->THIGH(NORMAL_THIGH);
                break;
            case TrinamicMode ::StallGuard:
                //info_serial("Stallguard");
                {
                    auto feedrate = config->_axes->_axis[axis_index()]->_homing->_feedRate;

                    tmcstepper->en_pwm_mode(false);
                    tmcstepper->pwm_autoscale(false);
                    tmcstepper->TCOOLTHRS(calc_tstep(feedrate, 150.0));
                    tmcstepper->THIGH(calc_tstep(feedrate, 60.0));
                    tmcstepper->sfilt(1);
                    tmcstepper->diag1_stall(true);  // stallguard i/o is on diag1
                    tmcstepper->sgt(constrain(_stallguard, -64, 63));
                    break;
                }
        }
    }

    /*
    This is the stallguard tuning info. It is call debug, so it could be generic across all classes.
*/
    void TrinamicDriver::debug_message() {
        if (_has_errors) {
            return;
        }

        uint32_t tstep = tmcstepper->TSTEP();

        if (tstep == 0xFFFFF || tstep < 1) {  // if axis is not moving return
            return;
        }
        float feedrate = st_get_realtime_rate();  //* settings.microsteps[axis_index] / 60.0 ; // convert mm/min to Hz

        info_serial("%s Stallguard %d   SG_Val: %04d   Rate: %05.0f mm/min SG_Setting:%d",
                    reportAxisNameMsg(axis_index(), dual_axis_index()),
                    tmcstepper->stallguard(),
                    tmcstepper->sg_result(),
                    feedrate,
                    constrain(_stallguard, -64, 63));

        // The bit locations differ somewhat between different chips.
        // The layout is very different between 2130 and 2208
        TMC2130_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
        status.sr = tmcstepper->DRV_STATUS();

        // these only report if there is a fault condition
        report_open_load(status.ola, status.olb);
        report_short_to_ground(status.s2ga, status.s2gb);
        report_over_temp(status.ot, status.otpw);
        report_short_to_ps(bit_istrue(status.sr, 12), bit_istrue(status.sr, 13));

        // info_serial("%s Status Register %08x GSTAT %02x",
        //             reportAxisNameMsg(axis_index(), dual_axis_index()),
        //             status.sr,
        //             tmcstepper->GSTAT());
    }

    // this can use the enable feature over SPI. The dedicated pin must be in the enable mode,
    // but that can be hardwired that way.
    void TrinamicDriver::set_disable(bool disable) {
        if (_has_errors) {
            return;
        }

        if (_disabled == disable) {
            return;
        }

        _disabled = disable;

        _disable_pin.write(_disabled);

        if (_use_enable) {
            if (_disabled) {
                tmcstepper->toff(_toff_disable);
            } else {
                if (_mode == TrinamicMode::StealthChop) {
                    tmcstepper->toff(_toff_stealthchop);
                } else {
                    tmcstepper->toff(_toff_coolstep);
                }
            }
        }
        // the pin based enable could be added here.
        // This would be for individual motors, not the single pin for all motors.
    }

    // Configuration registration
    namespace {
        MotorFactory::InstanceBuilder<TMC2130> registration_2130("tmc_2130");
        MotorFactory::InstanceBuilder<TMC5160> registration_5160("tmc_5160");
    }
}

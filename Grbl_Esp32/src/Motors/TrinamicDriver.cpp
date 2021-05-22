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
#include "../MachineConfig.h"

#include <TMCStepper.h>

#ifdef USE_I2S_OUT

// Override default function and insert a short delay
void TMC2130Stepper::switchCSpin(bool state) {
    digitalWrite(_pinCS, state);
    i2s_out_delay();
}
#endif

namespace Motors {
    uint8_t TrinamicDriver::get_next_index() {
#ifdef TRINAMIC_DAISY_CHAIN
        static uint8_t index = 1;  // they start at 1
        return index++;
#else
        return -1;
#endif
    }
    TrinamicDriver* TrinamicDriver::List = NULL;

    TrinamicDriver::TrinamicDriver(uint16_t driver_part_number, int8_t spi_index) :
        StandardStepper(), _homing_mode(TRINAMIC_HOMING_MODE), _driver_part_number(driver_part_number), _spi_index(spi_index) {}

    void TrinamicDriver::init() {
        _has_errors = false;

        auto cs_pin_native = _cs_pin.getNative(Pin::Capabilities::Output);

        if (_driver_part_number == 2130) {
            tmcstepper = new TMC2130Stepper(cs_pin_native, _r_sense, _spi_index);
        } else if (_driver_part_number == 5160) {
            tmcstepper = new TMC5160Stepper(cs_pin_native, _r_sense, _spi_index);
        } else {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Unsupported Trinamic part number TMC%d",
                           reportAxisNameMsg(axis_index(), dual_axis_index()),
                           _driver_part_number);
            _has_errors = true;  // This motor cannot be used
            return;
        }

        _has_errors = false;

        _cs_pin.setAttr(Pin::Attr::Output | Pin::Attr::InitialOn);

        // use slower speed if I2S
        if (_cs_pin.capabilities().has(Pin::Capabilities::I2S)) {
            tmcstepper->setSPISpeed(TRINAMIC_SPI_FREQ);
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
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "TMCStepper Library Ver. 0x%06x", TMCSTEPPER_VERSION);
        }

        config_message();

        auto spiConfig = config->_spi;
        if (spiConfig != nullptr) {
            auto ssPin   = spiConfig->_ss.getNative(Pin::Capabilities::Output | Pin::Capabilities::Native);
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
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "SPI bus is not available; cannot initialize TMC driver.");
            _has_errors = true;
        }
    }

    /*
    This is the startup message showing the basic definition
    */
    void TrinamicDriver::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Trinamic TMC%d Step:%s Dir:%s CS:%s Disable:%s Index:%d R:%0.3f %s",
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
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "%s Trinamic driver test failed. Check connection",
                               reportAxisNameMsg(axis_index(), dual_axis_index()));
                return false;
            case 2:
                grbl_msg_sendf(CLIENT_SERIAL,
                               MsgLevel::Info,
                               "%s Trinamic driver test failed. Check motor power",
                               reportAxisNameMsg(axis_index(), dual_axis_index()));
                return false;
            default:
                // driver responded, so check for other errors from the DRV_STATUS register

                TMC2130_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
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
                    CLIENT_SERIAL, MsgLevel::Info, "%s Trinamic driver test passed", reportAxisNameMsg(axis_index(), dual_axis_index()));
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
            if (hold_i_percent > 1.0)
                hold_i_percent = 1.0;
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
    Many people will want quiet and stallgaurd homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing
    */
    void TrinamicDriver::set_mode(bool isHoming) {
        if (_has_errors) {
            return;
        }

        TrinamicMode newMode = isHoming ? TRINAMIC_HOMING_MODE : TRINAMIC_RUN_MODE;

        if (newMode == _mode) {
            return;
        }
        _mode = newMode;

        switch (_mode) {
            case TrinamicMode ::StealthChop:
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "StealthChop");
                tmcstepper->en_pwm_mode(true);
                tmcstepper->pwm_autoscale(true);
                tmcstepper->diag1_stall(false);
                break;
            case TrinamicMode ::CoolStep:
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Coolstep");
                tmcstepper->en_pwm_mode(false);
                tmcstepper->pwm_autoscale(false);
                tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS);  // when to turn on coolstep
                tmcstepper->THIGH(NORMAL_THIGH);
                break;
            case TrinamicMode ::StallGuard:
                //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Stallguard");
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
            default:
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "TRINAMIC_MODE_UNDEFINED");
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

        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "%s Stallguard %d   SG_Val: %04d   Rate: %05.0f mm/min SG_Setting:%d",
                       reportAxisNameMsg(axis_index(), dual_axis_index()),
                       tmcstepper->stallguard(),
                       tmcstepper->sg_result(),
                       feedrate,
                       constrain(_stallguard, -64, 63));

        TMC2130_n ::DRV_STATUS_t status { 0 };  // a useful struct to access the bits.
        status.sr = tmcstepper->DRV_STATUS();

        // these only report if there is a fault condition
        report_open_load(status);
        report_short_to_ground(status);
        report_over_temp(status);
        report_short_to_ps(status);

        // grbl_msg_sendf(CLIENT_SERIAL,
        //                MsgLevel::Info,
        //                "%s Status Register %08x GSTAT %02x",
        //                reportAxisNameMsg(axis_index(), dual_axis_index()),
        //                status.sr,
        //                tmcstepper->GSTAT());
    }

    // calculate a tstep from a rate
    // tstep = TRINAMIC_FCLK / (time between 1/256 steps)
    // This is used to set the stallguard window from the homing speed.
    // The percent is the offset on the window
    uint32_t TrinamicDriver::calc_tstep(float speed, float percent) {
        double tstep = speed / 60.0 * config->_axes->_axis[axis_index()]->_stepsPerMm * (256.0 / _microsteps);
        tstep        = TRINAMIC_FCLK / tstep * percent / 100.0;

        return static_cast<uint32_t>(tstep);
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

#ifdef USE_TRINAMIC_ENABLE
        if (_disabled) {
            tmcstepper->toff(TRINAMIC_TOFF_DISABLE);
        } else {
            if (_mode == TrinamicMode::StealthChop) {
                tmcstepper->toff(TRINAMIC_TOFF_STEALTHCHOP);
            } else {
                tmcstepper->toff(TRINAMIC_TOFF_COOLSTEP);
            }
        }
#endif
        // the pin based enable could be added here.
        // This would be for individual motors, not the single pin for all motors.
    }

    // Prints StallGuard data that is useful for tuning.
    void TrinamicDriver::readSgTask(void* pvParameters) {
        auto trinamicDriver = static_cast<TrinamicDriver*>(pvParameters);

        TickType_t       xLastWakeTime;
        const TickType_t xreadSg = 200;  // in ticks (typically ms)
        auto             n_axis  = config->_axes->_numberAxis;

        xLastWakeTime = xTaskGetTickCount();  // Initialise the xLastWakeTime variable with the current time.
        while (true) {                        // don't ever return from this or the task dies
            if (sys.state == State::Cycle || sys.state == State::Homing || sys.state == State::Jog) {
                for (TrinamicDriver* p = List; p; p = p->link) {
                    if (p->_stallguardDebugMode) {
                        //grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "SG:%d", p->_stallguardDebugMode);
                        p->debug_message();
                    }
                }
            }  // sys.state

            vTaskDelayUntil(&xLastWakeTime, xreadSg);

            static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
            reportTaskStackSize(uxHighWaterMark);
#endif
        }
    }

    // =========== Reporting functions ========================

    bool TrinamicDriver::report_open_load(TMC2130_n ::DRV_STATUS_t status) {
        if (status.ola || status.olb) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Open Load a:%s b:%s",
                           reportAxisNameMsg(axis_index(), dual_axis_index()),
                           status.ola ? "Y" : "N",
                           status.olb ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    bool TrinamicDriver::report_short_to_ground(TMC2130_n ::DRV_STATUS_t status) {
        if (status.s2ga || status.s2gb) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Short Coil a:%s b:%s",
                           reportAxisNameMsg(axis_index(), dual_axis_index()),
                           status.s2ga ? "Y" : "N",
                           status.s2gb ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    bool TrinamicDriver::report_over_temp(TMC2130_n ::DRV_STATUS_t status) {
        if (status.ot || status.otpw) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Temp Warning:%s Fault:%s",
                           reportAxisNameMsg(axis_index(), dual_axis_index()),
                           status.otpw ? "Y" : "N",
                           status.ot ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    bool TrinamicDriver::report_short_to_ps(TMC2130_n ::DRV_STATUS_t status) {
        // check for short to power supply
        if ((status.sr & bit(12)) || (status.sr & bit(13))) {
            grbl_msg_sendf(CLIENT_SERIAL,
                           MsgLevel::Info,
                           "%s Driver Short vsa:%s vsb:%s",
                           reportAxisNameMsg(axis_index(), dual_axis_index()),
                           (status.sr & bit(12)) ? "Y" : "N",
                           (status.sr & bit(13)) ? "Y" : "N");
            return true;
        }
        return false;  // no error
    }

    // Configuration registration
    namespace {
        MotorFactory::InstanceBuilder<TMC2130> registration_2130("tmc_2130");
        MotorFactory::InstanceBuilder<TMC5160> registration_5160("tmc_5160");
    }
}

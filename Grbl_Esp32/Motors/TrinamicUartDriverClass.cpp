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

#include <TMCStepper.h>
#include "MotorClass.h"
// #include <ESP32SwSerial.h>

//add ESP32 to define SW_CAPABLE_PLATFORM to use SW UART
//TODO: important notes: add library ESPSoftwareSerial for ESP32 in platformio.ini.

#define TRINAMIC_FCLK       12700000.0 // Internal clock Approx (Hz) used to calculate TSTEP from homing rate

/* HW Serial Constructor. */
TrinamicUartDriver :: TrinamicUartDriver( uint8_t axis_index, 
                                          gpio_num_t step_pin,
                                          uint8_t dir_pin,
                                          uint16_t driver_part_number,
                                          float r_sense, 
                                          HardwareSerial *serial, 
                                          uint8_t addr) {
    this->axis_index = axis_index;
    this->dual_axis_index = axis_index < 6 ? 0 : 1; // 0 = primary 1 = ganged
    _driver_part_number = driver_part_number;
    _r_sense = r_sense;
    this->step_pin = step_pin;
    this->dir_pin  = dir_pin;
    this->addr = addr;
    serial->begin(115200, SERIAL_8N1, -1 , -1);
    serial->setRxBufferSize(128);
    hw_serial_init();
    init();
}

/* SW Serial Constructor. */
TrinamicUartDriver :: TrinamicUartDriver( uint8_t axis_index, 
                                          gpio_num_t step_pin,
                                          uint8_t dir_pin,
                                          uint16_t driver_part_number,
                                          float r_sense, 
                                          uint16_t SW_RX_pin,
                                          uint16_t SW_TX_pin,
                                          uint8_t addr) {
    this->axis_index = axis_index;
    this->dual_axis_index = axis_index < 6 ? 0 : 1; // 0 = primary 1 = ganged
    _driver_part_number = driver_part_number;
    _r_sense = r_sense;
    this->step_pin = step_pin;
    this->dir_pin  = dir_pin;
    this->SW_RX_pin = SW_RX_pin;
    this->SW_TX_pin = SW_TX_pin;
    this->addr = addr;
    sw_serial_init();
    init();
}

void TrinamicUartDriver::hw_serial_init() {
    if (_driver_part_number == 2208)
        // TMC 2208 does not use address, this field is 0, differently from 2209
        //tmcstepper = new TMC2208Stepper(SW_RX_pin, SW_TX_pin, _r_sense);//, addr);
        tmcstepper = new TMC2208Stepper(&Serial2, _r_sense);//, (uint8_t)0x0);//, SW_RX_pin, SW_TX_pin);
    else if (_driver_part_number == 2209)
        tmcstepper = new TMC2209Stepper(&Serial2, _r_sense, addr);
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic Uart unsupported p/n:%d", _driver_part_number);
        return;
    }
}
void TrinamicUartDriver::sw_serial_init() {
#if SW_SERIAL_MOTORS //TODO: this ifdef is added to avoid error because ESP32 is not define as SW CAPABLE PLATFORM. Alternatively it can be defined as such.
    if (_driver_part_number == 2208)
        // TMC 2208 does not use address, this field is 0, differently from 2209
        tmcstepper = new TMC2208Stepper(SW_RX_pin, SW_TX_pin, _r_sense);
    else if (_driver_part_number == 2209)
        // tmcstepper = new TMC2209Stepper(sw_serial, _r_sense, addr);
        tmcstepper = new TMC2209Stepper(SW_RX_pin, SW_TX_pin, _r_sense, addr);
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic Uart unsupported p/n:%d", _driver_part_number);
        return;
    }
    // grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "New SW SERIAL created."); 
    
    // Set PDN DISABLE to ensure UART pin is available to use.
    tmcstepper->beginSerial(57600);
    
	tmcstepper->pdn_disable(true);
    
    tmcstepper->senddelay(15);
#endif
}

void TrinamicUartDriver :: init() {
    config_message();
    
    init_step_dir_pins(); // from StandardStepper

    /* If communication with the driver is working, read the 
       main settings, apply new driver settings and then read 
       them back. */
    if(test()) {
        read_settings();
        set_settings();
        read_settings();    
    }
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Driver %c is off, cannot set motor.", report_get_axis_letter(axis_index));
    }

    // Should we move it to the set settings?
    // set_mode();
    
    _is_homing = false;    
    is_active = true;  // as opposed to NullMotors, this is a real motor
}

/*
    This is the startup message showing the basic definition
*/
void TrinamicUartDriver :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%c Axis Trinamic UART driver TMC%d Step:%d Dir:%d Addr:%d",
                   report_get_axis_letter(axis_index),
                   _driver_part_number,
                   step_pin,
                   dir_pin,                  
                   addr);
    // grbl_msg_sendf(CLIENT_SERIAL,
    //                MSG_LEVEL_INFO,
    //                "%c Axis Trinamic UART driver TMC%d Step:%d Dir:%d RX:%d TX:%d Addr:%d",
    //                report_get_axis_letter(axis_index),
    //                _driver_part_number,
    //                step_pin,
    //                dir_pin,                   
    //                SW_RX_pin,                   
    //                SW_TX_pin, 
    //                addr);
}

bool TrinamicUartDriver :: test() {
    uint32_t driver_status = tmcstepper->test_connection();
    
    // grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DRV_STATUS: 0x%x", tmcstepper->DRV_STATUS()); //debug
    switch (driver_status) {
    case 1:
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test failed. Check connection", report_get_axis_letter(axis_index));
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%Trinamic driver status: %x", driver_status);
        return false;
    case 2:
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test failed. Check motor power", report_get_axis_letter(axis_index));
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic driver status: %x", driver_status);
        return false;
    default:
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test passed", report_get_axis_letter(axis_index));
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic driver status: %x", driver_status);
        return true;
    }
}

/*
    This basically pings the driver. It will print an error message if there is one.
*/
void TrinamicUartDriver :: trinamic_test_response() {
    switch (tmcstepper->test_connection()) {
    case 1:
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test failed. Check connection", report_get_axis_letter(axis_index));
        break;
    case 2:
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test failed. Check motor power", report_get_axis_letter(axis_index));
        break;
    default:
        return;
    }
}

/*
    Set setting of the driver. Called at init() and whenever related settings change
*/
void TrinamicUartDriver :: set_settings() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis set_settings() ", report_get_axis_letter(axis_index));

    tmcstepper->I_scale_analog(false);
    tmcstepper->internal_Rsense(false);

	tmcstepper->mstep_reg_select(true);

    tmcstepper->microsteps(settings.microsteps[axis_index]);//(uint16_t)16);//

    tmcstepper->TPOWERDOWN((uint8_t)20);
    tmcstepper->iholddelay((uint8_t)1);
    
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis current request is %f mA ", report_get_axis_letter(axis_index), settings.current[axis_index] * 1000.0);
    tmcstepper->rms_current(settings.current[axis_index] * 1000.0 , settings.hold_current[axis_index] / 100.0);
    
    //TODO: call set mode???
    tmcstepper->en_spreadCycle(true);
    
    // #ifdef TMC2209
    //     tmcstepper->sgt(settings.stallguard[axis_index]);
    // #endif
}



/*
    Read setting and send them to the driver. Called at init() and whenever related settings change
*/
void TrinamicUartDriver :: read_settings() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis read_settings() ", report_get_axis_letter(axis_index));

    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis GCONF REG 0x%x ", report_get_axis_letter(axis_index), tmcstepper->GCONF());
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis GSTAT REG 0x%x ", report_get_axis_letter(axis_index), tmcstepper->GSTAT());
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "IFCNT %d ", tmcstepper->IFCNT());
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis IHOLD_IRUN register 0x%x ", report_get_axis_letter(axis_index), tmcstepper->IHOLD_IRUN());
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis CHOPCONF REG 0x%x ", report_get_axis_letter(axis_index), tmcstepper->CHOPCONF());

    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis RMS current %d mA", report_get_axis_letter(axis_index), tmcstepper->rms_current());
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Microsteps %d ", report_get_axis_letter(axis_index), tmcstepper->microsteps());

}

void TrinamicUartDriver :: set_homing_mode(bool is_homing) {
    _homing_mode = is_homing;
    set_mode();
}

/*
    There are ton of settings. I'll start by grouping then into modes for now.
    Many people will want quiet and stallgaurd homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing

*/
void TrinamicUartDriver :: set_mode() {
    uint8_t mode;

    //TODO: check if this is usable for TMC2209
    // if (_is_homing && (_homing_mode ==  TRINAMIC_HOMING_STALLGUARD)) {
    //     mode = TRINAMIC_RUN_MODE_STALLGUARD;
    // } else {
        mode = TRINAMIC_RUN_MODE;
    // }
    
    //TODO: review most of the commented settings
    if (mode == TRINAMIC_RUN_MODE_STEALTHCHOP) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "STEALTHCHOP");
        // tmcstepper->toff(5);
        tmcstepper->en_spreadCycle(0);//equivalent of tmcstepper->en_pwm_mode(1);// Enable extremely quiet stepping
        tmcstepper->pwm_autoscale(1);
    } else  {  // if (mode == TRINAMIC_RUN_MODE_COOLSTEP || mode == TRINAMIC_RUN_MODE_STALLGUARD)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "COOLSTEP");   
        // tmcstepper->en_spreadCycle(true);
        // tmcstepper->tbl(1);
        // tmcstepper->toff(3);
        // tmcstepper->hstrt(4);//hysteresis_start(4);
        // tmcstepper->hend(-2);//hysteresis_end(-2);
        // tmcstepper->sfilt(1);
        // tmcstepper->diag1_pushpull(0); // 0 = active low
        // tmcstepper->diag1_stall(1); // stallguard i/o is on diag1        
        // if (mode == TRINAMIC_RUN_MODE_COOLSTEP) {
        //     tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
        //     tmcstepper->THIGH(NORMAL_THIGH);
        // } else 
        // {
        //     uint32_t tcoolthrs = calc_tstep(settings.homing_feed_rate, 150.0);
        //     uint32_t thigh = calc_tstep(settings.homing_feed_rate, 60.0);
        //     grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Tstep range %d - %d SGV:%d", thigh, tcoolthrs, tmcstepper->sgt());
        //     tmcstepper->TCOOLTHRS(tcoolthrs);
        //     tmcstepper->THIGH(thigh);
        // }       
    }
}

    /*
        This is the stallguard tuning info. It is call debug, so it could be generic across all class.
    */
    void TrinamicUartDriver :: debug_message() {

        uint32_t tstep = tmcstepper->TSTEP();

        if (tstep == 0xFFFFF || tstep == -1)   // if axis is not moving return
            return;

        float feedrate = st_get_realtime_rate(); //* settings.microsteps[axis_index] / 60.0 ; // convert mm/min to Hz

        grbl_msg_sendf(CLIENT_SERIAL,
                       MSG_LEVEL_INFO,
                       "%c Rate: %05.0fmm/min,   %d",
                       report_get_axis_letter(axis_index),
                       feedrate,
                       tstep);
#if 0 //TMC2209
        grbl_msg_sendf(CLIENT_SERIAL,
                       MSG_LEVEL_INFO,
                       "%c Stallguard  %d   SG_Val: %04d   Rate: %05.0fmm/min,   %d",
                       report_get_axis_letter(axis_index),
                       tmcstepper->stallguard(),
                       tmcstepper->sg_result(),
                       feedrate,
                       tstep);
#endif
    }

#if 0 //TODO: is it useful for TMC2209 ??
    // calculate a tstep from a rate
    // tstep = TRINAMIC_FCLK / (time between 1/256 steps)
    // This is used to set the stallguard window from the homing speed.
    // The percent is the offset on the window 
    uint32_t TrinamicUartDriver :: calc_tstep(float speed, float percent) {
        float tstep = speed / 60.0 * settings.steps_per_mm[axis_index] * (float)(256 / settings.microsteps[axis_index]);
        tstep = TRINAMIC_FCLK / tstep * percent / 100.0;

        return (uint32_t)tstep;
    }
#endif

// this can use the enable feature over SPI. The dedicated pin must be in the enable mode, 
// but that can be hardwired that way.
    void TrinamicUartDriver :: set_disable(bool disable) {
        #ifdef USE_TRINAMIC_ENABLE
            if (disable) {
                tmcstepper->toff(0);
            } else {
                set_mode(); // resets everything including toff
            }
        #endif
        // the pin based enable could be added here.
        // This would be for individual motors, not the single pin for all motors.
    }
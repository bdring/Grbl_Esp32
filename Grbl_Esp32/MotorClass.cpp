/*
    MotorClass.cpp

    A Motor Class and derived classes
        Motor           - A base class. Do not use
        Nullmotor       - Used when there is not motor on the axis
        TrinamicDriver  - A motor powered by a Trinamic Driver

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

    TODO
        Deal with custom machine ... machine_trinamic_setup();
        Class is ready to deal with non SPI pins, but they have not been needed yet.
            It would be nice in the config message though
*/

#include "grbl.h"
#include "MotorClass.h"
#include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper

Motor* myMotor[6][2]; // number of axes (normal and ganged)
static TaskHandle_t readSgTaskHandle = 0;   // for realtime stallguard data diaplay


void init_motors() {

    SPI.begin(); // Yes, I know about the SD issue

    // this WILL be done better with settings
#ifdef X_TRINAMIC
    myMotor[X_AXIS][0] = new TrinamicDriver(X_AXIS, X_DRIVER, X_RSENSE, X_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[X_AXIS][0] = new Nullmotor();
#endif
#ifdef X2_TRINAMIC
    myMotor[X_AXIS][1] = new TrinamicDriver(X_AXIS, X2_DRIVER, X2_RSENSE, X2_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[X_AXIS][1] = new Nullmotor();
#endif


#ifdef Y_TRINAMIC
    myMotor[Y_AXIS][0] = new TrinamicDriver(Y_AXIS, Y_DRIVER, Y_RSENSE, Y_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[Y_AXIS][0] = new Nullmotor();
#endif
#ifdef Y2_TRINAMIC
    myMotor[Y_AXIS][1] = new TrinamicDriver(Y_AXIS, Y2_DRIVER, Y2_RSENSE, Y2_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[Y_AXIS][1] = new Nullmotor();
#endif

#ifdef Z_TRINAMIC
    myMotor[Z_AXIS][0] = new TrinamicDriver(Z_AXIS, Z_DRIVER, Z_RSENSE, Z_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[Z_AXIS][0] = new Nullmotor();
#endif
#ifdef Z2_TRINAMIC
    myMotor[Z_AXIS][1] = new TrinamicDriver(Z_AXIS, Z2_DRIVER, Z2_RSENSE, Z2_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[Z_AXIS][1] = new Nullmotor();
#endif

#ifdef A_TRINAMIC
    myMotor[A_AXIS][0] = new TrinamicDriver(A_AXIS, A_DRIVER, A_RSENSE, A_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[A_AXIS][0] = new Nullmotor();
#endif
#ifdef A2_TRINAMIC
    myMotor[A_AXIS][1] = new TrinamicDriver(A_AXIS, A2_DRIVER, A2_RSENSE, A2_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[A_AXIS][1] = new Nullmotor();
#endif

#ifdef B_TRINAMIC
    myMotor[B_AXIS][0] = new TrinamicDriver(B_AXIS, B_DRIVER, B_RSENSE, B_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[B_AXIS][0] = new Nullmotor();
#endif
#ifdef B2_TRINAMIC
    myMotor[B_AXIS][1] = new TrinamicDriver(B_AXIS, B2_DRIVER, B2_RSENSE, B2_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[B_AXIS][1] = new Nullmotor();
#endif

#ifdef C_TRINAMIC
    myMotor[C_AXIS][0] = new TrinamicDriver(C_AXIS, C_DRIVER, C_RSENSE, C_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[C_AXIS][0] = new Nullmotor();
#endif
#ifdef C2_TRINAMIC
    myMotor[C_AXIS][1] = new TrinamicDriver(C_AXIS, C2_DRIVER, C2_RSENSE, C2_CS_PIN, get_next_trinamic_driver_index());
#else
    myMotor[C_AXIS][1] = new Nullmotor();
#endif

// tuning gets turned on if this is defined and laser mode is on at boot time.
#ifdef ENABLE_STALLGUARD_TUNING  // TODO move this to a setting
    if (bit_istrue(settings.flags, BITFLAG_LASER_MODE)) {
        xTaskCreatePinnedToCore(readSgTask,     // task
                                "readSgTask", // name for task
                                4096,   // size of task stack
                                NULL,   // parameters
                                1, // priority
                                &readSgTaskHandle,
                                0 // core
                               );
    }
#endif


}
// returns the next spi index. We cannot preassign to axes because ganged (X2 type axes) might
// need to be inserted into the order of axes.
uint8_t get_next_trinamic_driver_index() {
    static uint8_t index = 1; // they start at 1
#ifndef TRINAMIC_DAISY_CHAIN
    return -1;
#else
    return index++;
#endif
}

/*
    This will print StallGuard data that is useful for tuning.
*/
void readSgTask(void* pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xreadSg = 50;  // in ticks (typically ms)
    uint32_t tstep;
    uint8_t sg;

    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
    while (true) { // don't ever return from this or the task dies
        if (bit_istrue(settings.flags, BITFLAG_LASER_MODE)) { // use laser mode as a way to turn off this data TODO... Needs its own setting
            for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
                for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
                    if (myMotor[axis][gang_index]->is_active)
                        myMotor[axis][gang_index]->debug_message();
                }
            }
        }
        vTaskDelayUntil(&xLastWakeTime, xreadSg);
    }
}

/*
    This is called when settings that affect SPI motors are changed
*/
void motor_read_settings() {
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
            if (myMotor[axis][gang_index]->is_active)
                myMotor[axis][gang_index]->read_settings();
        }
    }
}

// ============================== Class Methods ================================================

Motor :: Motor() {
}

void Motor :: init() {
}

void Motor :: config_message() {
}

void Motor :: debug_message() {
}

void Motor :: read_settings() {

}

// ========================================= TrinamicDriver Class =======================================

TrinamicDriver :: TrinamicDriver(uint8_t axis_index, uint16_t driver_part_number, float r_sense, uint8_t cs_pin, int8_t index) {
    this->axis_index = axis_index;
    _driver_part_number = driver_part_number;
    _r_sense = r_sense;
    this->cs_pin = cs_pin;
    this->index = index;
    init();
}

void TrinamicDriver :: init() {
    if (_driver_part_number == 2130)
        tmcstepper = new TMC2130Stepper(cs_pin, _r_sense, index);
    else if (_driver_part_number == 5160)
        tmcstepper = new TMC5160Stepper(cs_pin, _r_sense, index);
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic unsupported p/n:%d", _driver_part_number);
        return;
    }
    config_message();
    tmcstepper->begin();
    trinamic_test_response(); // Prints an error if there is a problem
    read_settings(); // pull info from settings
    #ifdef TRINAMIC_RUN_MODE // temporary fix for when not defined
    set_mode(TRINAMIC_RUN_MODE);
    #endif
    is_active = true;
}

/*
    This is the startup message showing the basic definition
*/
void TrinamicDriver :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%c Axis Trinamic driver TMC%d CS_PIN:%d Index:%d",
                   report_get_axis_letter(axis_index),
                   _driver_part_number,
                   cs_pin, index);
}

/*
    This basically pings the driver. It will print an error message if the is one.
*/
void TrinamicDriver :: trinamic_test_response() {
    uint8_t result;
    result = tmcstepper->test_connection();

    if (result) {
        switch (result) {
        case 1:
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test failed. Check connection", report_get_axis_letter(axis_index));
            break;
        case 2:
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Trinamic driver test failed. Check motor power", report_get_axis_letter(axis_index));
            break;
        }
    }
}


void TrinamicDriver :: read_settings() {
    tmcstepper->microsteps(settings.microsteps[axis_index]);
    tmcstepper->rms_current(settings.current[axis_index] * 1000.0, settings.hold_current[axis_index] / 100.0);
    tmcstepper->sgt(settings.stallguard[axis_index]);
}


/*
    There are ton of settings. I'll start by grouping then into modes for now.
    Many people will want quiet and stallgaurd homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing

*/
void TrinamicDriver :: set_mode(uint8_t mode) {
    if (mode == TRINAMIC_RUN_MODE_STEALTHCHOP) {
        tmcstepper->toff(5);
        tmcstepper->en_pwm_mode(1);      // Enable extremely quiet stepping
        tmcstepper->pwm_autoscale(1);
    } else if (mode == TRINAMIC_RUN_MODE_COOLSTEP) {
        tmcstepper->tbl(1);
        tmcstepper->toff(3);
        tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
        tmcstepper->THIGH(NORMAL_THIGH);
        tmcstepper->hysteresis_start(4);
        tmcstepper->hysteresis_end(-2);
        tmcstepper->diag1_stall(0); // stallguard i/o is not on diag1
    } else if (mode == TRINAMIC_RUN_MODE_STALLGUARD) {
        // setup the homing mode motor settings
        tmcstepper->tbl(1);
        tmcstepper->toff(3);
        tmcstepper->TCOOLTHRS(HOMING_TCOOLTHRS) ;
        tmcstepper->THIGH(HOMING_THIGH);
        tmcstepper->hysteresis_start(4);
        tmcstepper->hysteresis_end(-2);
        tmcstepper->sfilt(1);
        tmcstepper->diag1_stall(1); // stallguard i/o is on diag1
        tmcstepper->diag1_pushpull(0); // 0 = active low
    } else
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic undefined mode requested:%d", mode);
}
/*
    This is the stallguard tuning info. It is call debug, so it could be generic across all class.    
*/
void TrinamicDriver :: debug_message() {
    uint32_t tstep;
    uint8_t sg;
    float feedrate;

    tstep = tmcstepper->TSTEP();
    if (tstep == 0xFFFFF)   // if axis is not moving return
        return;

    feedrate = st_get_realtime_rate(); //* settings.microsteps[axis_index] / 60.0 ; // convert mm/min to Hz

    sg = tmcstepper->sg_result();
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%c Stallguard  SG:%03d TS:%05d Rate:%05.2fHz",
                   report_get_axis_letter(axis_index),
                   sg,
                   tstep,
                   feedrate);
}
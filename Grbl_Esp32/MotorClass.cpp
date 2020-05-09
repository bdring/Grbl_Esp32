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

    Reference
        TMC2130 Datasheet https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2130_datasheet.pdf
*/

#include "grbl.h"
#include "MotorClass.h"
#include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper

Motor* myMotor[6][2]; // number of axes (normal and ganged)
static TaskHandle_t readSgTaskHandle = 0;   // for realtime stallguard data diaplay


void init_motors() {

    SPI.begin(); // Yes, I know about the SD issue

    // this WILL be done better with settings
#ifdef X_TRINAMIC_DRIVER
    myMotor[X_AXIS][0] = new TrinamicDriver(X_AXIS, X_TRINAMIC_DRIVER, X_RSENSE, X_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef X_STEP_PIN
        myMotor[X_AXIS][0] = new StandardStepper(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN);
    #else
        myMotor[X_AXIS][0] = new Nullmotor();
    #endif    
#endif

#ifdef X2_TRINAMIC_DRIVER
    myMotor[X_AXIS][1] = new TrinamicDriver(X_AXIS, X2_TRINAMIC_DRIVER, X2_RSENSE, X2_CS_PIN, get_next_trinamic_driver_index());
#else
   #ifdef X2_STEP_PIN
        myMotor[X_AXIS][1] = new StandardStepper(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN);
    #else
        myMotor[X_AXIS][1] = new Nullmotor();
    #endif 
#endif


#ifdef Y_TRINAMIC_DRIVER
    myMotor[Y_AXIS][0] = new TrinamicDriver(Y_AXIS, Y_TRINAMIC_DRIVER, Y_RSENSE, Y_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Y_STEP_PIN
        myMotor[Y_AXIS][0] = new StandardStepper(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN);
    #else
        myMotor[Y_AXIS][0] = new Nullmotor();
    #endif 
#endif


#ifdef Y2_TRINAMIC_DRIVER
    myMotor[Y_AXIS][1] = new TrinamicDriver(Y_AXIS, Y2_TRINAMIC_DRIVER, Y2_RSENSE, Y2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Y2_STEP_PIN
        myMotor[Y_AXIS][1] = new StandardStepper(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN);
    #else
        myMotor[Y_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef Z_TRINAMIC_DRIVER
    myMotor[Z_AXIS][0] = new TrinamicDriver(Z_AXIS, Z_TRINAMIC_DRIVER, Z_RSENSE, Z_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Z_STEP_PIN
        myMotor[Z_AXIS][0] = new StandardStepper(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN);
    #else
        myMotor[Z_AXIS][0] = new Nullmotor();
    #endif 
#endif

#ifdef Z2_TRINAMIC_DRIVER
    myMotor[Z_AXIS][1] = new TrinamicDriver(Z_AXIS, Z2_TRINAMIC_DRIVER, Z2_RSENSE, Z2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Z2_STEP_PIN
        myMotor[Z_AXIS][1] = new StandardStepper(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN);
    #else
        myMotor[Z_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef A_TRINAMIC_DRIVER
    myMotor[A_AXIS][0] = new TrinamicDriver(A_AXIS, A_TRINAMIC_DRIVER, A_RSENSE, A_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A_STEP_PIN
        myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][0] = new Nullmotor();
    #endif 
#endif

#ifdef A2_TRINAMIC_DRIVER
    myMotor[A_AXIS][1] = new TrinamicDriver(A_AXIS, A2_TRINAMIC_DRIVER, A2_RSENSE, A2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A2_STEP_PIN
        myMotor[A_AXIS][1] = new StandardStepper(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef B_TRINAMIC_DRIVER
    myMotor[B_AXIS][0] = new TrinamicDriver(B_AXIS, B_TRINAMIC_DRIVER, B_RSENSE, B_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A_STEP_PIN
        myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][0] = new Nullmotor();
    #endif 
#endif
#ifdef B2_TRINAMIC_DRIVER
    myMotor[B_AXIS][1] = new TrinamicDriver(B_AXIS, B2_TRINAMIC_DRIVER, B2_RSENSE, B2_CS_PIN, get_next_trinamic_driver_index());
#else
   #ifdef B2_STEP_PIN
        myMotor[B_AXIS][1] = new StandardStepper(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN);
    #else
        myMotor[B_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef C_TRINAMIC_DRIVER
    myMotor[C_AXIS][0] = new TrinamicDriver(C_AXIS, C_TRINAMIC_DRIVER, C_RSENSE, C_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A_STEP_PIN
        myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][0] = new Nullmotor();
    #endif 
#endif
#ifdef C2_TRINAMIC_DRIVER
    myMotor[C_AXIS][1] = new TrinamicDriver(C_AXIS, C2_TRINAMIC_DRIVER, C2_RSENSE, C2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef C2_STEP_PIN
        myMotor[C_AXIS][1] = new StandardStepper(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN);
    #else
        myMotor[C_AXIS][1] = new Nullmotor();
    #endif 
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
                    if (myMotor[axis][gang_index]->is_active)// get rid of this
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
            if (myMotor[axis][gang_index]->is_active) // get rid of this
                myMotor[axis][gang_index]->read_settings();
        }
    }
}

// use this to tell all the motors what the current homing mode is
// They can use this to setup things like Stall
void motors_set_homing_mode(bool is_homing) {
    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "motors_set_homing_mode(%d)", is_homing);
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
            myMotor[axis][gang_index]->set_homing_mode(is_homing);
        }
    }
}

void motors_set_enable(bool enabled) {
    // TODO the global enable pin goes here 
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
            myMotor[axis][gang_index]->set_enable(enabled);
        }
    }
}

// ============================== Class Methods ================================================

Motor :: Motor() {
}

void Motor :: init() {
    _is_homing = false;
}

void Motor :: config_message() {
}

void Motor :: debug_message() {
}

void Motor :: read_settings() {
}

void Motor :: set_enable(bool enable) {
    // TODO Can be used for individual enables or SPI soft enables
}

void Motor :: set_homing_mode(bool is_homing) {
    _is_homing = is_homing;
}

StandardStepper :: StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin) {
    this->axis_index = axis_index;
    this->step_pin = step_pin;
    this->dir_pin = dir_pin;
    init();
    config_message();
}

void StandardStepper :: init() {
    _is_homing = false;

    // TODO Step but, but RMT complicates things
    pinMode(dir_pin, OUTPUT);
}


void StandardStepper :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%c Axis standard stepper motor Step:%d Dir:%d",
                   report_get_axis_letter(axis_index),
                   step_pin,
                   dir_pin);
}

// ========================================= TrinamicDriver Class =======================================

TrinamicDriver :: TrinamicDriver(uint8_t axis_index, uint16_t driver_part_number, float r_sense, uint8_t cs_pin, int8_t spi_index) {
    this->axis_index = axis_index;
    _driver_part_number = driver_part_number;
    _r_sense = r_sense;
    this->cs_pin = cs_pin;
    this->spi_index = spi_index;
    init();
}

void TrinamicDriver :: init() {
    if (_driver_part_number == 2130)
        tmcstepper = new TMC2130Stepper(cs_pin, _r_sense, spi_index);
    else if (_driver_part_number == 5160)
        tmcstepper = new TMC5160Stepper(cs_pin, _r_sense, spi_index);
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Trinamic unsupported p/n:%d", _driver_part_number);
        return;
    }
    config_message();
    // TODO step pins
    pinMode(dir_pin, OUTPUT);
    tmcstepper->begin();
    trinamic_test_response(); // Prints an error if there is a problem
    read_settings(); // pull info from settings
    set_mode();
    is_active = true;  // as opposed to NullMotors, this is a real motor
}

/*
    This is the startup message showing the basic definition
*/
void TrinamicDriver :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "%c Axis Trinamic driver TMC%d Step:%d Dir:%d CS:%d Index:%d",
                   report_get_axis_letter(axis_index),
                   _driver_part_number,
                   step_pin,
                   dir_pin,                   
                   cs_pin, 
                   spi_index);
}

/*
    This basically pings the driver. It will print an error message if there is one.
*/
void TrinamicDriver :: trinamic_test_response() {
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
    Read setting and send them to the driver. Called at init() and whenever related settings change
*/
void TrinamicDriver :: read_settings() {
    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "%c Axis read_settings() ", report_get_axis_letter(axis_index));
    tmcstepper->microsteps(settings.microsteps[axis_index]);
    tmcstepper->rms_current(settings.current[axis_index] * 1000.0, settings.hold_current[axis_index] / 100.0);
    tmcstepper->sgt(settings.stallguard[axis_index]);
}

void TrinamicDriver :: set_homing_mode(bool is_homing) {
    _homing_mode = is_homing;
    set_mode();
}


/*
    There are ton of settings. I'll start by grouping then into modes for now.
    Many people will want quiet and stallgaurd homing. Stallguard only run in
    Coolstep mode, so it will need to switch to Coolstep when homing

*/
void TrinamicDriver :: set_mode() {
    uint8_t mode;

    if (_is_homing && (_homing_mode ==  TRINAMIC_HOMING_STALLGUARD)) {
        mode = TRINAMIC_RUN_MODE_STALLGUARD;
    } else {
        mode = TRINAMIC_RUN_MODE;
    }
    
    if (mode == TRINAMIC_RUN_MODE_STEALTHCHOP) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TRINAMIC_RUN_MODE_STEALTHCHOP");
        tmcstepper->toff(5);
        tmcstepper->en_pwm_mode(1);      // Enable extremely quiet stepping
        tmcstepper->pwm_autoscale(1);
    } else if (mode == TRINAMIC_RUN_MODE_COOLSTEP || mode == TRINAMIC_RUN_MODE_STALLGUARD) {        
        tmcstepper->tbl(1);
        tmcstepper->toff(3);
        tmcstepper->hysteresis_start(4);
        tmcstepper->hysteresis_end(-2);
        tmcstepper->sfilt(1);
        tmcstepper->diag1_pushpull(0); // 0 = active low
        tmcstepper->diag1_stall(1); // stallguard i/o is on diag1
        if (mode == TRINAMIC_RUN_MODE_COOLSTEP) {
            tmcstepper->TCOOLTHRS(NORMAL_TCOOLTHRS); // when to turn on coolstep
            tmcstepper->THIGH(NORMAL_THIGH);
        } else {
            tmcstepper->TCOOLTHRS(HOMING_TCOOLTHRS);
            tmcstepper->THIGH(HOMING_THIGH);
        }       
    } else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "TRINAMIC_RUN_MODE_STALLGUARD");
        tmcstepper->tbl(1);
        tmcstepper->toff(3);
        tmcstepper->TCOOLTHRS(HOMING_TCOOLTHRS);
        tmcstepper->THIGH(HOMING_THIGH);
        tmcstepper->hysteresis_start(4);
        tmcstepper->hysteresis_end(-2);
        tmcstepper->sfilt(1);
        tmcstepper->diag1_stall(1); // stallguard i/o is on diag1
        tmcstepper->diag1_pushpull(0); // 0 = active low
    }    
}

    /*
        This is the stallguard tuning info. It is call debug, so it could be generic across all class.
    */
    void TrinamicDriver :: debug_message() {

        uint32_t tstep = tmcstepper->TSTEP();

        if (tstep == 0xFFFFF || tstep == -1)   // if axis is not moving return
            return;

        float feedrate = st_get_realtime_rate(); //* settings.microsteps[axis_index] / 60.0 ; // convert mm/min to Hz

        grbl_msg_sendf(CLIENT_SERIAL,
                       MSG_LEVEL_INFO,
                       "%c Stallguard  %d   SG_Val: %04d   Rate: %05.0fmm/min,   %d",
                       report_get_axis_letter(axis_index),
                       tmcstepper->stallguard(),
                       tmcstepper->sg_result(),
                       feedrate,
                       tstep);
    }
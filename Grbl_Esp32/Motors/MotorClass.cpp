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
        Move step pin stuff to classes
        Make Trinamic subclass of standard stepper
        Create a unipolar class
        Need a way to indicate ganged axes in messages
        Make sure public/private/protected is cleaned up.
              
        Deal with custom machine ... machine_trinamic_setup();
        Class is ready to deal with non SPI pins, but they have not been needed yet.
            It would be nice in the config message though        

    Reference
        TMC2130 Datasheet https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2130_datasheet.pdf
*/
#include "TrinamicDriverClass.cpp"
#include "StandardStepperClass.cpp"
#include "UnipolarMotorClass.cpp"

Motor* myMotor[6][2]; // number of axes (normal and ganged)
static TaskHandle_t readSgTaskHandle = 0;   // for realtime stallguard data diaplay

uint8_t rmt_chan_num[6][2];
rmt_item32_t rmtItem[2];
rmt_config_t rmtConfig;

void init_motors() {    
    // TODO SPI needs to be done before constructors because they have an init that uses SPI
    // Should move all inits to the end and conditionally turn on SPI
    SPI.begin(); // Yes, I know about the SD issue

    // this WILL be done better with settings
#ifdef X_TRINAMIC_DRIVER
    myMotor[X_AXIS][0] = new TrinamicDriver(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_TRINAMIC_DRIVER, X_RSENSE, X_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef X_UNIPOLAR
        myMotor[X_AXIS][0] = new UnipolarMotor(X_AXIS, X_PIN_PHASE_0, X_PIN_PHASE_1, X_PIN_PHASE_2, X_PIN_PHASE_3);
    #else
        #ifdef X_STEP_PIN
            myMotor[X_AXIS][0] = new StandardStepper(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN);
        #else
            myMotor[X_AXIS][0] = new Nullmotor();
        #endif    
    #endif
#endif

#ifdef X2_TRINAMIC_DRIVER
    myMotor[X_AXIS][1] = new TrinamicDriver(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE, X2_CS_PIN, get_next_trinamic_driver_index());
#else
   #ifdef X2_STEP_PIN
        myMotor[X_AXIS][1] = new StandardStepper(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN);
    #else
        myMotor[X_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef Y_TRINAMIC_DRIVER
    myMotor[Y_AXIS][0] = new TrinamicDriver(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE, Y_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Y_STEP_PIN
        myMotor[Y_AXIS][0] = new StandardStepper(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN);
    #else
        myMotor[Y_AXIS][0] = new Nullmotor();
    #endif 
#endif

#ifdef Y2_TRINAMIC_DRIVER
    myMotor[Y_AXIS][1] = new TrinamicDriver(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE, Y2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Y2_STEP_PIN
        myMotor[Y_AXIS][1] = new StandardStepper(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN);
    #else
        myMotor[Y_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef Z_TRINAMIC_DRIVER
    myMotor[Z_AXIS][0] = new TrinamicDriver(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE, Z_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Z_STEP_PIN
        myMotor[Z_AXIS][0] = new StandardStepper(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN);
    #else
        myMotor[Z_AXIS][0] = new Nullmotor();
    #endif 
#endif

#ifdef Z2_TRINAMIC_DRIVER
    myMotor[Z_AXIS][1] = new TrinamicDriver(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE, Z2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef Z2_STEP_PIN
        myMotor[Z_AXIS][1] = new StandardStepper(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN);
    #else
        myMotor[Z_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef A_TRINAMIC_DRIVER
    myMotor[A_AXIS][0] = new TrinamicDriver(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_TRINAMIC_DRIVER, A_RSENSE, A_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A_STEP_PIN
        myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][0] = new Nullmotor();
    #endif 
#endif

#ifdef A2_TRINAMIC_DRIVER
    myMotor[A_AXIS][1] = new TrinamicDriver(A_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_TRINAMIC_DRIVER, A2_RSENSE, A2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A2_STEP_PIN
        myMotor[A_AXIS][1] = new StandardStepper(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef B_TRINAMIC_DRIVER
    myMotor[B_AXIS][0] = new TrinamicDriver(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_TRINAMIC_DRIVER, B_RSENSE, B_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A_STEP_PIN
        myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][0] = new Nullmotor();
    #endif 
#endif
#ifdef B2_TRINAMIC_DRIVER
    myMotor[B_AXIS][1] = new TrinamicDriver(B_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_TRINAMIC_DRIVER, B2_RSENSE, B2_CS_PIN, get_next_trinamic_driver_index());
#else
   #ifdef B2_STEP_PIN
        myMotor[B_AXIS][1] = new StandardStepper(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN);
    #else
        myMotor[B_AXIS][1] = new Nullmotor();
    #endif 
#endif

#ifdef C_TRINAMIC_DRIVER
    myMotor[C_AXIS][0] = new TrinamicDriver(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_TRINAMIC_DRIVER, C_RSENSE, C_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef A_STEP_PIN
        myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
    #else
        myMotor[A_AXIS][0] = new Nullmotor();
    #endif 
#endif
#ifdef C2_TRINAMIC_DRIVER
    myMotor[C_AXIS][1] = new TrinamicDriver(C_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_TRINAMIC_DRIVER, C2_RSENSE, C2_CS_PIN, get_next_trinamic_driver_index());
#else
    #ifdef C2_STEP_PIN
        myMotor[C_AXIS][1] = new StandardStepper(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN);
    #else
        myMotor[C_AXIS][1] = new Nullmotor();
    #endif 
#endif
    
    #ifdef STEPPERS_DISABLE_PIN
        pinMode(STEPPERS_DISABLE_PIN, OUTPUT); // global motor enable pin
    #endif
    motors_set_disable(true);

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

void motors_set_disable(bool disable) {
    
    // now step through all the motors
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
            myMotor[axis][gang_index]->set_disable(disable);
        }
    }
    // global diable pin
    #ifdef STEPPERS_DISABLE_PIN   
        if (bit_istrue(settings.flags, BITFLAG_INVERT_ST_ENABLE)) {
            disable = !disable;    // Apply pin invert.
        }
        digitalWrite(STEPPERS_DISABLE_PIN, disable);
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

void motors_set_direction_pins(uint8_t onMask) {
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
            myMotor[axis][gang_index]->set_direction_pins(onMask);
        }
    }
}

// ============================== Class Methods ================================================

Motor :: Motor() {
}

void Motor :: init() {
    _is_homing = false;
}

void Motor :: config_message() {}
void Motor :: debug_message() {}
void Motor :: read_settings() {}
void Motor :: set_disable(bool disable) {}
void Motor :: set_direction_pins(uint8_t onMask) {}
void Motor :: step(bool step, bool dir_forward) {}

void Motor :: set_homing_mode(bool is_homing) {
    _is_homing = is_homing;
}

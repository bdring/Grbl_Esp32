/*
    MotorClass.cpp

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
        Make sure public/private/protected is cleaned up.
        Only a few Unipolar axes have been setup in init()
        Get rid of Z_SERVO, just reply on Z_SERVO_PIN

        Deal with custom machine ... machine_trinamic_setup();
        Class is ready to deal with non SPI pins, but they have not been needed yet.
            It would be nice in the config message though

    Testing
        Done (success)
            3 Axis (3 Standard Steppers)
            MPCNC (ganged with shared direction pin)
            TMC2130 Pen Laser (trinamics, stallguard tuning)
            Unipolar

        TODO
            4 Axis SPI (Daisy Chain, Ganged with unique direction pins)


    Reference
        TMC2130 Datasheet https://www.trinamic.com/fileadmin/assets/Products/ICs_Documents/TMC2130_datasheet.pdf
*/
#include "TrinamicDriverClass.cpp"
#include "TrinamicUartDriverClass.cpp"
#include "StandardStepperClass.cpp"
#include "UnipolarMotorClass.cpp"
#include "RcServoClass.cpp"
#include "SolenoidClass.cpp"

Motor* myMotor[6][2]; // number of axes (normal and ganged)
static TaskHandle_t readSgTaskHandle = 0;   // for realtime stallguard data diaplay
static TaskHandle_t servoUpdateTaskHandle = 0;

uint8_t rmt_chan_num[6][2];
rmt_item32_t rmtItem[2];
rmt_config_t rmtConfig;

bool motor_class_steps; // true if at least one motor class is handling steps

void init_motors() {
    bool need_servo_task = false;
    motor_class_steps = false; // does any class need step info?
    // TODO SPI needs to be done before constructors because they have an init that uses SPI
    // Should move all inits to the end and conditionally turn on SPI
    SPI.begin(); // Yes, I know about the SD issue

    // this WILL be done better with settings
#ifdef X_TRINAMIC_DRIVER
    #if(X_TRINAMIC_DRIVER == 2130)
    {
    myMotor[X_AXIS][0] = new TrinamicDriver(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_TRINAMIC_DRIVER, X_RSENSE, X_CS_PIN, get_next_trinamic_driver_index());
    }
    #elif(X_TRINAMIC_DRIVER == 2208 || X_TRINAMIC_DRIVER == 2209)
    {
        #ifdef HW_SERIAL_MOTORS
        myMotor[X_AXIS][0] = new TrinamicUartDriver(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_TRINAMIC_DRIVER, X_RSENSE, SERIAL_FOR_MOTORS, get_trinamic_driver_uart_address(X_AXIS));
        #elif defined(SW_SERIAL_MOTORS)
        myMotor[X_AXIS][0] = new TrinamicUartDriver(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_TRINAMIC_DRIVER, X_RSENSE, RX_SW_SERIAL_MOTORS, TX_SW_SERIAL_MOTORS, get_trinamic_driver_uart_address(X_AXIS)); 
        #else
        #error "Please define which type of serial you want to use for the trinamic motor, either HW_SERIAL_MOTORS or SW_SERIAL_MOTORS."
        #endif
    }
    #endif
#elif defined(X_SERVO_PIN)
    myMotor[X_AXIS][0] = new RcServo(X_AXIS, X_SERVO_PIN, X_SERVO_RANGE_MIN, X_SERVO_RANGE_MAX);
#elif defined(X_UNIPOLAR)
    myMotor[X_AXIS][0] = new UnipolarMotor(X_AXIS, X_PIN_PHASE_0, X_PIN_PHASE_1, X_PIN_PHASE_2, X_PIN_PHASE_3);
#elif defined(X_STEP_PIN)
    myMotor[X_AXIS][0] = new StandardStepper(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN);
#else
    myMotor[X_AXIS][0] = new Nullmotor();
#endif

#ifdef X2_TRINAMIC_DRIVER
    #if(X2_TRINAMIC_DRIVER == 2130)
    {
    myMotor[X_AXIS][1] = new TrinamicDriver(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE, X2_CS_PIN, get_next_trinamic_driver_index());
    }
    #elif(X2_TRINAMIC_DRIVER == 2208 || X2_TRINAMIC_DRIVER == 2209)
    {
        #ifdef HW_SERIAL_MOTORS
        myMotor[X_AXIS][1] = new TrinamicUartDriver(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE, SERIAL_FOR_MOTORS, get_trinamic_driver_uart_address(X2_AXIS));
        #elif defined(SW_SERIAL_MOTORS)
        myMotor[X_AXIS][1] = new TrinamicUartDriver(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE, RX_SW_SERIAL_MOTORS, TX_SW_SERIAL_MOTORS, get_trinamic_driver_uart_address(X2_AXIS)); 
        #else
        #error "Please define which type of serial you want to use for the trinamic motor, either HW_SERIAL_MOTORS or SW_SERIAL_MOTORS."
        #endif
    }
    #endif
#elif defined(X2_SERVO_PIN)
    myMotor[X_AXIS][1] = new RcServo(X2_AXIS, X2_SERVO_PIN), X2_SERVO_RANGE_MIN, X2_SERVO_RANGE_MAX;
#elif defined(X2_UNIPOLAR)
    myMotor[X_AXIS][1] = new UnipolarMotor(X2_AXIS, X2_PIN_PHASE_0, X2_PIN_PHASE_1, X2_PIN_PHASE_2, X2_PIN_PHASE_3);
#elif defined(X2_STEP_PIN)
    myMotor[X_AXIS][1] = new StandardStepper(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN);
#else
    myMotor[X_AXIS][1] = new Nullmotor();
#endif


    // this WILL be done better with settings
#ifdef Y_TRINAMIC_DRIVER
    #if(Y_TRINAMIC_DRIVER == 2130)
    {
    myMotor[Y_AXIS][0] = new TrinamicDriver(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE, Y_CS_PIN, get_next_trinamic_driver_index());
    }
    #elif(Y_TRINAMIC_DRIVER == 2208 || Y_TRINAMIC_DRIVER == 2209)
    {
        #ifdef HW_SERIAL_MOTORS
        myMotor[Y_AXIS][0] = new TrinamicUartDriver(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE, SERIAL_FOR_MOTORS, get_trinamic_driver_uart_address(Y_AXIS));
        #elif defined(SW_SERIAL_MOTORS)
        myMotor[Y_AXIS][0] = new TrinamicUartDriver(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE, RX_SW_SERIAL_MOTORS, TX_SW_SERIAL_MOTORS, get_trinamic_driver_uart_address(Y_AXIS)); 
        #else
        #error "Please define which type of serial you want to use for the trinamic motor, either HW_SERIAL_MOTORS or SW_SERIAL_MOTORS."
        #endif
    }
    #endif
#elif defined(Y_SERVO_PIN)
    myMotor[Y_AXIS][0] = new RcServo(Y_AXIS, Y_SERVO_PIN, Y_SERVO_RANGE_MIN, Y_SERVO_RANGE_MAX);
#elif defined(Y_UNIPOLAR)
    myMotor[Y_AXIS][0] = new UnipolarMotor(Y_AXIS, Y_PIN_PHASE_0, Y_PIN_PHASE_1, Y_PIN_PHASE_2, Y_PIN_PHASE_3);
#elif defined(Y_STEP_PIN)
    myMotor[Y_AXIS][0] = new StandardStepper(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN);
#else
    myMotor[Y_AXIS][0] = new Nullmotor();
#endif

#ifdef Y2_TRINAMIC_DRIVER
    #if(Y2_TRINAMIC_DRIVER == 2130)
    {
    myMotor[Y_AXIS][1] = new TrinamicDriver(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE, Y2_CS_PIN, get_next_trinamic_driver_index());
    }
    #elif(Y2_TRINAMIC_DRIVER == 2208 || Y2_TRINAMIC_DRIVER == 2209)
    {
        #ifdef HW_SERIAL_MOTORS
        myMotor[Y_AXIS][1] = new TrinamicUartDriver(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE, SERIAL_FOR_MOTORS, get_trinamic_driver_uart_address(Y2_AXIS));
        #elif defined(SW_SERIAL_MOTORS)
        myMotor[Y_AXIS][1] = new TrinamicUartDriver(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE, RX_SW_SERIAL_MOTORS, TX_SW_SERIAL_MOTORS, get_trinamic_driver_uart_address(Y2_AXIS)); 
        #else
        #error "Please define which type of serial you want to use for the trinamic motor, either HW_SERIAL_MOTORS or SW_SERIAL_MOTORS."
        #endif
    }
    #endif
#elif defined(Y2_SERVO_PIN)
    myMotor[Y_AXIS][1] = new RcServo(Y2_AXIS, Y2_SERVO_PIN, Y2_SERVO_RANGE_MIN, Y2_SERVO_RANGE_MAX);
#elif defined(Y2_UNIPOLAR)
    myMotor[Y_AXIS][1] = new UnipolarMotor(Y2_AXIS, Y2_PIN_PHASE_0, Y2_PIN_PHASE_1, Y2_PIN_PHASE_2, Y2_PIN_PHASE_3);
#elif defined(Y2_STEP_PIN)
    myMotor[Y_AXIS][1] = new StandardStepper(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN);
#else
    myMotor[Y_AXIS][1] = new Nullmotor();
#endif

    // this WILL be done better with settings
#ifdef Z_TRINAMIC_DRIVER
    #if(Z_TRINAMIC_DRIVER == 2130)
    {
    myMotor[Z_AXIS][0] = new TrinamicDriver(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE, Z_CS_PIN, get_next_trinamic_driver_index());
    }
    #elif(Z_TRINAMIC_DRIVER == 2208 || Z_TRINAMIC_DRIVER == 2209)
    {
        #ifdef HW_SERIAL_MOTORS
        myMotor[Z_AXIS][0] = new TrinamicUartDriver(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE, SERIAL_FOR_MOTORS, get_trinamic_driver_uart_address(Z_AXIS));
        #elif defined(SW_SERIAL_MOTORS)
        myMotor[Z_AXIS][0] = new TrinamicUartDriver(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE, RX_SW_SERIAL_MOTORS, TX_SW_SERIAL_MOTORS, get_trinamic_driver_uart_address(Z_AXIS)); 
        #else
        #error "Please define which type of serial you want to use for the trinamic motor, either HW_SERIAL_MOTORS or SW_SERIAL_MOTORS."
        #endif
    }
    #endif
#elif defined(Z_SERVO_PIN)
    myMotor[Z_AXIS][0] = new RcServo(Z_AXIS, Z_SERVO_PIN, Z_SERVO_RANGE_MIN, Z_SERVO_RANGE_MAX);
#elif defined(Z_UNIPOLAR)
    myMotor[Z_AXIS][0] = new UnipolarMotor(Z_AXIS, Z_PIN_PHASE_0, Z_PIN_PHASE_1, Z_PIN_PHASE_2, Z_PIN_PHASE_3);
#elif defined(Z_STEP_PIN)
    myMotor[Z_AXIS][0] = new StandardStepper(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN);
#elif defined(Z_SOLENOID_PIN)
    myMotor[Z_AXIS][0] = new Solenoid(Z_AXIS, Z_SOLENOID_PIN, Z_SOLENOID_MAX);
#else
    myMotor[Z_AXIS][0] = new Nullmotor();
#endif

#ifdef Z2_TRINAMIC_DRIVER
    #if(Z2_TRINAMIC_DRIVER == 2130)
    {
    myMotor[Z_AXIS][1] = new TrinamicDriver(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE, Z2_CS_PIN, get_next_trinamic_driver_index());
    }
    #elif(Z2_TRINAMIC_DRIVER == 2208 || Z2_TRINAMIC_DRIVER == 2209) 
    {
        #ifdef HW_SERIAL_MOTORS
        myMotor[Z_AXIS][1] = new TrinamicUartDriver(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE, SERIAL_FOR_MOTORS, get_trinamic_driver_uart_address(Z2_AXIS));
        #elif defined(SW_SERIAL_MOTORS)
        myMotor[Z_AXIS][1] = new TrinamicUartDriver(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE, RX_SW_SERIAL_MOTORS, TX_SW_SERIAL_MOTORS, get_trinamic_driver_uart_address(Z2_AXIS)); 
        #else
        #error "Please define which type of serial you want to use for the trinamic motor, either HW_SERIAL_MOTORS or SW_SERIAL_MOTORS."
        #endif
    }
    #endif
#elif defined(Z2_SERVO_PIN)
    myMotor[Z_AXIS][1] = new RcServo(Z2_AXIS, Z2_SERVO_PIN, Z2_SERVO_RANGE_MIN, Z2_SERVO_RANGE_MAX);
#elif defined(Z2_UNIPOLAR)
    myMotor[Z_AXIS][1] = new UnipolarMotor(Z2_AXIS, Z2_PIN_PHASE_0, Z2_PIN_PHASE_1, Z2_PIN_PHASE_2, Z2_PIN_PHASE_3);
#elif defined(Z2_STEP_PIN)
    myMotor[Z_AXIS][1] = new StandardStepper(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN);
#else
    myMotor[Z_AXIS][1] = new Nullmotor();
#endif

    // this WILL be done better with settings
#ifdef A_TRINAMIC_DRIVER
    myMotor[A_AXIS][0] = new TrinamicDriver(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_TRINAMIC_DRIVER, A_RSENSE, A_CS_PIN, get_next_trinamic_driver_index());
#elif defined(A_SERVO_PIN)
    myMotor[A_AXIS][0] = new RcServo(A_AXIS, A_SERVO_PIN, A_SERVO_RANGE_MIN, A_SERVO_RANGE_MAX);
#elif defined(A_UNIPOLAR)
    myMotor[A_AXIS][0] = new UnipolarMotor(A_AXIS, A_PIN_PHASE_0, A_PIN_PHASE_1, A_PIN_PHASE_2, A_PIN_PHASE_3);
#elif defined(A_STEP_PIN)
    myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN);
#else
    myMotor[A_AXIS][0] = new Nullmotor();
#endif

#ifdef A2_TRINAMIC_DRIVER
    myMotor[A_AXIS][1] = new TrinamicDriver(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_TRINAMIC_DRIVER, A2_RSENSE, A2_CS_PIN, get_next_trinamic_driver_index());
#elif defined(A2_SERVO_PIN)
    myMotor[A_AXIS][1] = new RcServo(A2_AXIS, A2_SERVO_PIN, A2_SERVO_RANGE_MIN, A2_SERVO_RANGE_MAX);
#elif defined(A2_UNIPOLAR)
    myMotor[A_AXIS][1] = new UnipolarMotor(A2_AXIS, A2_PIN_PHASE_0, A2_PIN_PHASE_1, A2_PIN_PHASE_2, A2_PIN_PHASE_3);
#elif defined(A2_STEP_PIN)
    myMotor[A_AXIS][1] = new StandardStepper(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN);
#else
    myMotor[A_AXIS][1] = new Nullmotor();
#endif

    // this WILL be done better with settings
#ifdef B_TRINAMIC_DRIVER
    myMotor[B_AXIS][0] = new TrinamicDriver(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_TRINAMIC_DRIVER, B_RSENSE, B_CS_PIN, get_next_trinamic_driver_index());
#elif defined(B_SERVO_PIN)
    myMotor[B_AXIS][0] = new RcServo(B_AXIS, B_SERVO_PIN, B_SERVO_RANGE_MIN, B_SERVO_RANGE_MAX);
#elif defined(B_UNIPOLAR)
    myMotor[B_AXIS][0] = new UnipolarMotor(B_AXIS, B_PIN_PHASE_0, B_PIN_PHASE_1, B_PIN_PHASE_2, B_PIN_PHASE_3);
#elif defined(B_STEP_PIN)
    myMotor[B_AXIS][0] = new StandardStepper(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN);
#else
    myMotor[B_AXIS][0] = new Nullmotor();
#endif

#ifdef B2_TRINAMIC_DRIVER
    myMotor[B_AXIS][1] = new TrinamicDriver(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_TRINAMIC_DRIVER, B2_RSENSE, B2_CS_PIN, get_next_trinamic_driver_index());
#elif defined(B2_SERVO_PIN)
    myMotor[B_AXIS][1] = new RcServo(B2_AXIS, B2_SERVO_PIN, B2_SERVO_RANGE_MIN, B2_SERVO_RANGE_MAX);
#elif defined(B2_UNIPOLAR)
    myMotor[B_AXIS][1] = new UnipolarMotor(B2_AXIS, B2_PIN_PHASE_0, B2_PIN_PHASE_1, B2_PIN_PHASE_2, B2_PIN_PHASE_3);
#elif defined(B2_STEP_PIN)
    myMotor[B_AXIS][1] = new StandardStepper(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN);
#else
    myMotor[B_AXIS][1] = new Nullmotor();
#endif

    // this WILL be done better with settings
#ifdef C_TRINAMIC_DRIVER
    myMotor[C_AXIS][0] = new TrinamicDriver(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_TRINAMIC_DRIVER, C_RSENSE, C_CS_PIN, get_next_trinamic_driver_index());
#elif defined(C_SERVO_PIN)
    myMotor[C_AXIS][0] = new RcServo(C_AXIS, C_SERVO_PIN, C_SERVO_RANGE_MIN, C_SERVO_RANGE_MAX);
#elif defined(C_UNIPOLAR)
    myMotor[C_AXIS][0] = new UnipolarMotor(C_AXIS, C_PIN_PHASE_0, C_PIN_PHASE_1, C_PIN_PHASE_2, C_PIN_PHASE_3);
#elif defined(C_STEP_PIN)
    myMotor[C_AXIS][0] = new StandardStepper(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN);
#else
    myMotor[C_AXIS][0] = new Nullmotor();
#endif

#ifdef C2_TRINAMIC_DRIVER
    myMotor[C_AXIS][1] = new TrinamicDriver(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_TRINAMIC_DRIVER, C2_RSENSE, C2_CS_PIN, get_next_trinamic_driver_index());
#elif defined(C2_SERVO_PIN)
    myMotor[C_AXIS][1] = new RcServo(C2_AXIS, C2_SERVO_PIN, C2_SERVO_RANGE_MIN, C2_SERVO_RANGE_MAX);
#elif defined(C2_UNIPOLAR)
    myMotor[C_AXIS][1] = new UnipolarMotor(C2_AXIS, C2_PIN_PHASE_0, C2_PIN_PHASE_1, C2_PIN_PHASE_2, C2_PIN_PHASE_3);
#elif defined(C2_STEP_PIN)
    myMotor[C_AXIS][1] = new StandardStepper(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN);
#else
    myMotor[C_AXIS][1] = new Nullmotor();
#endif

    for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
        for (uint8_t gang_index = 0; gang_index < 2; gang_index++)
            myMotor[axis][gang_index]->test();
    }

#ifdef STEPPERS_DISABLE_PIN
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "Global stepper enable pin:%d",
                   STEPPERS_DISABLE_PIN);
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

    // certain motors need features to be turned on. Check them here
    for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
        for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
            if (myMotor[axis][gang_index]->type_id == RC_SERVO_MOTOR || myMotor[axis][gang_index]->type_id == SOLENOID) 
                need_servo_task = true;
                
            if (myMotor[axis][gang_index]->type_id == UNIPOLAR_MOTOR) 
                motor_class_steps = true;
        }
    }

    if (need_servo_task) {
        xTaskCreatePinnedToCore(servoUpdateTask,     // task
                                "servoUpdateTask", // name for task
                                4096,   // size of task stack
                                NULL,   // parameters
                                1, // priority
                                &servoUpdateTaskHandle,
                                0 // core
                               );
    }
}

uint8_t get_trinamic_driver_uart_address(uint8_t axis) {
    if(axis == X_AXIS) {
        #if(X_TRINAMIC_DRIVER == 2208)
        return 0;
        #elif defined(X_DRIVER_ADDRESS)
        return X_DRIVER_ADDRESS;
        #endif
    }
    else if(axis == X2_AXIS){
        #if(X2_TRINAMIC_DRIVER == 2208)
        return 0;
        #elif defined(X2_DRIVER_ADDRESS)
        return X2_DRIVER_ADDRESS;
        #endif
    }
    else if(axis == Y_AXIS){
        #if(Y_TRINAMIC_DRIVER == 2208)
        return 0;
        #elif defined(Y_DRIVER_ADDRESS)
        return Y_DRIVER_ADDRESS;
        #endif
    }
    else if(axis == Y2_AXIS){
        #if(Y2_TRINAMIC_DRIVER == 2208)
        return 0;
        #elif defined(Y2_DRIVER_ADDRESS)
        return Y2_DRIVER_ADDRESS;
        #endif
    }
    else if(axis == Z_AXIS){
        #if(Z_TRINAMIC_DRIVER == 2208)
        return 0;
        #elif defined(Z_DRIVER_ADDRESS)
        return Z_DRIVER_ADDRESS;
        #endif
    }
    else if(axis == Z2_AXIS){
        #if(Z2_TRINAMIC_DRIVER == 2208)
        return 0;
        #elif defined(Z2_DRIVER_ADDRESS)
        return Z2_DRIVER_ADDRESS;
        #endif
    }
    return 0;
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

void servoUpdateTask(void* pvParameters) {
    TickType_t xLastWakeTime;
    const TickType_t xUpdate = SERVO_TIMER_INT_FREQ;  // in ticks (typically ms)


    xLastWakeTime = xTaskGetTickCount(); // Initialise the xLastWakeTime variable with the current time.
    while (true) { // don't ever return from this or the task dies

        //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo update");

        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
            for (uint8_t gang_index = 0; gang_index < 2; gang_index++)
                myMotor[axis][gang_index]->update();
        }

        vTaskDelayUntil(&xLastWakeTime, xUpdate);
    }
}

void motors_set_disable(bool disable) {
    static bool _current_disable = true;

    if (disable == _current_disable)
        return;

    _current_disable = disable;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "motors_set_disable(%d)", disable);

    // now step through all the motors
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++)
            myMotor[axis][gang_index]->set_disable(disable);
    }
    // global disable pin
#ifdef STEPPERS_DISABLE_PIN
    if (bit_istrue(settings.flags, BITFLAG_INVERT_ST_ENABLE)) {
        disable = !disable;    // Apply pin invert.
    }
    digitalWrite(STEPPERS_DISABLE_PIN, disable);
#endif
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
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++)
            myMotor[axis][gang_index]->set_homing_mode(is_homing);
    }
}

void motors_set_direction_pins(uint8_t onMask) {
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++)
            myMotor[axis][gang_index]->set_direction_pins(onMask);
    }
}

/*
    currently much of the step pulse I/O is done in stepper.cpp
    Some classes like UnipolarMotorClass do it themselves.
    This gives every class a notification of step.
    Note: global variable (bool motor_class_steps) decides whether this
    is needed to be called.
*/
void motors_step(uint8_t step_mask, uint8_t dir_mask) {
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++)
            myMotor[axis][gang_index]->step(step_mask, dir_mask);
    }
}

// ============================== Class Methods ================================================

Motor :: Motor() {
    type_id = MOTOR;
}

void Motor :: init() {
    _is_homing = false;
}

void Motor :: config_message() {}
void Motor :: debug_message() {}
void Motor :: read_settings() {}
void Motor :: set_disable(bool disable) {}
void Motor :: set_direction_pins(uint8_t onMask) {}
void Motor :: step(uint8_t step_mask, uint8_t dir_mask) {}
bool Motor :: test() {return true;}; // true = OK
void Motor :: update() {}

void Motor :: set_axis_name() {
    sprintf(_axis_name, "%c%s", report_get_axis_letter(axis_index), dual_axis_index ? "2" : "");
}

void Motor :: set_homing_mode(bool is_homing) {
    _is_homing = is_homing;
}

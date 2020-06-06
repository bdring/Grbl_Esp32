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
#include "StandardStepperClass.cpp"
#include "UnipolarMotorClass.cpp"
#include "RcServoClass.cpp"
//#include "SolenoidClass.cpp"

Motor* myMotor[MAX_AXES][MAX_GANGED]; // number of axes (normal and ganged)
static TaskHandle_t readSgTaskHandle = 0;   // for realtime stallguard data diaplay
static TaskHandle_t servoUpdateTaskHandle = 0;

uint8_t rmt_chan_num[MAX_AXES][MAX_GANGED];
rmt_item32_t rmtItem[2];
rmt_config_t rmtConfig;

bool motor_class_steps; // true if at least one motor class is handling steps

void init_motors() {
    bool need_servo_task = false;
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Init Motors");

#ifdef X_TRINAMIC_DRIVER
    myMotor[X_AXIS][0] = new TrinamicDriver(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_TRINAMIC_DRIVER, X_RSENSE, X_CS_PIN, get_next_trinamic_driver_index());
#elif defined(X_SERVO_PIN)
    myMotor[X_AXIS][0] = new RcServo(X_AXIS, X_SERVO_PIN, X_SERVO_RANGE_MIN, X_SERVO_RANGE_MAX);
#elif defined(X_UNIPOLAR)
    myMotor[X_AXIS][0] = new UnipolarMotor(X_AXIS, X_PIN_PHASE_0, X_PIN_PHASE_1, X_PIN_PHASE_2, X_PIN_PHASE_3);
#elif defined(X_STEP_PIN)
    myMotor[X_AXIS][0] = new StandardStepper(X_AXIS, X_STEP_PIN, X_DIRECTION_PIN, X_DISABLE_PIN);
#else
    myMotor[X_AXIS][0] = new Nullmotor();
#endif

#ifdef X2_TRINAMIC_DRIVER
    myMotor[X_AXIS][1] = new TrinamicDriver(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_TRINAMIC_DRIVER, X2_RSENSE, X2_CS_PIN, get_next_trinamic_driver_index());
#elif defined(X2_SERVO_PIN)
    myMotor[X_AXIS][1] = new RcServo(X2_AXIS, X2_SERVO_PIN, X2_SERVO_RANGE_MIN, X2_SERVO_RANGE_MAX);
#elif defined(X2_UNIPOLAR)
    myMotor[X_AXIS][1] = new UnipolarMotor(X2_AXIS, X2_PIN_PHASE_0, X2_PIN_PHASE_1, X2_PIN_PHASE_2, X2_PIN_PHASE_3);
#elif defined(X2_STEP_PIN)
    myMotor[X_AXIS][1] = new StandardStepper(X2_AXIS, X2_STEP_PIN, X2_DIRECTION_PIN, X2_DISABLE_PIN);
#else
    myMotor[X_AXIS][1] = new Nullmotor();
#endif


    // this WILL be done better with settings
#ifdef Y_TRINAMIC_DRIVER
    myMotor[Y_AXIS][0] = new TrinamicDriver(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_TRINAMIC_DRIVER, Y_RSENSE, Y_CS_PIN, get_next_trinamic_driver_index());
#elif defined(Y_SERVO_PIN)
    myMotor[Y_AXIS][0] = new RcServo(Y_AXIS, Y_SERVO_PIN, Y_SERVO_RANGE_MIN, Y_SERVO_RANGE_MAX);
#elif defined(Y_UNIPOLAR)
    myMotor[Y_AXIS][0] = new UnipolarMotor(Y_AXIS, Y_PIN_PHASE_0, Y_PIN_PHASE_1, Y_PIN_PHASE_2, Y_PIN_PHASE_3);
#elif defined(Y_STEP_PIN)
    myMotor[Y_AXIS][0] = new StandardStepper(Y_AXIS, Y_STEP_PIN, Y_DIRECTION_PIN, Y_DISABLE_PIN);
#else
    myMotor[Y_AXIS][0] = new Nullmotor();
#endif

#ifdef Y2_TRINAMIC_DRIVER
    myMotor[Y_AXIS][1] = new TrinamicDriver(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_TRINAMIC_DRIVER, Y2_RSENSE, Y2_CS_PIN, get_next_trinamic_driver_index());
#elif defined(Y2_SERVO_PIN)
    myMotor[Y_AXIS][1] = new RcServo(Y2_AXIS, Y2_SERVO_PIN, Y2_SERVO_RANGE_MIN, Y2_SERVO_RANGE_MAX);
#elif defined(Y2_UNIPOLAR)
    myMotor[Y_AXIS][1] = new UnipolarMotor(Y2_AXIS, Y2_PIN_PHASE_0, Y2_PIN_PHASE_1, Y2_PIN_PHASE_2, Y2_PIN_PHASE_3);
#elif defined(Y2_STEP_PIN)
    myMotor[Y_AXIS][1] = new StandardStepper(Y2_AXIS, Y2_STEP_PIN, Y2_DIRECTION_PIN, Y2_DISABLE_PIN);
#else
    myMotor[Y_AXIS][1] = new Nullmotor();
#endif


    // this WILL be done better with settings
#ifdef Z_TRINAMIC_DRIVER
    myMotor[Z_AXIS][0] = new TrinamicDriver(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_TRINAMIC_DRIVER, Z_RSENSE, Z_CS_PIN, get_next_trinamic_driver_index());
#elif defined(Z_SERVO_PIN)
    myMotor[Z_AXIS][0] = new RcServo(Z_AXIS, Z_SERVO_PIN, Z_SERVO_RANGE_MIN, Z_SERVO_RANGE_MAX);
#elif defined(Z_UNIPOLAR)
    myMotor[Z_AXIS][0] = new UnipolarMotor(Z_AXIS, Z_PIN_PHASE_0, Z_PIN_PHASE_1, Z_PIN_PHASE_2, Z_PIN_PHASE_3);
#elif defined(Z_STEP_PIN)
    myMotor[Z_AXIS][0] = new StandardStepper(Z_AXIS, Z_STEP_PIN, Z_DIRECTION_PIN, Z_DISABLE_PIN);
#else
    myMotor[Z_AXIS][0] = new Nullmotor();
#endif

#ifdef Z2_TRINAMIC_DRIVER
    myMotor[Z_AXIS][1] = new TrinamicDriver(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_TRINAMIC_DRIVER, Z2_RSENSE, Z2_CS_PIN, get_next_trinamic_driver_index());
#elif defined(Z2_SERVO_PIN)
    myMotor[Z_AXIS][1] = new RcServo(Z2_AXIS, Z2_SERVO_PIN, Z2_SERVO_RANGE_MIN, Z2_SERVO_RANGE_MAX);
#elif defined(Z2_UNIPOLAR)
    myMotor[Z_AXIS][1] = new UnipolarMotor(Z2_AXIS, Z2_PIN_PHASE_0, Z2_PIN_PHASE_1, Z2_PIN_PHASE_2, Z2_PIN_PHASE_3);
#elif defined(Z2_STEP_PIN)
    myMotor[Z_AXIS][1] = new StandardStepper(Z2_AXIS, Z2_STEP_PIN, Z2_DIRECTION_PIN, Z2_DISABLE_PIN);
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
    myMotor[A_AXIS][0] = new StandardStepper(A_AXIS, A_STEP_PIN, A_DIRECTION_PIN, A_DISABLE_PIN);
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
    myMotor[A_AXIS][1] = new StandardStepper(A2_AXIS, A2_STEP_PIN, A2_DIRECTION_PIN, A2_DISABLE_PIN);
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
    myMotor[B_AXIS][0] = new StandardStepper(B_AXIS, B_STEP_PIN, B_DIRECTION_PIN, B_DISABLE_PIN);
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
    myMotor[B_AXIS][1] = new StandardStepper(B2_AXIS, B2_STEP_PIN, B2_DIRECTION_PIN, B2_DISABLE_PIN);
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
    myMotor[C_AXIS][0] = new StandardStepper(C_AXIS, C_STEP_PIN, C_DIRECTION_PIN, C_DISABLE_PIN);
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
    myMotor[C_AXIS][1] = new StandardStepper(C2_AXIS, C2_STEP_PIN, C2_DIRECTION_PIN, C2_DISABLE_PIN);
#else
    myMotor[C_AXIS][1] = new Nullmotor();
#endif


#ifdef USE_STEPSTICK
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Using StepStick Mode");
#ifdef STEPPER_MS1
    pinMode(STEPPER_MS1, OUTPUT);
    digitalWrite(STEPPER_MS1, HIGH);
#endif
#ifdef STEPPER_MS2
    pinMode(STEPPER_MS2, OUTPUT);
    digitalWrite(STEPPER_MS2, HIGH);
#endif
#ifdef STEPPER_X_MS3
    pinMode(STEPPER_X_MS3, OUTPUT);
    digitalWrite(STEPPER_X_MS3, HIGH);
#endif
#ifdef STEPPER_Y_MS3
    pinMode(STEPPER_Y_MS3, OUTPUT);
    digitalWrite(STEPPER_Y_MS3, HIGH);
#endif
#ifdef STEPPER_Z_MS3
    pinMode(STEPPER_Z_MS3, OUTPUT);
    digitalWrite(STEPPER_Z_MS3, HIGH);
#endif
#ifdef STEPPER_A_MS3
    pinMode(STEPPER_A_MS3, OUTPUT);
    digitalWrite(STEPPER_A_MS3, HIGH);
#endif
#ifdef STEPPER_B_MS3
    pinMode(STEPPER_B_MS3, OUTPUT);
    digitalWrite(STEPPER_B_MS3, HIGH);
#endif
#ifdef STEPPER_C_MS3
    pinMode(STEPPER_C_MS3, OUTPUT);
    digitalWrite(STEPPER_C_MS3, HIGH);
#endif
#ifdef STEPPER_RESET
    // !RESET pin on steppers  (MISO On Schematic)
    pinMode(STEPPER_RESET, OUTPUT);
    digitalWrite(STEPPER_RESET, HIGH);
#endif
    // Note !SLEEP is set via jumper

#endif

#ifdef STEPPERS_DISABLE_PIN
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "Global stepper enable pin:%d",
                   STEPPERS_DISABLE_PIN);
    pinMode(STEPPERS_DISABLE_PIN, OUTPUT); // global motor enable pin
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
    static bool previous_state = false;

    if (previous_state == disable)
        return;

    previous_state = disable;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "motors_set_disable:%d", disable);


#ifdef USE_TRINAMIC_ENABLE
    trinamic_stepper_enable(!disable);
#endif
   if (step_enable_invert->get()) {
         disable = !disable;    // Apply pin invert.
    }
#ifdef USE_UNIPOLAR
    unipolar_disable(disable);
#endif
#ifdef STEPPERS_DISABLE_PIN
    digitalWrite(STEPPERS_DISABLE_PIN, disable);
#endif

// now loop through all the motors
    for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++)
            myMotor[axis][gang_index]->set_disable(disable);
    }
}

void motors_read_settings() {
    for (uint8_t gang_index = 0; gang_index < 2; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++) {
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
    static uint8_t previous_val = 255;  // should never be this value
    if (previous_val == onMask)
        return;
    previous_val = onMask;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "motors_set_direction_pins:0x%02X", onMask);

    for (uint8_t gang_index = 0; gang_index < MAX_GANGED; gang_index++) {
        for (uint8_t axis = X_AXIS; axis < N_AXIS; axis++)
            myMotor[axis][gang_index]->set_direction_pins(onMask);
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

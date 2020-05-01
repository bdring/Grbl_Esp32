#include "grbl.h"
#include <TMCStepper.h>

#define MOTOR_TYPE_NONE         0
#define MOTOR_TYPE_TRINAMIC     1

#define TRINAMIC_FCLK       12000000 // Hz Approx

#define TRINAMIC_RUN_MODE_STEALTHCHOP   0   // very quiet
#define TRINAMIC_RUN_MODE_COOLSTEP      1   // everything runs cooler so higher current possible
#define TRINAMIC_RUN_MODE_STALLGUARD    2   // A coolstep mode with stallguard on

// the cooolstep setting for the different modes
// TODO these should be settings
#define NORMAL_TCOOLTHRS 		0xFFFFF // 20 bit is max
#define NORMAL_THIGH 			0
#define HOMING_TCOOLTHRS 		500
#define HOMING_THIGH 			300

// TODO !!!!! Look at using homing speed for TSTEP stuff

#define TMC2130_RSENSE_DEFAULT  0.11f
#define TMC5160_RSENSE_DEFAULT  0.075f

#ifndef MOTORCLASS_H
    #define MOTORCLASS_H

class Motor {
  public:
    virtual void init(); // not in constructor because this also gets called when $$ settings change
    virtual void config_message();
    virtual void debug_message();
    virtual void read_settings();
    
    //virtual void set_enable(bool enable);
    //virtual void set_step_pin(bool step);    

    uint8_t axis_index;  // X_AXIS, etc
    uint8_t step_pin = UNDEFINED_PIN;
    uint8_t dir_pin;
    uint8_t enable_pin;
    uint8_t is_active = false;

    Motor();

};

class Nullmotor : public Motor {

};

class TrinamicDriver : public Motor{
    public:
        void config_message();
        void init();
        void set_mode(uint8_t mode);
        void read_settings();
        void trinamic_test_response();
        void trinamic_stepper_enable(bool enable);
        void debug_message();
        
        uint8_t cs_pin = UNDEFINED_PIN;  // The chip select pin (can be the same for daisy chain)
        int8_t index;                    // The indexd in the daisy chain

        TrinamicDriver(uint8_t axis_index, uint16_t driver_part_number, float r_sense, uint8_t cs_pin, int8_t index);

    private:
        TMC2130Stepper * tmcstepper; // all other driver types are subclasses of this one
        uint16_t _driver_part_number; // example: use 2130 for TMC2130
        float _r_sense;

};

void init_motors();

// returns the next spi index. We cannot preassign to axes because ganged (X2 type axes) might
// need to be inserted into the order of axes.
uint8_t get_next_trinamic_driver_index();

void readSgTask(void* pvParameters);
void motor_read_settings();

#endif
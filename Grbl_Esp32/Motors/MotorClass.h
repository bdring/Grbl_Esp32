/*
    MotorClass.h

    Header file for Motor Classes
        Motor
            Nullmotor
            StandardStepper
                TrinamicDriver
                Unipolar

    These are for motors coordinated by Grbl_ESP32

    See motorClass.cpp for more details

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

#define TRINAMIC_RUN_MODE_STEALTHCHOP   0   // very quiet
#define TRINAMIC_RUN_MODE_COOLSTEP      1   // everything runs cooler so higher current possible
#define TRINAMIC_RUN_MODE_STALLGUARD    2   // everything runs cooler so higher current possible

#define TRINAMIC_HOMING_NONE        0
#define TRINAMIC_HOMING_STALLGUARD  1

#define NORMAL_TCOOLTHRS 		0xFFFFF // 20 bit is max
#define NORMAL_THIGH 			0

#define TMC2130_RSENSE_DEFAULT  0.11f
#define TMC5160_RSENSE_DEFAULT  0.075f

// ============ defaults =================
#ifndef TRINAMIC_RUN_MODE
    #define TRINAMIC_RUN_MODE           TRINAMIC_RUN_MODE_COOLSTEP
#endif

#ifndef TRINAMIC_HOMING_MODE
    #define TRINAMIC_HOMING_MODE        TRINAMIC_HOMING_NONE
#endif

#ifndef MOTORCLASS_H
#define MOTORCLASS_H

#include "../grbl.h"
#include <TMCStepper.h> // https://github.com/teemuatlut/TMCStepper

class Motor {
  public:
    virtual void init(); // not in constructor because this also gets called when $$ settings change
    virtual void config_message();
    virtual void debug_message();
    virtual void read_settings();
    virtual void set_homing_mode(bool is_homing);
    virtual void set_disable(bool disable);
    virtual void set_direction_pins(uint8_t onMask);

    uint8_t axis_index;  // X_AXIS, etc
    uint8_t step_pin = UNDEFINED_PIN;
    uint8_t dir_pin;
    uint8_t enable_pin;
    uint8_t is_active = false;
    bool _is_homing;

protected:
    bool _invert_step_pin;

    Motor();

};

class Nullmotor : public Motor {

};

class StandardStepper : public Motor {
  public:
    virtual void config_message();
    StandardStepper();
    StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin);
    virtual void init();
    virtual void set_direction_pins(uint8_t onMask);
    void init_step_dir_pins();

    //void set_diable(bool disable);
};

class TrinamicDriver : public StandardStepper {
  public:
    void config_message();
    void init();
    void set_mode();
    void read_settings();
    void trinamic_test_response();
    void trinamic_stepper_enable(bool enable);
    void debug_message();
    void set_homing_mode(bool is_homing);
    void set_disable(bool disable);
    //void set_direction_pins(uint8_t onMask);
    //uint8_t _run_mode;
    uint8_t _homing_mode;
    uint8_t cs_pin = UNDEFINED_PIN;  // The chip select pin (can be the same for daisy chain)
    

    TrinamicDriver(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint16_t driver_part_number, float r_sense, uint8_t cs_pin, int8_t spi_index);

  private:
    TMC2130Stepper* tmcstepper;  // all other driver types are subclasses of this one
    uint16_t _driver_part_number; // example: use 2130 for TMC2130
    float _r_sense;
    int8_t spi_index;
    uint32_t calc_tstep(float speed, float percent);
};

// ========== global functions ===================

// These are used for setup and to talk to the motors as a group.

void init_motors();

// returns the next spi index. We cannot preassign to axes because ganged (X2 type axes) might
// need to be inserted into the order of axes.
uint8_t get_next_trinamic_driver_index();

void readSgTask(void* pvParameters);
void motor_read_settings();
void motors_set_homing_mode(bool is_homing);
void motors_set_disable(bool disable);
void motors_set_direction_pins(uint8_t onMask);

#endif
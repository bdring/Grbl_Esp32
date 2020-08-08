#pragma once

/*
    MotorClass.h
    Header file for Motor Classes
    Here is the hierarchy
        Motor
            Nullmotor
            StandardStepper
                TrinamicDriver
            Unipolar
            RC Servo

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

#include "../grbl.h"
#include <TMCStepper.h>  // https://github.com/teemuatlut/TMCStepper
#include "TrinamicDriverClass.h"
#include "RcServoClass.h"
//#include "SolenoidClass.h"

extern uint8_t      rmt_chan_num[MAX_AXES][2];
extern rmt_item32_t rmtItem[2];
extern rmt_config_t rmtConfig;

typedef enum { MOTOR, NULL_MOTOR, STANDARD_MOTOR, TRINAMIC_SPI_MOTOR, UNIPOLAR_MOTOR, RC_SERVO_MOTOR, SOLENOID } motor_class_id_t;

// These are used for setup and to talk to the motors as a group.
void    init_motors();
uint8_t get_next_trinamic_driver_index();
bool    motors_have_type_id(motor_class_id_t id);
void    readSgTask(void* pvParameters);
void    motors_read_settings();
void    motors_set_homing_mode(uint8_t homing_mask, bool isHoming);
void    motors_set_disable(bool disable);
void    motors_set_direction_pins(uint8_t onMask);
void    motors_step(uint8_t step_mask, uint8_t dir_mask);
void    servoUpdateTask(void* pvParameters);

extern bool motor_class_steps;  // true if at least one motor class is handling steps

// ==================== Motor Classes ====================

class Motor {
public:
    Motor();

    virtual void init();  // not in constructor because this also gets called when $$ settings change
    virtual void config_message();
    virtual void debug_message();
    virtual void read_settings();
    virtual void set_homing_mode(uint8_t homing_mask, bool isHoming);
    virtual void set_disable(bool disable);
    virtual void set_direction_pins(uint8_t onMask);
    virtual void step(uint8_t step_mask, uint8_t dir_mask);  // only used on Unipolar right now
    virtual bool test();
    virtual void set_axis_name();
    virtual void update();

    motor_class_id_t type_id;
    uint8_t          is_active = false;

protected:
    uint8_t axis_index;       // X_AXIS, etc
    uint8_t dual_axis_index;  // 0 = primary 1=ganged

    bool    _showError;
    bool    _use_mpos = true;
    uint8_t _homing_mask;
    char    _axis_name[10];  // this the name to use when reporting like "X" or "X2"
};

class Nullmotor : public Motor {};

class StandardStepper : public Motor {
public:
    StandardStepper();
    StandardStepper(uint8_t axis_index, uint8_t step_pin, uint8_t dir_pin, uint8_t disable_pin);

    virtual void config_message();
    virtual void init();
    virtual void set_direction_pins(uint8_t onMask);
    void         init_step_dir_pins();
    virtual void set_disable(bool disable);
    uint8_t      step_pin;

protected:
    bool    _invert_step_pin;
    uint8_t dir_pin;
    uint8_t disable_pin;
};

class TrinamicDriver : public StandardStepper {
public:
    TrinamicDriver(uint8_t  axis_index,
                   uint8_t  step_pin,
                   uint8_t  dir_pin,
                   uint8_t  disable_pin,
                   uint8_t  cs_pin,
                   uint16_t driver_part_number,
                   float    r_sense,
                   int8_t   spi_index);

    void config_message();
    void init();
    void set_mode(bool isHoming);
    void read_settings();
    void trinamic_test_response();
    void trinamic_stepper_enable(bool enable);
    void debug_message();
    void set_homing_mode(uint8_t homing_mask, bool ishoming);
    void set_disable(bool disable);
    bool test();

private:
    uint32_t calc_tstep(float speed, float percent);

    TMC2130Stepper* tmcstepper;  // all other driver types are subclasses of this one
    uint8_t         _homing_mode;
    uint8_t         cs_pin = UNDEFINED_PIN;  // The chip select pin (can be the same for daisy chain)
    uint16_t        _driver_part_number;     // example: use 2130 for TMC2130
    float           _r_sense;
    int8_t          spi_index;

protected:
    uint8_t _mode;
    uint8_t _lastMode = 255;
};

class UnipolarMotor : public Motor {
public:
    UnipolarMotor();
    UnipolarMotor(uint8_t axis_index, uint8_t pin_phase0, uint8_t pin_phase1, uint8_t pin_phase2, uint8_t pin_phase3);
    void init();
    void config_message();
    void set_disable(bool disable);
    void step(uint8_t step_mask, uint8_t dir_mask);  // only used on Unipolar right now

private:
    uint8_t _pin_phase0;
    uint8_t _pin_phase1;
    uint8_t _pin_phase2;
    uint8_t _pin_phase3;
    uint8_t _current_phase;
    bool    _half_step;
    bool    _enabled;
};

class RcServo : public Motor {
public:
    RcServo();
    RcServo(uint8_t axis_index, uint8_t pwm_pin, float min, float max);
    virtual void config_message();
    virtual void init();
    void         _write_pwm(uint32_t duty);
    virtual void set_disable(bool disable);
    virtual void update();
    void         read_settings();
    void         set_homing_mode(bool is_homing, bool isHoming);

protected:
    void set_location();
    void _get_calibration();

    uint8_t  _pwm_pin;
    uint8_t  _channel_num;
    uint32_t _current_pwm_duty;
    bool     _disabled;

    float _position_min;
    float _position_max;  // position in millimeters
    float _homing_position;

    float _pwm_pulse_min;
    float _pwm_pulse_max;
};

class Solenoid : public RcServo {
public:
    Solenoid();
    Solenoid(uint8_t axis_index, gpio_num_t pwm_pin, float transition_poiont);
    void config_message();
    void set_location();
    void update();
    void init();
    void set_disable(bool disable);

    float _transition_poiont;
};

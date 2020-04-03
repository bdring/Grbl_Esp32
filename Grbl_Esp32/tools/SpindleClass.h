#include "grbl.h"

#ifndef SPINDLE_CLASS_H
#define SPINDLE_CLASS_H

class Spindle {
  public:
    virtual void init();
    virtual float set_rpm(float rpm);
    virtual void set_pwm(uint32_t duty);
    virtual void set_state(uint8_t state, float rpm);
    virtual uint8_t get_state();
    virtual void stop();
    virtual void config_message();
    virtual bool isRateAdjusted();

    //bool _isRateAdjused = false;

};

class PWMSpindle : public Spindle {
  public:
    void init();
    float set_rpm(float rpm);
    void set_state(uint8_t state, float rpm);
    uint8_t get_state();
    void stop();
    void config_message();
    virtual void set_pwm(uint32_t duty);

  private:
    int8_t _spindle_pwm_chan_num;
    int32_t _current_pwm_duty;

    float _pwm_gradient; // Precalulated value to speed up rpm to PWM conversions.
    uint32_t _pwm_period; // how many counts in 1 period
    uint32_t _pwm_off_value;
    uint32_t _pwm_min_value;
    uint32_t _pwm_max_value;

    
    void set_enable_pin(bool enable_pin);
    void set_spindle_dir_pin(bool Clockwise);
};

class RelaySpindle : public PWMSpindle {
  public:
    void init();
    void config_message();
    void set_pwm(uint32_t duty);
private:
    
};

class Laser : public PWMSpindle {
    public:
    bool isRateAdjusted(); 
};


#endif
/*
 Important ESC Settings
 $33=50 // Hz this is the typical good frequency for an ESC
 #define DEFAULT_SPINDLE_FREQ 5000.0 // $33 Hz (extended set)

 Determine the typical min and max pulse length of your ESC
  min_pulse is typically 1ms (0.001 sec) or less
  max_pulse is typically 2ms (0.002 sec) or more

 determine PWM_period. It is (1/freq) if freq = 50...period = 0.02

 determine pulse length for min_pulse and max_pulse in percent.

  (pulse / PWM_period)
   min_pulse = (0.001 / 0.02) = 0.05 = 5%  so ... $34 and $35 = 5.0
   max_pulse = (0.002 / .02) = 0.1 = 10%  so ... $36=10
*/

#include "grbl.h"
#include "SpindleClass.h"


// don't change these
#define BESC_PWM_FREQ           50.0 // Hz 
#define BESC_PWM_BIT_PRECISION  16   // bits
#define BESC_DEFAULT_MIN_PULSE  5.0 // percent
#define BESC_DEFAULT_MAX_PULSE  10.0 // percent


void BESCSpindle :: init() {

    get_pin_numbers();

    if (_output_pin == UNDEFINED_PIN) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: BESC output pin not defined");
        return; // We cannot continue without the output pin
    }

    // override some settings to what is required for a BESC
    _pwm_freq = BESC_PWM_FREQ;
    _pwm_precision = 16;

    // to do make these tweakable using existing setting as percentages on these defaults
    _pwm_off_value = BESC_DEFAULT_MIN_PULSE;
    _pwm_min_value = BESC_DEFAULT_MIN_PULSE;
    _pwm_max_value = BESC_DEFAULT_MAX_PULSE;

    ledcSetup(_spindle_pwm_chan_num, (double)_pwm_freq, _pwm_precision); // setup the channel
    ledcAttachPin(_output_pin, _spindle_pwm_chan_num); // attach the PWM to the pin

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    config_message();
}

// prints the startup message of the spindle config
void BESCSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "BESC spindle on GPIO %d", _output_pin);
}

float BESCSpindle::set_rpm(float rpm) {
    uint32_t pwm_value;

    if (_output_pin == UNDEFINED_PIN)
        return rpm;

    // apply speed overrides
    rpm *= (0.010 * sys.spindle_speed_ovr); // Scale by spindle speed override value (percent)  

    // apply limits limits
    if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        rpm = _max_rpm;
    } else if (rpm != 0.0 && rpm <= _min_rpm) {        
        rpm = _min_rpm;
    }

    sys.spindle_speed = rpm;
    
    // determine the pwm value
    if (rpm == 0.0) {
        pwm_value = _pwm_off_value;
    } else if (rpm == _min_rpm) {
        pwm_value = _pwm_min_value;
    } else if (rpm == _max_rpm) {
        pwm_value = _pwm_max_value;
    } else {
        pwm_value = floor((rpm - _min_rpm) * _pwm_gradient) + _pwm_min_value;
    }

#ifdef  SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    set_enable_pin(rpm != 0);
#endif

    set_pwm(pwm_value);
    return rpm;
}
/*
    SpindleClass.cpp

    A Spindle Class
        Spindle         - A base class. Do not use
        PWMSpindle     - A spindle with a PWM output
        RelaySpindle    - An on/off only spindle
        Laser           - Output is PWM, but the M4 laser power mode can be used
        DacSpindle      - Uses the DAC to output a 0-3.3V output

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
        Consider breaking into one file per class.

    Get rid of dependance on machine definition #defines
        SPINDLE_PWM_BIT_PRECISION
        SPINDLE_PWM_PIN
        SPINDLE_ENABLE_PIN
        SPINDLE_DIR_PIN

*/
#include "grbl.h"
#include "SpindleClass.h"

bool Spindle::isRateAdjusted() {
    return false; // default for basic spindles is false
}

void Spindle :: spindle_sync(uint8_t state, float rpm) {
    if (sys.state == STATE_CHECK_MODE)
        return;
    protocol_buffer_synchronize(); // Empty planner buffer to ensure spindle is set when programmed.
    set_state(state, rpm);
}

// ======================= NullSpindle ==============================
// NullSpindle is just bunch of do nothing (ignore) methods to be used when you don't want a spindle
void NullSpindle :: init() {
    config_message();
}
float NullSpindle :: set_rpm(float rpm) {
    return rpm;
}
void NullSpindle :: set_state(uint8_t state, float rpm) {}
uint8_t NullSpindle :: get_state() {
    return (SPINDLE_STATE_DISABLE);
}
void NullSpindle :: stop() {}
void NullSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "No spindle");
}



// ======================= PWMSpindle ==============================
void PWMSpindle::init() {

    get_pin_numbers();

    if (_output_pin == UNDEFINED_PIN) {
        return; // We cannot continue without the output pin
    }

    _pwm_freq = settings.spindle_pwm_freq;
    _pwm_period = ((1 << SPINDLE_PWM_BIT_PRECISION) - 1);

    if (settings.spindle_pwm_min_value > settings.spindle_pwm_min_value)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Spindle min pwm is greater than max. Check $35 and $36");

    if ((F_TIMERS / _pwm_freq) < _pwm_period)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning spindle PWM precision (%d bits) too high for frequency (%.2f Hz)", SPINDLE_PWM_BIT_PRECISION, _pwm_freq);

    // pre-caculate some PWM count values
    _pwm_off_value = (_pwm_period * settings.spindle_pwm_off_value / 100.0);
    _pwm_min_value = (_pwm_period * settings.spindle_pwm_min_value / 100.0);
    _pwm_max_value = (_pwm_period * settings.spindle_pwm_max_value / 100.0);

#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
    _min_rpm = RPM_MIN;
    _max_rpm = RPM_MAX;
#else
    _min_rpm = settings.rpm_min;
    _max_rpm = settings.rpm_max;
#endif
    // The pwm_gradient is the pwm duty cycle units per rpm
    _pwm_gradient = (_pwm_max_value - _pwm_min_value) / (_max_rpm - _min_rpm);

    _spindle_pwm_chan_num = sys_get_next_PWM_chan_num();
    ledcSetup(_spindle_pwm_chan_num, (double)_pwm_freq, SPINDLE_PWM_BIT_PRECISION); // setup the channel
    ledcAttachPin(_output_pin, _spindle_pwm_chan_num); // attach the PWM to the pin    

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    if (_direction_pin != UNDEFINED_PIN)
        pinMode(_direction_pin, OUTPUT);

    config_message();
}

// Get the GPIO from the machine definition
void PWMSpindle :: get_pin_numbers() {
    // setup all the pins

#ifdef SPINDLE_PWM_PIN
    _output_pin = SPINDLE_PWM_PIN;
#else
    _output_pin = UNDEFINED_PIN;
#endif

#ifdef SPINDLE_ENABLE_PIN
    _enable_pin = SPINDLE_ENABLE_PIN;
#else
    _enable_pin = UNDEFINED_PIN;
#endif

#ifdef SPINDLE_DIR_PIN
    _direction_pin = SPINDLE_DIR_PIN;
#else
    _direction_pin = UNDEFINED_PIN;
#endif

    if (_output_pin == UNDEFINED_PIN)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Spindle output pin not defined");

}

float PWMSpindle::set_rpm(float rpm) {
    if (_output_pin == UNDEFINED_PIN)
        return rpm;

    uint32_t pwm_value;

    // apply overrides and limits
    rpm *= (0.010 * sys.spindle_speed_ovr); // Scale by spindle speed override value (percent)

    // Calculate PWM register value based on rpm max/min settings and programmed rpm.
    if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        // No PWM range possible. Set simple on/off spindle control pin state.
        sys.spindle_speed = _max_rpm;
        pwm_value = _pwm_max_value;
    } else if (rpm <= _min_rpm) {
        if (rpm == 0.0) { // S0 disables spindle
            sys.spindle_speed = 0.0;
            pwm_value = _pwm_off_value;
        } else { // Set minimum PWM output
            rpm = _min_rpm;
            sys.spindle_speed = rpm;
            pwm_value = _pwm_min_value;
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM less than min RPM:%5.2f %d", rpm, pwm_value);
        }
    } else {
        // Compute intermediate PWM value with linear spindle speed model.
        // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
        sys.spindle_speed = rpm;
#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
        pwm_value = piecewise_linear_fit(rpm);
#else
        pwm_value = floor((rpm - _min_rpm) * _pwm_gradient) + _pwm_min_value;
#endif
    }

#ifdef  SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    set_enable_pin(rpm != 0);
#endif

    set_pwm(pwm_value);

    return rpm;
}

void PWMSpindle::set_state(uint8_t state, float rpm) {

    if (sys.abort)
        return;   // Block during abort.

    if (state == SPINDLE_DISABLE) { // Halt or set spindle direction and rpm.
        sys.spindle_speed = 0.0;
        stop();
    } else {
        set_spindle_dir_pin(state == SPINDLE_ENABLE_CW);
        set_enable_pin(true);
        set_rpm(rpm);
    }
    sys.report_ovr_counter = 0; // Set to report change immediately
}

uint8_t PWMSpindle::get_state() {


    if (_current_pwm_duty == 0  || _output_pin == UNDEFINED_PIN)
        return (SPINDLE_STATE_DISABLE);
    else {
        if (_direction_pin != UNDEFINED_PIN) {
            if (digitalRead(_direction_pin))
                return (SPINDLE_STATE_CW);
            else
                return (SPINDLE_STATE_CCW);
        } else
            return (SPINDLE_STATE_CW);
    }
}

void PWMSpindle::stop() {
    // inverts are delt with in methods
    set_enable_pin(false);
    set_pwm(0);
}

// prints the startup message of the spindle config
void PWMSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "PWM spindle on GPIO %d, freq %.2fHz, Res %d bits", _output_pin, _pwm_freq, SPINDLE_PWM_BIT_PRECISION);
}


void PWMSpindle::set_pwm(uint32_t duty) {
    if (_output_pin == UNDEFINED_PIN)
        return;

    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;

#ifdef INVERT_SPINDLE_PWM
    duty = (1 << SPINDLE_PWM_BIT_PRECISION) - duty;
#endif
    ledcWrite(_spindle_pwm_chan_num, duty);
}

void PWMSpindle::set_enable_pin(bool enable) {
    if (_enable_pin == UNDEFINED_PIN)
        return;
#ifndef INVERT_SPINDLE_ENABLE_PIN
    digitalWrite(_enable_pin, enable);
#else
    digitalWrite(_enable_pin, !enable);
#endif
}

void PWMSpindle::set_spindle_dir_pin(bool Clockwise) {
    if (_direction_pin != UNDEFINED_PIN)
        digitalWrite(_direction_pin, Clockwise);
}

// ========================= RelaySpindle ==================================
/*
    This is the same as a PWM spindle, but is a digital rather than PWM output
*/
void RelaySpindle::init() {
    get_pin_numbers();
    if (_output_pin == UNDEFINED_PIN)
        return;

    pinMode(_output_pin, OUTPUT);

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(SPINDLE_ENABLE_PIN, OUTPUT);

    if (_direction_pin != UNDEFINED_PIN)
        pinMode(_direction_pin, OUTPUT);

    config_message();
}

// prints the startup message of the spindle config
void RelaySpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Relay spindle on GPIO %d", _output_pin);
}

void RelaySpindle::set_pwm(uint32_t duty) {
#ifdef INVERT_SPINDLE_PWM
    duty = (duty == 0); // flip duty
#endif
    digitalWrite(_output_pin, duty > 0); // anything greater
}


// ===================================== Laser ==============================================


bool Laser :: isRateAdjusted() {
    // must be in $32=1 (laser mode)
    return (settings.flags & BITFLAG_LASER_MODE);
}

void Laser :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "Laser spindle on GPIO:%d, Freq:%.2fHz, Res:%dbits Laser mode:$32=%d",
                   _output_pin,
                   _pwm_freq,
                   SPINDLE_PWM_BIT_PRECISION,
                   isRateAdjusted());  // the current mode
}

// ======================================== DacSpindle ======================================
void DacSpindle :: init() {
    get_pin_numbers();
    if (_output_pin == UNDEFINED_PIN)
        return;
    
    _min_rpm = settings.rpm_min;
    _max_rpm = settings.rpm_max;
    _pwm_min_value = 0;     // not actually PWM...DAC counts
    _pwm_max_value = 255;   // not actually PWM...DAC counts
    _gpio_ok = true;

    if (_output_pin != GPIO_NUM_25 && _output_pin != GPIO_NUM_26) { // DAC can only be used on these pins 
        _gpio_ok = false;
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DAC spindle pin invalid GPIO_NUM_%d", _output_pin);
        return;
    }    

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    if (_direction_pin != UNDEFINED_PIN)
        pinMode(_direction_pin, OUTPUT);

    config_message();
}

void DacSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DAC spindle on GPIO %d", _output_pin);
}

float DacSpindle::set_rpm(float rpm) {
    if (_output_pin == UNDEFINED_PIN)
        return rpm;

    uint32_t pwm_value;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set RPM %5.1f", rpm);

    // apply overrides and limits
    rpm *= (0.010 * sys.spindle_speed_ovr); // Scale by spindle speed override value (percent)

    // Calculate PWM register value based on rpm max/min settings and programmed rpm.
    if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        // No PWM range possible. Set simple on/off spindle control pin state.
        sys.spindle_speed = _max_rpm;
        pwm_value = 255;
    } else if (rpm <= _min_rpm) {
        if (rpm == 0.0) { // S0 disables spindle
            sys.spindle_speed = 0.0;
            pwm_value = 0;
        } else { // Set minimum PWM output
            rpm = _min_rpm;
            sys.spindle_speed = rpm;
            pwm_value = 0;
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM less than min RPM:%5.2f %d", rpm, pwm_value);
        }
    } else {
        // Compute intermediate PWM value with linear spindle speed model.
        // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
        sys.spindle_speed = rpm;
        
        pwm_value = (uint32_t)map_float(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
    }

#ifdef  SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    set_enable_pin(rpm != 0);
#endif

    set_pwm(pwm_value);

    return rpm;
}

void DacSpindle :: set_pwm(uint32_t duty) {
    if (_gpio_ok) {
         dacWrite(_output_pin, (uint8_t)duty);
    }
       
}

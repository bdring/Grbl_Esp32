/*
    PWMSpindle.cpp

    This is a full featured TTL PWM spindle. This does not include speed/power
    compensation. Use the Laser class for that.

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
#include "grbl.h"
#include "SpindleClass.h"

// ======================= PWMSpindle ==============================
void PWMSpindle::init() {

    get_pin_numbers();

    if (_output_pin == UNDEFINED_PIN) {
        return; // We cannot continue without the output pin
    }

    _min_rpm = grbl_preferences.getFloat("SPIN_MAX_RPM", DEFAULT_SPINDLE_MIN_VALUE);

    _pwm_freq = settings.spindle_pwm_freq;
    _pwm_period = ((1 << settings.spindle_pwm_precision_bits) - 1);

    if (settings.spindle_pwm_min_value > settings.spindle_pwm_min_value)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Spindle min pwm is greater than max. Check $35 and $36");

    if ((F_TIMERS / _pwm_freq) < _pwm_period)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning spindle PWM precision (%d bits) too high for frequency (%.2f Hz)", settings.spindle_pwm_precision_bits, _pwm_freq);

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
    ledcSetup(_spindle_pwm_chan_num, (double)_pwm_freq, settings.spindle_pwm_precision_bits); // setup the channel
    ledcAttachPin(_output_pin, _spindle_pwm_chan_num); // attach the PWM to the pin    

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    if (_direction_pin != UNDEFINED_PIN)
        pinMode(_direction_pin, OUTPUT);

    is_reversable = (_direction_pin != UNDEFINED_PIN);

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

    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM:%5.2f Min:%5.2f Max:%5.2f", rpm, _min_rpm, _max_rpm);

    uint32_t pwm_value;

    // apply overrides and limits
    rpm *= (0.010 * sys.spindle_speed_ovr); // Scale by spindle speed override value (percent)
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM 1:%5.2f", rpm);

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
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM 2:%5.2f", rpm);
            sys.spindle_speed = rpm;
            pwm_value = _pwm_min_value;
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM less than min RPM:%5.2f %d", rpm, pwm_value);
        }
    } else {
        // Compute intermediate PWM value with linear spindle speed model.
        // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
        sys.spindle_speed = rpm;
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM 3:%5.2f", rpm);
#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
        pwm_value = piecewise_linear_fit(rpm);
#else
        pwm_value = floor((rpm - _min_rpm) * _pwm_gradient) + _pwm_min_value;
#endif
    }

#ifdef  SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    set_enable_pin(rpm != 0);
#endif

    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Spindle RPM Final:%5.2f", rpm);
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
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "PWM spindle on GPIO %d, freq %.2fHz, Res %d bits", _output_pin, _pwm_freq, settings.spindle_pwm_precision_bits);
}


void PWMSpindle::set_pwm(uint32_t duty) {
    if (_output_pin == UNDEFINED_PIN)
        return;

    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;

#ifdef INVERT_SPINDLE_PWM
    duty = (1 << settings.spindle_pwm_precision_bits) - duty;
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
/*
    SpindleClass.cpp

    A Spindle Class
        Spindle         - A base class. Do not use
        PWMSpindel      - A spindle with a PWM output
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

*/
#include "grbl.h"
#include "SpindleClass.h"

bool Spindle::isRateAdjusted() {
    return false; // default for basic spindles is false
}

// ======================= NullSpindle ==============================
// A bunch of do nothing (ignore) methods
void NullSpindle :: init() {
    config_message();
}
float NullSpindle :: set_rpm(float rpm) {
    return rpm;
}
void NullSpindle :: set_pwm(uint32_t duty) {}
void NullSpindle :: set_state(uint8_t state, float rpm) {}
uint8_t NullSpindle :: get_state() {
    return (SPINDLE_STATE_DISABLE);
}
void NullSpindle :: stop() {}
void NullSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "No spindle (NullSpindle)");
}

// ======================= PWMSpindle ==============================

void PWMSpindle::init() {
    _pwm_freq = settings.spindle_pwm_freq;
    _pwm_period = ((1 << SPINDLE_PWM_BIT_PRECISION) - 1);

    if (settings.spindle_pwm_min_value > settings.spindle_pwm_min_value)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning spindle min pwm is greater than max. Check $35 and $36");

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
    ledcAttachPin(SPINDLE_PWM_PIN, _spindle_pwm_chan_num); // attach the PWM to the pin
    config_message();

#ifdef SPINDLE_ENABLE_PIN
    pinMode(SPINDLE_ENABLE_PIN, OUTPUT);
#endif
#ifdef SPINDLE_DIR_PIN
    pinMode(SPINDLE_DIR_PIN, OUTPUT);
#endif
}

float PWMSpindle::set_rpm(float rpm) {
    uint32_t pwm_value;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set RPM %5.1f", rpm);

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

    set_pwm(pwm_value);
    return rpm;
}

void PWMSpindle::set_state(uint8_t state, float rpm) {

    if (sys.abort)
        return;   // Block during abort.

    if (state == SPINDLE_DISABLE) { // Halt or set spindle direction and rpm.
        sys.spindle_speed = 0.0;
        spindle_stop();
    } else {
        set_spindle_dir_pin(state == SPINDLE_ENABLE_CW);
        set_rpm(rpm);
    }
    sys.report_ovr_counter = 0; // Set to report change immediately
}

uint8_t PWMSpindle::get_state() {


    if (_current_pwm_duty == 0) // Check the PWM value
        return (SPINDLE_STATE_DISABLE);
    else {
#ifdef SPINDLE_DIR_PIN
        if (digitalRead(SPINDLE_DIR_PIN))
            return (SPINDLE_STATE_CW);
        else
            return (SPINDLE_STATE_CCW);
#else
        return (SPINDLE_STATE_CW);
#endif
    }
}

void PWMSpindle::stop() {
    // inverts are delt with in methods
    set_enable_pin(false);
    set_pwm(0);
}

// prints the startup message of the spindle config
void PWMSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "PWM spindle on GPIO %d, freq %.2fHz, Res %d bits", SPINDLE_PWM_PIN, _pwm_freq, SPINDLE_PWM_BIT_PRECISION);
    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "PWM Off:%d Min:%d Max:%d", _pwm_off_value, _pwm_min_value, _pwm_max_value);
}


void PWMSpindle::set_pwm(uint32_t duty) {
    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;

#ifdef INVERT_SPINDLE_PWM
    duty = (1 << SPINDLE_PWM_BIT_PRECISION) - duty;
#endif
    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set PWM %d of %d", duty, (1 << SPINDLE_PWM_BIT_PRECISION) - 1);
    ledcWrite(_spindle_pwm_chan_num, duty);
}

void PWMSpindle::set_enable_pin(bool enable) {
#ifdef SPINDLE_ENABLE_PIN
#ifndef INVERT_SPINDLE_ENABLE_PIN
    digitalWrite(SPINDLE_ENABLE_PIN, enable);
#else
    digitalWrite(SPINDLE_ENABLE_PIN, !enable);
#endif
#endif
}

void PWMSpindle::set_spindle_dir_pin(bool Clockwise) {
#ifdef SPINDLE_DIR_PIN
    digitalWrite(SPINDLE_DIR_PIN, state == SPINDLE_ENABLE_CW);
#endif
}

// ========================= RelaySpindle ==================================
/*
    This is the same as a PWM spindle, but is a digital rather than PWM output
*/

void RelaySpindle::init() {
    pinMode(SPINDLE_PWM_PIN, OUTPUT);
#ifdef SPINDLE_ENABLE_PIN
    pinMode(SPINDLE_ENABLE_PIN, OUTPUT);
#endif
#ifdef SPINDLE_DIR_PIN
    pinMode(SPINDLE_DIR_PIN, OUTPUT);
#endif
    config_message();
}

// prints the startup message of the spindle config
void RelaySpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Relay spindle on GPIO %d", SPINDLE_PWM_PIN);
}

void RelaySpindle::set_pwm(uint32_t duty) {
#ifdef INVERT_SPINDLE_PWM
    duty = (duty == 0); // flip duty
#endif
    digitalWrite(SPINDLE_PWM_PIN, duty > 0); // anything greater
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
                   SPINDLE_PWM_PIN,
                   _pwm_freq,
                   SPINDLE_PWM_BIT_PRECISION,
                   isRateAdjusted());  // the current mode
}

// ======================================== DacSpindle ======================================
void DacSpindle :: init() {
    _pwm_period = ((1 << SPINDLE_PWM_BIT_PRECISION) - 1);
    _dac_channel_num = (dac_channel_t)0;
    _gpio_ok = true;
    switch (SPINDLE_PWM_PIN) {
    case GPIO_NUM_25:
        _dac_channel_num = DAC_CHANNEL_1;
        break;
    case GPIO_NUM_26:
        _dac_channel_num = DAC_CHANNEL_1;
        break;
    default:
        _gpio_ok = false;
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DAC spindle pin invalid GPIO_NUM_%d", SPINDLE_PWM_PIN);
        return; // skip config message
    }
    config_message();
}

void DacSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "DAC spindle on GPIO %d", SPINDLE_PWM_PIN);
}

void DacSpindle :: set_pwm(uint32_t duty) {
    // remap duty over DAC's 0-255 range
    duty = map(duty, 0, _pwm_period, 0, 255);
    if (_gpio_ok)
        dac_output_voltage(_dac_channel_num, (uint8_t)duty);
}



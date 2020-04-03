/*


Testing
    Should $G show actual speed 
*/
#include "grbl.h"
#include "SpindleClass.h"

bool Spindle::isRateAdjusted() {
    return false; // default for basic spindles is false
}

void PWMSpindle::init() {
    _pwm_period = ((1 << SPINDLE_PWM_BIT_PRECISION) - 1);
    if (settings.spindle_pwm_min_value > settings.spindle_pwm_min_value)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning spindle min pwm is greater than max. Check $35 and $36");
    // pre-caculate some PWM count values
    _pwm_off_value = (_pwm_period * settings.spindle_pwm_off_value / 100.0);
    _pwm_min_value = (_pwm_period * settings.spindle_pwm_min_value / 100.0);
    _pwm_max_value = (_pwm_period * settings.spindle_pwm_max_value / 100.0);
    // The pwm_gradient is the pwm duty cycle units per rpm
    _pwm_gradient = (_pwm_max_value - _pwm_min_value) / (settings.rpm_max - settings.rpm_min);


    _spindle_pwm_chan_num = sys_get_next_PWM_chan_num();
    ledcSetup(_spindle_pwm_chan_num, (double)settings.spindle_pwm_freq, SPINDLE_PWM_BIT_PRECISION); // setup the channel
    ledcAttachPin(SPINDLE_PWM_PIN, _spindle_pwm_chan_num); // attach the PWM to the pin

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
    if ((settings.rpm_min >= settings.rpm_max) || (rpm >= settings.rpm_max)) {
        // No PWM range possible. Set simple on/off spindle control pin state.
        sys.spindle_speed = settings.rpm_max;
        pwm_value = _pwm_max_value;
    } else if (rpm <= settings.rpm_min) {
        if (rpm == 0.0) { // S0 disables spindle
            sys.spindle_speed = 0.0;
            pwm_value = _pwm_off_value;
        } else { // Set minimum PWM output
            sys.spindle_speed = settings.rpm_min;
            pwm_value = _pwm_min_value;
        }
    } else {
        // Compute intermediate PWM value with linear spindle speed model.
        // NOTE: A nonlinear model could be installed here, if required, but keep it VERY light-weight.
        sys.spindle_speed = rpm;
#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
        pwm_value = piecewise_linear_fit(rpm);
#else
        pwm_value = floor((rpm - settings.rpm_min) * _pwm_gradient) + _pwm_min_value;
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
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "PWM spindle on GPIO %d, freq %.2fHz, Res %d bits", SPINDLE_PWM_PIN, settings.spindle_pwm_freq, SPINDLE_PWM_BIT_PRECISION);
}


void PWMSpindle::set_pwm(uint32_t duty) {
    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;

#ifdef INVERT_SPINDLE_PWM
    duty = (1 << SPINDLE_PWM_BIT_PRECISION) - duty;
#endif
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set PWM %d of %d", duty, (1 << SPINDLE_PWM_BIT_PRECISION)-1);
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


// ====================== Laser =============================


bool Laser :: isRateAdjusted() {
    // must be in $32=1 (laser mode) and M4 (CCW rotation)
    return ( settings.flags & BITFLAG_LASER_MODE );
}

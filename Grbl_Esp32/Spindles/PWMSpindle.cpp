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

// ======================= PWMSpindle ==============================
/*
    This gets called at startup or whenever a spindle setting changes
    If the spindle is running it will stop and need to be restarted with M3Snnnn
*/
void PWMSpindle::init() {

    get_pins_and_settings();

    if (_output_pin == UNDEFINED_PIN) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Spindle output pin not defined");
        return; // We cannot continue without the output pin
    }

    ledcSetup(_spindle_pwm_chan_num, (double)_pwm_freq, _pwm_precision); // setup the channel
    ledcAttachPin(_output_pin, _spindle_pwm_chan_num); // attach the PWM to the pin

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    if (_direction_pin != UNDEFINED_PIN)
        pinMode(_direction_pin, OUTPUT);

    config_message();
}

// Get the GPIO from the machine definition
void PWMSpindle :: get_pins_and_settings() {
    // setup all the pins

#ifdef SPINDLE_OUTPUT_PIN
    _output_pin = SPINDLE_OUTPUT_PIN;
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
    is_reversable = (_direction_pin != UNDEFINED_PIN);

    _pwm_freq = settings.spindle_pwm_freq;
    _pwm_precision = calc_pwm_precision(_pwm_freq); // detewrmine the best precision
    _pwm_period = (1 << _pwm_precision);

    if (settings.spindle_pwm_min_value > settings.spindle_pwm_min_value)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Spindle min pwm is greater than max. Check $35 and $36");

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

    _spindle_pwm_chan_num = 0; // Channel 0 is reserved for spindle use


}

float PWMSpindle::set_rpm(float rpm) {
    uint32_t pwm_value;

    if (_output_pin == UNDEFINED_PIN)
        return rpm;

    // apply override
    rpm *= (0.010 * sys.spindle_speed_ovr); // Scale by spindle speed override value (percent)

    // apply limits
    if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        rpm = _max_rpm;
    } else if (rpm != 0.0 && rpm <= _min_rpm) {
        rpm = _min_rpm;
    }

    sys.spindle_speed = rpm;

#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
    pwm_value = piecewise_linear_fit(rpm);
#else
    // Calculate PWM register value based on rpm max/min settings and programmed rpm.
    if (rpm == 0.0) {
        pwm_value = _pwm_off_value;
    } else {
        pwm_value = (uint16_t)map_float(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
    }
#endif

#ifdef  SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    set_enable_pin(rpm != 0);
#endif

    set_output(pwm_value);

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
    set_output(_pwm_off_value);
}

// prints the startup message of the spindle config
void PWMSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "PWM spindle on Pin:%d, Freq:%.2fHz, Res:%dbits", _output_pin, _pwm_freq, _pwm_precision);
}


void PWMSpindle::set_output(uint32_t duty) {

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


/*
    Calculate the highest precision of a PWM based on the frequency in bits

    80,000,000 / freq = period
    determine the highest precision where (1 << precision) < period
*/
uint8_t PWMSpindle :: calc_pwm_precision(float freq) {
    uint8_t precision = 0;

    while ((1 << precision) < (uint32_t)(80000000.0 / freq) && precision <= 16)
        precision++;

    return precision - 1;
}

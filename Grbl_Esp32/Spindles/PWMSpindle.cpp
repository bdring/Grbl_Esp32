/*
    PWMSpindle.cpp

    This is a full featured TTL PWM spindle This does not include speed/power
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
#include "SpindleClass.h"

// ======================= PWMSpindle ==============================
/*
    This gets called at startup or whenever a spindle setting changes
    If the spindle is running it will stop and need to be restarted with M3Snnnn
*/

//#include "grbl.h"

void PWMSpindle :: init() {

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

#ifdef INVERT_SPINDLE_ENABLE_PIN
    _invert_pwm = true;
#else
    _invert_pwm = false;
#endif

#ifdef SPINDLE_ENABLE_PIN
    _enable_pin = SPINDLE_ENABLE_PIN;
#ifdef SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    _off_with_zero_speed = true;
#endif
#else
    _enable_pin = UNDEFINED_PIN;
    _off_with_zero_speed = false;
#endif


#ifdef SPINDLE_DIR_PIN
    _direction_pin = SPINDLE_DIR_PIN;
#else
    _direction_pin = UNDEFINED_PIN;
#endif

    is_reversable = (_direction_pin != UNDEFINED_PIN);

    _pwm_freq = (uint32_t)settings.spindle_pwm_freq;
    _pwm_precision = calc_pwm_precision(_pwm_freq); // detewrmine the best precision
    _pwm_period = (1 << _pwm_precision);

    if (settings.spindle_pwm_min_value > settings.spindle_pwm_min_value)
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Spindle min pwm is greater than max. Check $35 and $36");

    // pre-caculate some PWM count values
    _pwm_off_value = (_pwm_period * (uint32_t)settings.spindle_pwm_off_value / 100.0);
    _pwm_min_value = (_pwm_period * (uint32_t)settings.spindle_pwm_min_value / 100.0);
    _pwm_max_value = (_pwm_period * (uint32_t)settings.spindle_pwm_max_value / 100.0);

#ifdef ENABLE_PIECEWISE_LINEAR_SPINDLE
    _min_rpm = RPM_MIN;
    _max_rpm = RPM_MAX;
    _piecewide_linear = true;
#else
    _min_rpm = (uint32_t)settings.rpm_min;
    _max_rpm = (uint32_t)settings.rpm_max;
    _piecewide_linear = false;
#endif
    // The pwm_gradient is the pwm duty cycle units per rpm
    // _pwm_gradient = (_pwm_max_value - _pwm_min_value) / (_max_rpm - _min_rpm);

    _spindle_pwm_chan_num = 0; // Channel 0 is reserved for spindle use


}

uint32_t PWMSpindle::set_rpm(uint32_t rpm) {
    uint32_t pwm_value;

    if (_output_pin == UNDEFINED_PIN)
        return rpm;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set rpm %d", rpm);

    // apply override
    rpm = rpm * sys.spindle_speed_ovr / 100; // Scale by spindle speed override value (uint8_t percent)

    // apply limits
    if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm))
        rpm = _max_rpm;
    else if (rpm != 0 && rpm <= _min_rpm)
        rpm = _min_rpm;

    sys.spindle_speed = rpm;

    if (_piecewide_linear) {
        //pwm_value = piecewise_linear_fit(rpm); TODO
        pwm_value = 0;
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: Linear fit not implemented yet.");

    } else {
        if (rpm == 0)
            pwm_value = _pwm_off_value;
        else
            pwm_value = map_uint32_t(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
    }

    if (_off_with_zero_speed)
        set_enable_pin(rpm != 0);

    set_output(pwm_value);

    return 0;
}

void PWMSpindle::set_state(uint8_t state, uint32_t rpm) {
    if (sys.abort)
        return;   // Block during abort.

    if (state == SPINDLE_DISABLE) { // Halt or set spindle direction and rpm.
        sys.spindle_speed = 0;
        stop();
    } else {
        set_spindle_dir_pin(state == SPINDLE_ENABLE_CW);
        set_rpm(rpm);
    }

    set_enable_pin(state == SPINDLE_DISABLE);

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
    grbl_msg_sendf(CLIENT_SERIAL,
                   MSG_LEVEL_INFO,
                   "PWM spindle Output:%d, Enbl:%d, Dir:%d, Freq:%dHz, Res:%dbits",
                   report_pin_number(_output_pin),
                   report_pin_number(_enable_pin), // 255 means pin not defined
                   report_pin_number(_direction_pin), // 255 means pin not defined
                   _pwm_freq,
                   _pwm_precision);
}


void PWMSpindle::set_output(uint32_t duty) {
    if (_output_pin == UNDEFINED_PIN)
        return;

    //grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Set output %d", duty);

    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;

    if (_invert_pwm)
        duty = (1 << _pwm_precision) - duty;

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
uint8_t PWMSpindle :: calc_pwm_precision(uint32_t freq) {
    uint8_t precision = 0;

    // increase the precision (bits) until it exceeds allow by frequency the max or is 16
    while ((1 << precision) < (uint32_t)(80000000 / freq) && precision <= 16)
        precision++;

    return precision - 1;
}

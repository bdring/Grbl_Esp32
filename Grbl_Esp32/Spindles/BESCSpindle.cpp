/*
    BESCSpindle.cpp

    This a special type of PWM spindle for RC type Brushless DC Speed
    controllers.

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



    Important ESC Settings
    $33=50 // Hz this is the typical good frequency for an ESC
    #define DEFAULT_SPINDLE_FREQ 5000.0 // $33 Hz (extended set)

    Determine the typical min and max pulse length of your ESC
    min_pulse is typically 1ms (0.001 sec) or less
    max_pulse is typically 2ms (0.002 sec) or more

    determine PWM_period. It is (1/freq) if freq = 50...period = 0.02

    determine pulse length for min_pulse and max_pulse in percent.

    (pulse / PWM_period)
    min_pulse = (0.001 / 0.02) = 0.05 = 5%
    max_pulse = (0.002 / .02) = 0.1 = 10%

*/

// don't change these
#define BESC_PWM_FREQ           50.0f // Hz
#define BESC_PWM_BIT_PRECISION  16   // bits
#define BESC_PULSE_PERIOD       (1.0 / BESC_PWM_FREQ)
// ok to tweak
#define BESC_MIN_PULSE_SECS     0.001f
#define BESC_MAX_PULSE_SECS     0.002f
//don't change
#define BESC_MIN_PULSE_CNT      (uint16_t)(BESC_MIN_PULSE_SECS / BESC_PULSE_PERIOD * 65535.0)
#define BESC_MAX_PULSE_CNT      (uint16_t)(BESC_MAX_PULSE_SECS / BESC_PULSE_PERIOD * 65535.0)

void BESCSpindle :: init() {

    get_pins_and_settings();

    if (_output_pin == UNDEFINED_PIN) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Warning: BESC output pin not defined");
        return; // We cannot continue without the output pin
    }

    // override some settings to what is required for a BESC
    _pwm_freq = BESC_PWM_FREQ;
    _pwm_precision = 16;

    // override these settings
    // to do make these tweakable
    _pwm_off_value = BESC_MIN_PULSE_CNT;
    _pwm_min_value = BESC_MIN_PULSE_CNT;
    _pwm_max_value = BESC_MAX_PULSE_CNT;

    ledcSetup(_spindle_pwm_chan_num, (double)_pwm_freq, _pwm_precision); // setup the channel
    ledcAttachPin(_output_pin, _spindle_pwm_chan_num); // attach the PWM to the pin

    if (_enable_pin != UNDEFINED_PIN)
        pinMode(_enable_pin, OUTPUT);

    set_rpm(0);

    config_message();
}

// prints the startup message of the spindle config
void BESCSpindle :: config_message() {
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "BESC spindle on Pin:%d", _output_pin);
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
    } else {
        pwm_value = (uint16_t)map_float(rpm, _min_rpm, _max_rpm, _pwm_min_value, _pwm_max_value);
    }

#ifdef  SPINDLE_ENABLE_OFF_WITH_ZERO_SPEED
    set_enable_pin(rpm != 0);
#endif

    set_output(pwm_value);
    return rpm;
}

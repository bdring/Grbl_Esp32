/*
    StandardStepperClass.cpp

    This is used for a stepper motor that just requires step and direction
    pins.

    TODO: Add an enable pin 

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

// this is the pulse range of a the servo. Typical servos are 0.001 to 0.002 seconds
// some servos have a wider range. You can adjust this here or in the calibration feature
#define SERVO_MIN_PULSE_SEC 0.001 // min pulse in seconds
#define SERVO_MAX_PULSE_SEC 0.002 // max pulse in seconds

#define SERVO_POSITION_MIN_DEFAULT   0.0 // mm
#define SERVO_POSITION_MAX_DEFAULT   20.0 // mm

#define SERVO_PULSE_FREQ 50 // 50Hz ...This is a standard analog servo value. Digital ones can repeat faster

#define SERVO_PULSE_RES_BITS 16 // bits of resolution of PWM (16 is max)
#define SERVO_PULSE_RES_COUNT 65535 // see above  TODO...do the math here 2^SERVO_PULSE_RES_BITS

#define SERVO_TIME_PER_BIT  ((1.0 / (float)SERVO_PULSE_FREQ) / ((float)SERVO_PULSE_RES_COUNT) ) // seconds

#define SERVO_MIN_PULSE    (uint16_t)(SERVO_MIN_PULSE_SEC / SERVO_TIME_PER_BIT) // in timer counts
#define SERVO_MAX_PULSE    (uint16_t)(SERVO_MAX_PULSE_SEC / SERVO_TIME_PER_BIT) // in timer counts

#define SERVO_PULSE_RANGE (SERVO_MAX_PULSE-SERVO_MIN_PULSE)

#define SERVO_CAL_MIN 20.0 // Percent: the minimum allowable calibration value
#define SERVO_CAL_MAX 180.0 // Percent: the maximum allowable calibration value

#define SERVO_TIMER_INT_FREQ 20.0 // Hz This is the task frequency

#define SERVO_HOMING_OFF 0 // servo is off during homing
#define SERVO_HOMING_TARGET 1 // servo is send to a location during homing

RcServo :: RcServo() {

}

RcServo :: RcServo(uint8_t axis_index, uint8_t pwm_pin) {
    this->axis_index = axis_index % MAX_AXES;
    this->dual_axis_index = axis_index < MAX_AXES ? 0 : 1; // 0 = primary 1 = ganged
    this->_pwm_pin = pwm_pin;
    set_axis_name();
    init();
    config_message();
    is_active = true;  // as opposed to NullMotors, this is a real motor
}

void RcServo :: init() {
    _channel_num = sys_get_next_PWM_chan_num();
     ledcSetup(_channel_num, SERVO_PULSE_FREQ, SERVO_PULSE_RES_BITS);
    ledcAttachPin(_pwm_pin, _channel_num);
    _current_pwm_duty = 0;

}

void RcServo :: config_message() {    
                                    grbl_msg_sendf(CLIENT_SERIAL,
                                    MSG_LEVEL_INFO,
                                    "%s Axis RC Servo motor PWM:%d",
                                    _axis_name,
                                    _pwm_pin);
}

void RcServo::_write_pwm(uint32_t duty) {
    // to prevent excessive calls to ledcWrite, make sure duty hass changed
    if (duty == _current_pwm_duty)
        return;

    _current_pwm_duty = duty;
    
    ledcWrite(_channel_num, duty);
}

// sets the PWM to zero. This allows most servos to be manually moved
void RcServo::set_disable(bool disable) {
    if (disable)
        _write_pwm(0);
}


/*
  servo_axis.cpp
  Part of Grbl_ESP32

	copyright (c) 2018 -	Bart Dring. This file was intended for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

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

	See servo_axis.h for more details

*/

#include "grbl.h"

#include "servo_axis.h"

#ifdef USE_SERVO_AXES

static TaskHandle_t servosSyncTaskHandle = 0;

#    ifdef SERVO_X_PIN
ServoAxis X_Servo_Axis(X_AXIS, SERVO_X_PIN);
#    endif
#    ifdef SERVO_Y_PIN
ServoAxis Y_Servo_Axis(Y_AXIS, SERVO_Y_PIN);
#    endif
#    ifdef SERVO_Z_PIN
ServoAxis Z_Servo_Axis(Z_AXIS, SERVO_Z_PIN);
#    endif

#    ifdef SERVO_A_PIN
ServoAxis A_Servo_Axis(A_AXIS, SERVO_A_PIN);
#    endif
#    ifdef SERVO_B_PIN
ServoAxis B_Servo_Axis(B_AXIS, SERVO_B_PIN);
#    endif
#    ifdef SERVO_C_PIN
ServoAxis C_Servo_Axis(C_AXIS, SERVO_C_PIN);
#    endif

void init_servos() {
    // ======================== X Axis ===========================
#    ifdef SERVO_X_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "X Servo range %4.3f to %4.3f", SERVO_X_RANGE_MIN, SERVO_X_RANGE_MAX);
    X_Servo_Axis.init();
    X_Servo_Axis.set_range(SERVO_X_RANGE_MIN, SERVO_X_RANGE_MAX);
#        ifdef SERVO_X_HOMING_TYPE
    X_Servo_Axis.set_homing_type(SERVO_X_HOMING_TYPE);
#        endif
#        ifdef SERVO_X_HOME_POS
    X_Servo_Axis.set_homing_position(SERVO_X_HOME_POS);
#        endif
#        ifdef SERVO_X_MPOS  // value should be true or false
    X_Servo_Axis.set_use_mpos(SERVO_X_MPOS);
#        endif
#        ifdef SERVO_X_DISABLE_ON_ALARM
    set_disable_on_alarm(SERVO_X_DISABLE_ON_ALARM);
#        endif
#        ifdef SERVO_X_DISABLE_WITH_STEPPERS
    set_disable_with_steppers(SERVO_X_DISABLE_WITH_STEPPERS);
#        endif
#    endif
    // ======================== Y Axis ===========================
#    ifdef SERVO_Y_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Y Servo range %4.3f to %4.3f", SERVO_Y_RANGE_MIN, SERVO_Y_RANGE_MAX);
    Y_Servo_Axis.init();
    Y_Servo_Axis.set_range(SERVO_Y_RANGE_MIN, SERVO_Y_RANGE_MAX);
#        ifdef SERVO_Y_HOMING_TYPE
    Y_Servo_Axis.set_homing_type(SERVO_Y_HOMING_TYPE);
#        endif
#        ifdef SERVO_Y_HOME_POS
    Y_Servo_Axis.set_homing_position(SERVO_Y_HOME_POS);
#        endif
#        ifdef SERVO_Y_MPOS  // value should be true or false
    Y_Servo_Axis.set_use_mpos(SERVO_Y_MPOS);
#        endif
#        ifdef SERVO_Y_DISABLE_ON_ALARM
    set_disable_on_alarm(SERVO_Y_DISABLE_ON_ALARM);
#        endif
#        ifdef SERVO_Y_DISABLE_WITH_STEPPERS
    set_disable_with_steppers(SERVO_Y_DISABLE_WITH_STEPPERS);
#        endif
#    endif
    // ======================== Z Axis ===========================
#    ifdef SERVO_Z_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Z Servo range %4.3f to %4.3f", SERVO_Z_RANGE_MIN, SERVO_Z_RANGE_MAX);
    Z_Servo_Axis.init();
    Z_Servo_Axis.set_range(SERVO_Z_RANGE_MIN, SERVO_Z_RANGE_MAX);
#        ifdef SERVO_Z_HOMING_TYPE
    Z_Servo_Axis.set_homing_type(SERVO_Z_HOMING_TYPE);
#        endif
#        ifdef SERVO_Z_HOME_POS
    Z_Servo_Axis.set_homing_position(SERVO_Z_HOME_POS);
#        endif
#        ifdef SERVO_Z_MPOS  // value should be true or false
    Z_Servo_Axis.set_use_mpos(SERVO_Z_MPOS);
#        endif
#        ifdef SERVO_Z_DISABLE_ON_ALARM
    set_disable_on_alarm(SERVO_Z_DISABLE_ON_ALARM);
#        endif
#        ifdef SERVO_Z_DISABLE_WITH_STEPPERS
    set_disable_with_steppers(SERVO_Z_DISABLE_WITH_STEPPERS);
#        endif
#    endif
    // ======================== A Axis ===========================
#    ifdef SERVO_A_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "A Servo range %4.3f to %4.3f", SERVO_A_RANGE_MIN, SERVO_A_RANGE_MAX);
    A_Servo_Axis.init();
    A_Servo_Axis.set_range(SERVO_A_RANGE_MIN, SERVO_A_RANGE_MAX);
#        ifdef SERVO_A_HOMING_TYPE
    A_Servo_Axis.set_homing_type(SERVO_A_HOMING_TYPE);
#        endif
#        ifdef SERVO_A_HOME_POS
    A_Servo_Axis.set_homing_position(SERVO_A_HOME_POS);
#        endif
#        ifdef SERVO_A_MPOS  // value should be true or false
    A_Servo_Axis.set_use_mpos(SERVO_A_MPOS);
#        endif
#        ifdef SERVO_A_DISABLE_ON_ALARM
    set_disable_on_alarm(SERVO_A_DISABLE_ON_ALARM);
#        endif
#        ifdef SERVO_A_DISABLE_WITH_STEPPERS
    set_disable_with_steppers(SERVO_A_DISABLE_WITH_STEPPERS);
#        endif
#    endif
    // ======================== B Axis ===========================
#    ifdef SERVO_B_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "B Servo range %4.3f to %4.3f", SERVO_B_RANGE_MIN, SERVO_B_RANGE_MAX);
    B_Servo_Axis.init();
    B_Servo_Axis.set_range(SERVO_B_RANGE_MIN, SERVO_B_RANGE_MAX);
#        ifdef SERVO_B_HOMING_TYPE
    B_Servo_Axis.set_homing_type(SERVO_B_HOMING_TYPE);
#        endif
#        ifdef SERVO_B_HOME_POS
    B_Servo_Axis.set_homing_position(SERVO_B_HOME_POS);
#        endif
#        ifdef SERVO_B_MPOS  // value should be true or false
    B_Servo_Axis.set_use_mpos(SERVO_B_MPOS);
#        endif
#        ifdef SERVO_B_DISABLE_ON_ALARM
    set_disable_on_alarm(SERVO_B_DISABLE_ON_ALARM);
#        endif
#        ifdef SERVO_B_DISABLE_WITH_STEPPERS
    set_disable_with_steppers(SERVO_B_DISABLE_WITH_STEPPERS);
#        endif
#    endif
    // ======================== C Axis ===========================
#    ifdef SERVO_C_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "C Servo range %4.3f to %4.3f", SERVO_C_RANGE_MIN, SERVO_C_RANGE_MAX);
    C_Servo_Axis.init();
    C_Servo_Axis.set_range(SERVO_C_RANGE_MIN, SERVO_C_RANGE_MAX);
#        ifdef SERVO_C_HOMING_TYPE
    C_Servo_Axis.set_homing_type(SERVO_C_HOMING_TYPE);
#        endif
#        ifdef SERVO_C_HOME_POS
    C_Servo_Axis.set_homing_position(SERVO_C_HOME_POS);
#        endif
#        ifdef SERVO_C_MPOS  // value should be true or false
    C_Servo_Axis.set_use_mpos(SERVO_C_MPOS);
#        endif
#        ifdef SERVO_C_DISABLE_ON_ALARM
    set_disable_on_alarm(SERVO_C_DISABLE_ON_ALARM);
#        endif
#        ifdef SERVO_C_DISABLE_WITH_STEPPERS
    set_disable_with_steppers(SERVO_C_DISABLE_WITH_STEPPERS);
#        endif
#    endif
    // setup a task that will calculate the determine and set the servo positions
    xTaskCreatePinnedToCore(servosSyncTask,    // task
                            "servosSyncTask",  // name for task
                            4096,              // size of task stack
                            NULL,              // parameters
                            1,                 // priority
                            &servosSyncTaskHandle,
                            0  // core
    );
}

// this is the task
void servosSyncTask(void* pvParameters) {
    TickType_t       xLastWakeTime;
    const TickType_t xServoFrequency = SERVO_TIMER_INT_FREQ;  // in ticks (typically ms)
    xLastWakeTime                    = xTaskGetTickCount();   // Initialise the xLastWakeTime variable with the current time.
    while (true) {                                            // don't ever return from this or the task dies
#    ifdef SERVO_X_PIN
        X_Servo_Axis.set_location();
#    endif
#    ifdef SERVO_Y_PIN
        Y_Servo_Axis.set_location();
#    endif
#    ifdef SERVO_Z_PIN
        Z_Servo_Axis.set_location();
#    endif
#    ifdef SERVO_A_PIN
        A_Servo_Axis.set_location();
#    endif
#    ifdef SERVO_B_PIN
        B_Servo_Axis.set_location();
#    endif
#    ifdef SERVO_C_PIN
        C_Servo_Axis.set_location();
#    endif
        vTaskDelayUntil(&xLastWakeTime, xServoFrequency);
    }
}

// =============================== Class Stuff ================================= //

ServoAxis::ServoAxis(uint8_t axis, uint8_t pin_num) {  // constructor
    _axis        = axis;
    _pin_num     = pin_num;
    _channel_num = sys_get_next_PWM_chan_num();
    _showError   = true;  // this will be used to show calibration error only once
    _use_mpos    = true;  // default is to use the machine position rather than work position
}

void ServoAxis::init() {
    _cal_is_valid();
    ledcSetup(_channel_num, _pwm_freq, _pwm_resolution_bits);
    ledcAttachPin(_pin_num, _channel_num);
    disable();
}

void ServoAxis::set_location() {
    // These are the pulse lengths for the minimum and maximum positions
    // Note: Some machines will have the physical max/min inverted with pulse length max/min due to invert setting $3=...
    float    servo_pulse_min, servo_pulse_max;
    float    min_pulse_cal, max_pulse_cal;  // calibration values in percent 110% = 1.1
    uint32_t servo_pulse_len;
    float    servo_pos, mpos, offset;
    // skip location if we are in alarm mode
    if (_disable_on_alarm && (sys.state == STATE_ALARM)) {
        disable();
        return;
    }
    // track the disable status of the steppers if desired.
    if (_disable_with_steppers && get_stepper_disable()) {
        disable();
        return;
    }
    if ((_homing_type == SERVO_HOMING_TARGET) && (sys.state == STATE_HOMING)) {
        servo_pos = _homing_position;  // go to servos home position
    } else {
        mpos = system_convert_axis_steps_to_mpos(sys_position, _axis);  // get the axis machine position in mm
        if (_use_mpos)
            servo_pos = mpos;
        else {
            offset    = gc_state.coord_system[_axis] + gc_state.coord_offset[_axis];  // get the current axis work offset
            servo_pos = mpos - offset;                                                // determine the current work position
        }
    }
    // 1. Get the pulse ranges of the servos
    // 2. Invert if selected in the settings
    // 3. Get the calibration values from the settings
    // 4. Adjust the calibration offset direction of the cal based on the direction
    // 5. Apply the calibrarion
    servo_pulse_min = SERVO_MIN_PULSE;
    servo_pulse_max = SERVO_MAX_PULSE;
    if (bit_istrue(dir_invert_mask->get(), bit(_axis)))  // this allows the user to change the direction via settings
        swap(servo_pulse_min, servo_pulse_max);
    // get the calibration values
    if (_cal_is_valid()) {  // if calibration settings are OK then apply them
        // apply a calibration
        // the cals apply differently if the direction is reverse (i.e. longer pulse is lower position)
        if (bit_isfalse(dir_invert_mask->get(), bit(_axis))) {  // normal direction
            min_pulse_cal = 2.0 - (axis_settings[_axis]->steps_per_mm->get() / 100.0);
            max_pulse_cal = (axis_settings[_axis]->max_travel->get() / 100.0);
        } else {  // inverted direction
            min_pulse_cal = (axis_settings[_axis]->steps_per_mm->get() / 100.0);
            max_pulse_cal = 2.0 - (axis_settings[_axis]->max_travel->get() / -100.0);
        }
    } else {  // settings are not valid so don't apply any calibration
        min_pulse_cal = 1.0;
        max_pulse_cal = 1.0;
    }
    // apply the calibrations
    servo_pulse_min *= min_pulse_cal;
    servo_pulse_max *= max_pulse_cal;
    // determine the pulse length
    servo_pulse_len = (uint32_t)mapConstrain(servo_pos, _position_min, _position_max, servo_pulse_min, servo_pulse_max);
    _write_pwm(servo_pulse_len);
}

void ServoAxis::_write_pwm(uint32_t duty) {
    if (ledcRead(_channel_num) != duty)  // only write if it is changing
        ledcWrite(_channel_num, duty);
}

// sets the PWM to zero. This allows most servos to be manually moved
void ServoAxis::disable() {
    _write_pwm(0);
}

// checks to see if calibration values are in an acceptable range
// vebose = true if you want an error sent to serial port
bool ServoAxis::_cal_is_valid() {
    bool settingsOK = true;
    if ((axis_settings[_axis]->steps_per_mm->get() < SERVO_CAL_MIN) || (axis_settings[_axis]->steps_per_mm->get() > SERVO_CAL_MAX)) {
        if (_showError) {
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo calibration ($10%d) value error. Reset to 100", _axis);
            char reset_val[] = "100";
            axis_settings[_axis]->steps_per_mm->setStringValue(reset_val);
        }
        settingsOK = false;
    }
    // Note: Max travel is set positive via $$, but stored as a negative number
    auto travel = -axis_settings[_axis]->max_travel->get();
    if ((travel < -SERVO_CAL_MAX) || travel > -SERVO_CAL_MIN) {
        if (_showError) {
            grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Servo calibration ($13%d) value error. Reset to 100", _axis);
            char reset_val[] = "-100";  // stored as a negative
            axis_settings[_axis]->max_travel->setStringValue(reset_val);
        }
        settingsOK = false;
    }
    _showError = false;  // to show error once
    return settingsOK;
}

/*
		Use this to set the max and min position in mm of the servo
		This is used when mapping pulse length to the position
*/
void ServoAxis::set_range(float min, float max) {
    if (min < max) {
        _position_min = min;
        _position_max = max;
    } else
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Error setting range. Min not smaller than max");
}

/*
		Sets the mode the servo will be in during homing
		See servo_axis.h for SERVO_HOMING_xxxxx types
*/
void ServoAxis::set_homing_type(uint8_t homing_type) {
    if (homing_type <= SERVO_HOMING_TARGET)
        _homing_type = homing_type;
}

/*
		Use this to set the homing position the servo will be commanded to go if
		the current homing mode is SERVO_HOMING_TARGET
*/
void ServoAxis::set_homing_position(float homing_position) {
    _homing_position = homing_position;
}

/*
		Use this to set the disable on alarm feature. If true, then hobby servo PWM
		will be disable in Grbl alarm mode (like before homing). Typical hobby servo
		can be moved by hand in this mode
*/
void ServoAxis::set_disable_on_alarm(bool disable_on_alarm) {
    _disable_on_alarm = disable_on_alarm;
}

void ServoAxis::set_disable_with_steppers(bool disable_with_steppers) {
    _disable_with_steppers = disable_with_steppers;
}

/*
	If true, servo position will alway be calculated in machine position
	Offsets will not be applied
*/
void ServoAxis::set_use_mpos(bool use_mpos) {
    _use_mpos = use_mpos;
}

#endif

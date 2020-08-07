/*
  system.cpp - Header for system level commands and real-time processes
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

	2018 -	Bart Dring This file was modified for use on the ESP32
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
*/

#include "grbl.h"
#include "config.h"

xQueueHandle control_sw_queue;    // used by control switch debouncing
bool         debouncing = false;  // debouncing in process

void system_ini() {  // Renamed from system_init() due to conflict with esp32 files
    // setup control inputs

#ifdef CONTROL_SAFETY_DOOR_PIN
    pinMode(CONTROL_SAFETY_DOOR_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_SAFETY_DOOR_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef CONTROL_RESET_PIN
    pinMode(CONTROL_RESET_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_RESET_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef CONTROL_FEED_HOLD_PIN
    pinMode(CONTROL_FEED_HOLD_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_FEED_HOLD_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef CONTROL_CYCLE_START_PIN
    pinMode(CONTROL_CYCLE_START_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(CONTROL_CYCLE_START_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_0_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Macro Pin 0");
    pinMode(MACRO_BUTTON_0_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_0_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_1_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Macro Pin 1");
    pinMode(MACRO_BUTTON_1_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_1_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_2_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Macro Pin 2");
    pinMode(MACRO_BUTTON_2_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_2_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef MACRO_BUTTON_3_PIN
    grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Macro Pin 3");
    pinMode(MACRO_BUTTON_3_PIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MACRO_BUTTON_3_PIN), isr_control_inputs, CHANGE);
#endif
#ifdef ENABLE_CONTROL_SW_DEBOUNCE
    // setup task used for debouncing
    control_sw_queue = xQueueCreate(10, sizeof(int));
    xTaskCreate(controlCheckTask,
                "controlCheckTask",
                2048,
                NULL,
                5,  // priority
                NULL);
#endif

    //customize pin definition if needed
#if (GRBL_SPI_SS != -1) || (GRBL_SPI_MISO != -1) || (GRBL_SPI_MOSI != -1) || (GRBL_SPI_SCK != -1)
    SPI.begin(GRBL_SPI_SCK, GRBL_SPI_MISO, GRBL_SPI_MOSI, GRBL_SPI_SS);
#endif
    // Setup USER_DIGITAL_PINs controlled by M62 and M63
#ifdef USER_DIGITAL_PIN_1
    pinMode(USER_DIGITAL_PIN_1, OUTPUT);
    sys_io_control(bit(1), false);  // turn off
#endif
#ifdef USER_DIGITAL_PIN_2
    pinMode(USER_DIGITAL_PIN_2, OUTPUT);
    sys_io_control(bit(2), false);  // turn off
#endif
#ifdef USER_DIGITAL_PIN_3
    pinMode(USER_DIGITAL_PIN_3, OUTPUT);
    sys_io_control(bit(3), false);  // turn off
#endif
#ifdef USER_DIGITAL_PIN_4
    pinMode(USER_DIGITAL_PIN_4, OUTPUT);
    sys_io_control(bit(4), false);  // turn off
#endif
}

#ifdef ENABLE_CONTROL_SW_DEBOUNCE
// this is the debounce task
void controlCheckTask(void* pvParameters) {
    while (true) {
        int evt;
        xQueueReceive(control_sw_queue, &evt, portMAX_DELAY);  // block until receive queue
        vTaskDelay(CONTROL_SW_DEBOUNCE_PERIOD);                // delay a while
        uint8_t pin = system_control_get_state();
        if (pin)
            system_exec_control_pin(pin);
        debouncing = false;
    }
}
#endif

void IRAM_ATTR isr_control_inputs() {
#ifdef ENABLE_CONTROL_SW_DEBOUNCE
    // we will start a task that will recheck the switches after a small delay
    int evt;
    if (!debouncing) {  // prevent resending until debounce is done
        debouncing = true;
        xQueueSendFromISR(control_sw_queue, &evt, NULL);
    }
#else
    uint8_t pin = system_control_get_state();
    system_exec_control_pin(pin);
#endif
}

// Returns if safety door is ajar(T) or closed(F), based on pin state.
uint8_t system_check_safety_door_ajar() {
#ifdef ENABLE_SAFETY_DOOR_INPUT_PIN
    return (system_control_get_state() & CONTROL_PIN_INDEX_SAFETY_DOOR);
#else
    return (false);  // Input pin not enabled, so just return that it's closed.
#endif
}

// Special handlers for setting and clearing Grbl's real-time execution flags.
void system_set_exec_state_flag(uint8_t mask) {
    // TODO uint8_t sreg = SREG;
    // TODO cli();
    sys_rt_exec_state |= (mask);
    // TODO SREG = sreg;
}

void system_clear_exec_state_flag(uint8_t mask) {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_state &= ~(mask);
    //SREG = sreg;
}

void system_set_exec_alarm(uint8_t code) {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_alarm = code;
    //SREG = sreg;
}

void system_clear_exec_alarm() {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_alarm = 0;
    //SREG = sreg;
}

void system_set_exec_motion_override_flag(uint8_t mask) {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_motion_override |= (mask);
    //SREG = sreg;
}

void system_set_exec_accessory_override_flag(uint8_t mask) {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_accessory_override |= (mask);
    //SREG = sreg;
}

void system_clear_exec_motion_overrides() {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_motion_override = 0;
    //SREG = sreg;
}

void system_clear_exec_accessory_overrides() {
    //uint8_t sreg = SREG;
    //cli();
    sys_rt_exec_accessory_override = 0;
    //SREG = sreg;
}

void system_flag_wco_change() {
#ifdef FORCE_BUFFER_SYNC_DURING_WCO_CHANGE
    protocol_buffer_synchronize();
#endif
    sys.report_wco_counter = 0;
}

// Returns machine position of axis 'idx'. Must be sent a 'step' array.
// NOTE: If motor steps and machine position are not in the same coordinate frame, this function
//   serves as a central place to compute the transformation.
float system_convert_axis_steps_to_mpos(int32_t* steps, uint8_t idx) {
    float pos;
    float steps_per_mm = axis_settings[idx]->steps_per_mm->get();
#ifdef COREXY
    if (idx == X_AXIS)
        pos = (float)system_convert_corexy_to_x_axis_steps(steps) / steps_per_mm;
    else if (idx == Y_AXIS)
        pos = (float)system_convert_corexy_to_y_axis_steps(steps) / steps_per_mm;
    else
        pos = steps[idx] / steps_per_mm;
#else
    pos = steps[idx] / steps_per_mm;
#endif
    return (pos);
}

void system_convert_array_steps_to_mpos(float* position, int32_t* steps) {
    uint8_t idx;
    for (idx = 0; idx < N_AXIS; idx++)
        position[idx] = system_convert_axis_steps_to_mpos(steps, idx);
    return;
}

// Checks and reports if target array exceeds machine travel limits.
uint8_t system_check_travel_limits(float* target) {
    uint8_t idx;
    for (idx = 0; idx < N_AXIS; idx++) {
        float travel = axis_settings[idx]->max_travel->get();
#ifdef HOMING_FORCE_SET_ORIGIN
        uint8_t mask = homing_dir_mask->get();
        // When homing forced set origin is enabled, soft limits checks need to account for directionality.
        if (bit_istrue(mask, bit(idx))) {
            if (target[idx] < 0 || target[idx] > travel)
                return (true);
        } else {
            if (target[idx] > 0 || target[idx] < -travel)
                return (true);
        }
#else
#    ifdef HOMING_FORCE_POSITIVE_SPACE
        if (target[idx] < 0 || target[idx] > travel)
            return (true);
#    else
        if (target[idx] > 0 || target[idx] < -travel)
            return (true);
#    endif
#endif
    }
    return (false);
}

// Returns control pin state as a uint8 bitfield. Each bit indicates the input pin state, where
// triggered is 1 and not triggered is 0. Invert mask is applied. Bitfield organization is
// defined by the CONTROL_PIN_INDEX in the header file.
uint8_t system_control_get_state() {
    uint8_t defined_pin_mask = 0;  // a mask of defined pins
    uint8_t control_state    = 0;

#ifdef CONTROL_SAFETY_DOOR_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_SAFETY_DOOR;
    if (digitalRead(CONTROL_SAFETY_DOOR_PIN))
        control_state |= CONTROL_PIN_INDEX_SAFETY_DOOR;
#endif
#ifdef CONTROL_RESET_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_RESET;
    if (digitalRead(CONTROL_RESET_PIN))
        control_state |= CONTROL_PIN_INDEX_RESET;
#endif
#ifdef CONTROL_FEED_HOLD_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_FEED_HOLD;
    if (digitalRead(CONTROL_FEED_HOLD_PIN))
        control_state |= CONTROL_PIN_INDEX_FEED_HOLD;
#endif
#ifdef CONTROL_CYCLE_START_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_CYCLE_START;
    if (digitalRead(CONTROL_CYCLE_START_PIN))
        control_state |= CONTROL_PIN_INDEX_CYCLE_START;
#endif
#ifdef MACRO_BUTTON_0_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_MACRO_0;
    if (digitalRead(MACRO_BUTTON_0_PIN))
        control_state |= CONTROL_PIN_INDEX_MACRO_0;
#endif
#ifdef MACRO_BUTTON_1_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_MACRO_1;
    if (digitalRead(MACRO_BUTTON_1_PIN))
        control_state |= CONTROL_PIN_INDEX_MACRO_1;
#endif
#ifdef MACRO_BUTTON_2_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_MACRO_2;
    if (digitalRead(MACRO_BUTTON_2_PIN))
        control_state |= CONTROL_PIN_INDEX_MACRO_2;
#endif
#ifdef MACRO_BUTTON_3_PIN
    defined_pin_mask |= CONTROL_PIN_INDEX_MACRO_3;
    if (digitalRead(MACRO_BUTTON_3_PIN))
        control_state |= CONTROL_PIN_INDEX_MACRO_3;
#endif
#ifdef INVERT_CONTROL_PIN_MASK
    control_state ^= (INVERT_CONTROL_PIN_MASK & defined_pin_mask);
#endif
    return (control_state);
}

// execute the function of the control pin
void system_exec_control_pin(uint8_t pin) {
    if (bit_istrue(pin, CONTROL_PIN_INDEX_RESET)) {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_INFO, "Reset via control pin");
        mc_reset();
    } else if (bit_istrue(pin, CONTROL_PIN_INDEX_CYCLE_START))
        bit_true(sys_rt_exec_state, EXEC_CYCLE_START);
    else if (bit_istrue(pin, CONTROL_PIN_INDEX_FEED_HOLD))
        bit_true(sys_rt_exec_state, EXEC_FEED_HOLD);
    else if (bit_istrue(pin, CONTROL_PIN_INDEX_SAFETY_DOOR))
        bit_true(sys_rt_exec_state, EXEC_SAFETY_DOOR);
#ifdef MACRO_BUTTON_0_PIN
    else if (bit_istrue(pin, CONTROL_PIN_INDEX_MACRO_0)) {
        user_defined_macro(CONTROL_PIN_INDEX_MACRO_0);  // function must be implemented by user
    }
#endif
#ifdef MACRO_BUTTON_1_PIN
    else if (bit_istrue(pin, CONTROL_PIN_INDEX_MACRO_1)) {
        user_defined_macro(CONTROL_PIN_INDEX_MACRO_1);  // function must be implemented by user
    }
#endif
#ifdef MACRO_BUTTON_2_PIN
    else if (bit_istrue(pin, CONTROL_PIN_INDEX_MACRO_2)) {
        user_defined_macro(CONTROL_PIN_INDEX_MACRO_2);  // function must be implemented by user
    }
#endif
#ifdef MACRO_BUTTON_3_PIN
    else if (bit_istrue(pin, CONTROL_PIN_INDEX_MACRO_3)) {
        user_defined_macro(CONTROL_PIN_INDEX_MACRO_3);  // function must be implemented by user
    }
#endif
}

// CoreXY calculation only. Returns x or y-axis "steps" based on CoreXY motor steps.
int32_t system_convert_corexy_to_x_axis_steps(int32_t* steps) {
    return ((steps[A_MOTOR] + steps[B_MOTOR]) / 2);
}
int32_t system_convert_corexy_to_y_axis_steps(int32_t* steps) {
    return ((steps[A_MOTOR] - steps[B_MOTOR]) / 2);
}

// io_num is the virtual pin# and has nothing to do with the actual esp32 GPIO_NUM_xx
// It uses a mask so all can be turned of in ms_reset
// This version waits until realtime commands have been executed
void sys_io_control(uint8_t io_num_mask, bool turnOn) {
    protocol_buffer_synchronize();
    fast_sys_io_control(io_num_mask, turnOn);
}

// This version works immediately, without waiting, to prevent deadlocks.
// It is used when resetting via mc_reset()
void fast_sys_io_control(uint8_t io_num_mask, bool turnOn) {
#ifdef USER_DIGITAL_PIN_1
    if (io_num_mask & bit(1)) {
        digitalWrite(USER_DIGITAL_PIN_1, turnOn);
        return;
    }
#endif
#ifdef USER_DIGITAL_PIN_2
    if (io_num_mask & bit(2)) {
        digitalWrite(USER_DIGITAL_PIN_2, turnOn);
        return;
    }
#endif
#ifdef USER_DIGITAL_PIN_3
    if (io_num_mask & bit(3)) {
        digitalWrite(USER_DIGITAL_PIN_3, turnOn);
        return;
    }
#endif
#ifdef USER_DIGITAL_PIN_4
    if (io_num_mask & bit(4)) {
        digitalWrite(USER_DIGITAL_PIN_4, turnOn);
        return;
    }
#endif
}

// Call this function to get an RMT channel number
// returns -1 for error
int8_t sys_get_next_RMT_chan_num() {
    static uint8_t next_RMT_chan_num = 0;  // channels 0-7 are valid
    if (next_RMT_chan_num < 8)             // 7 is the max PWM channel number
        return next_RMT_chan_num++;
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_ERROR, "Error: out of RMT channels");
        return -1;
    }
}

/*
    This returns an unused pwm channel.
    The 8 channels share 4 timers, so pairs 0,1 & 2,3 , etc
    have to be the same frequency. The spindle always uses channel 0
    so we start counting from 2.

    There are still possible issues if requested channels use different frequencies
    TODO: Make this more robust.
*/
int8_t sys_get_next_PWM_chan_num() {
    static uint8_t next_PWM_chan_num = 2;  // start at 2 to avoid spindle
    if (next_PWM_chan_num < 8)             // 7 is the max PWM channel number
        return next_PWM_chan_num++;
    else {
        grbl_msg_sendf(CLIENT_SERIAL, MSG_LEVEL_ERROR, "Error: out of PWM channels");
        return -1;
    }
}

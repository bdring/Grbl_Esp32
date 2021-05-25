/*
  System.cpp - Header for system level commands and real-time processes
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

#include "Grbl.h"
#include "Config.h"
#include "SettingsDefinitions.h"
#include "MachineConfig.h"
#include <atomic>

// Declare system global variable structure
system_t               sys;
int32_t                sys_position[MAX_N_AXIS];        // Real-time machine (aka home) position vector in steps.
int32_t                sys_probe_position[MAX_N_AXIS];  // Last probe position in machine coordinates and steps.
volatile ProbeState    sys_probe_state;                 // Probing state value.  Used to coordinate the probing cycle with stepper ISR.
volatile ExecState     sys_rt_exec_state;  // Global realtime executor bitflag variable for state management. See EXEC bitmasks.
volatile ExecAlarm     sys_rt_exec_alarm;  // Global realtime executor bitflag variable for setting various alarms.
volatile ExecAccessory sys_rt_exec_accessory_override;  // Global realtime executor bitflag variable for spindle/coolant overrides.
volatile bool          cycle_stop;                      // For state transitions, instead of bitflag
volatile void*         sys_pl_data_inflight;  // holds a plan_line_data_t while cartesian_to_motors has taken ownership of a line motion
#ifdef DEBUG
volatile bool sys_rt_exec_debug;
#endif
volatile Percent sys_rt_f_override;  // Global realtime executor feedrate override percentage
volatile Percent sys_rt_r_override;  // Global realtime executor rapid override percentage
volatile Percent sys_rt_s_override;  // Global realtime executor spindle override percentage

UserOutput::AnalogOutput*  myAnalogOutputs[MaxUserDigitalPin];
UserOutput::DigitalOutput* myDigitalOutputs[MaxUserDigitalPin];

xQueueHandle control_sw_queue;    // used by control switch debouncing
bool         debouncing = false;  // debouncing in process

void system_ini() {  // Renamed from system_init() due to conflict with esp32 files
    // setup control inputs

    if (ControlSafetyDoorPin->get().defined()) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Door switch on pin %s", ControlSafetyDoorPin->getStringValue());
        auto pin  = ControlSafetyDoorPin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (ControlResetPin->get().defined()) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Reset switch on pin %s", ControlResetPin->getStringValue());
        auto pin  = ControlResetPin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (ControlFeedHoldPin->get().defined()) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Hold switch on pin %s", ControlFeedHoldPin->getStringValue());
        auto pin  = ControlFeedHoldPin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (ControlCycleStartPin->get().defined()) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Start switch on pin %s", ControlCycleStartPin->getStringValue());
        auto pin  = ControlCycleStartPin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (MacroButton0Pin->get().defined()) {
        auto pin  = MacroButton0Pin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (MacroButton1Pin->get().defined()) {
        auto pin  = MacroButton1Pin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (MacroButton2Pin->get().defined()) {
        auto pin  = MacroButton2Pin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

    if (MacroButton3Pin->get().defined()) {
        auto pin  = MacroButton3Pin->get();
        auto attr = Pin::Attr::Input | Pin::Attr::ISR;
        if (pin.capabilities().has(Pins::PinCapabilities::PullUp)) {
            attr = attr | Pin::Attr::PullUp;
        }
        pin.setAttr(attr);
        pin.attachInterrupt(isr_control_inputs, CHANGE);
    }

#ifdef ENABLE_CONTROL_SW_DEBOUNCE
    // setup task used for debouncing
    control_sw_queue = xQueueCreate(10, sizeof(int));
    xTaskCreate(controlCheckTask,
                "controlCheckTask",
                3096,
                NULL,
                5,  // priority
                NULL);
#endif

    //customize pin definition if needed
#if (GRBL_SPI_SS != -1) || (GRBL_SPI_MISO != -1) || (GRBL_SPI_MOSI != -1) || (GRBL_SPI_SCK != -1)
    SPI.begin(GRBL_SPI_SCK, GRBL_SPI_MISO, GRBL_SPI_MOSI, GRBL_SPI_SS);
#endif

    // Setup M62,M63,M64,M65 pins
    for (int i = 0; i < 4; ++i) {
        myDigitalOutputs[i] = new UserOutput::DigitalOutput(i, UserDigitalPin[i]->get());
    }

    // Setup M67 Pins
    myAnalogOutputs[0] = new UserOutput::AnalogOutput(0, UserAnalogPin[0]->get(), USER_ANALOG_PIN_0_FREQ);
    myAnalogOutputs[1] = new UserOutput::AnalogOutput(1, UserAnalogPin[1]->get(), USER_ANALOG_PIN_1_FREQ);
    myAnalogOutputs[2] = new UserOutput::AnalogOutput(2, UserAnalogPin[2]->get(), USER_ANALOG_PIN_2_FREQ);
    myAnalogOutputs[3] = new UserOutput::AnalogOutput(3, UserAnalogPin[3]->get(), USER_ANALOG_PIN_3_FREQ);
}

#ifdef ENABLE_CONTROL_SW_DEBOUNCE
// this is the debounce task
void controlCheckTask(void* pvParameters) {
    while (true) {
        std::atomic_thread_fence(std::memory_order::memory_order_seq_cst);  // read fence for settings and other state
        int evt;
        xQueueReceive(control_sw_queue, &evt, portMAX_DELAY);  // block until receive queue
        vTaskDelay(CONTROL_SW_DEBOUNCE_PERIOD);                // delay a while
        ControlPins pins = system_control_get_state();
        if (pins.value) {
            system_exec_control_pin(pins);
        }
        debouncing = false;

        static UBaseType_t uxHighWaterMark = 0;
#    ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#    endif
    }
}
#endif

void IRAM_ATTR isr_control_inputs(void*) {
#ifdef ENABLE_CONTROL_SW_DEBOUNCE
    // we will start a task that will recheck the switches after a small delay
    int evt;
    if (!debouncing) {  // prevent resending until debounce is done
        debouncing = true;
        xQueueSendFromISR(control_sw_queue, &evt, NULL);
    }
#else
    ControlPins pins = system_control_get_state();
    system_exec_control_pin(pins);
#endif
}

// Returns if safety door is ajar(T) or closed(F), based on pin state.
uint8_t system_check_safety_door_ajar() {
#ifdef ENABLE_SAFETY_DOOR_INPUT_PIN
    return system_control_get_state().bit.safetyDoor;
#else
    return false;  // Input pin not enabled, so just return that it's closed.
#endif
}

void system_flag_wco_change() {
#ifdef FORCE_BUFFER_SYNC_DURING_WCO_CHANGE
    protocol_buffer_synchronize();
#endif
    sys.report_wco_counter = 0;
}

float system_convert_axis_steps_to_mpos(int32_t* steps, uint8_t idx) {
    float pos;
    float steps_per_mm = config->_axes->_axis[idx]->_stepsPerMm;
    pos                = steps[idx] / steps_per_mm;
    return pos;
}

void system_convert_array_steps_to_mpos(float* position, int32_t* steps) {
    auto  n_axis = config->_axes->_numberAxis;
    float motors[n_axis];
    for (int idx = 0; idx < n_axis; idx++) {
        motors[idx] = (float)steps[idx] / config->_axes->_axis[idx]->_stepsPerMm;
    }
    motors_to_cartesian(position, motors, n_axis);
}

float* system_get_mpos() {
    static float position[MAX_N_AXIS];
    system_convert_array_steps_to_mpos(position, sys_position);
    return position;
};

// Returns control pin state as a uint8 bitfield. Each bit indicates the input pin state, where
// triggered is 1 and not triggered is 0. Invert mask is applied. Bitfield organization is
// defined by the ControlPin in System.h.
ControlPins system_control_get_state() {
    ControlPins defined_pins;
    defined_pins.value = 0;

    ControlPins pin_states;
    pin_states.value = 0;

    defined_pins.bit.safetyDoor = ControlSafetyDoorPin->get().defined();
    if (ControlSafetyDoorPin->get().read()) {
        pin_states.bit.safetyDoor = true;
    }

    defined_pins.bit.reset = ControlResetPin->get().defined();
    if (ControlResetPin->get().read()) {
        pin_states.bit.reset = true;
    }

    defined_pins.bit.feedHold = ControlFeedHoldPin->get().defined();
    if (ControlFeedHoldPin->get().read()) {
        pin_states.bit.feedHold = true;
    }

    defined_pins.bit.cycleStart = ControlCycleStartPin->get().defined();
    if (ControlCycleStartPin->get().read()) {
        pin_states.bit.cycleStart = true;
    }

    defined_pins.bit.macro0 = MacroButton0Pin->get().defined();
    if (MacroButton0Pin->get().read()) {
        pin_states.bit.macro0 = true;
    }

    defined_pins.bit.macro1 = MacroButton1Pin->get().defined();
    if (MacroButton1Pin->get().read()) {
        pin_states.bit.macro1 = true;
    }

    defined_pins.bit.macro2 = MacroButton2Pin->get().defined();
    if (MacroButton2Pin->get().read()) {
        pin_states.bit.macro2 = true;
    }

    defined_pins.bit.macro3 = MacroButton3Pin->get().defined();
    if (MacroButton3Pin->get().read()) {
        pin_states.bit.macro3 = true;
    }

#ifdef INVERT_CONTROL_PIN_MASK
    pin_states.value ^= (INVERT_CONTROL_PIN_MASK & defined_pins.value);
#endif
    return pin_states;
}

// TODO: make this work with Control.cpp
// execute the function of the control pin
void system_exec_control_pin(ControlPins pins) {
    if (pins.bit.reset) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Reset via control pin");
        mc_reset();
    } else if (pins.bit.cycleStart) {
        sys_rt_exec_state.bit.cycleStart = true;
    } else if (pins.bit.feedHold) {
        sys_rt_exec_state.bit.feedHold = true;
    } else if (pins.bit.safetyDoor) {
        sys_rt_exec_state.bit.safetyDoor = true;
    } else if (pins.bit.macro0) {
        user_defined_macro(0);  // function must be implemented by user
    } else if (pins.bit.macro1) {
        user_defined_macro(1);  // function must be implemented by user
    } else if (pins.bit.macro2) {
        user_defined_macro(2);  // function must be implemented by user
    } else if (pins.bit.macro3) {
        user_defined_macro(3);  // function must be implemented by user
    }
}

void sys_digital_all_off() {
    for (uint8_t io_num = 0; io_num < MaxUserDigitalPin; io_num++) {
        myDigitalOutputs[io_num]->set_level(LOW);
    }
}

// io_num is the virtual digital pin#
bool sys_set_digital(uint8_t io_num, bool turnOn) {
    return myDigitalOutputs[io_num]->set_level(turnOn);
}

// Turn off all analog outputs
void sys_analog_all_off() {
    for (uint8_t io_num = 0; io_num < MaxUserDigitalPin; io_num++) {
        myAnalogOutputs[io_num]->set_level(0);
    }
}

// io_num is the virtual analog pin#
bool sys_set_analog(uint8_t io_num, float percent) {
    auto     analog    = myAnalogOutputs[io_num];
    uint32_t numerator = percent / 100.0 * analog->denominator();
    return analog->set_level(numerator);
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
    if (next_PWM_chan_num < 8) {           // 7 is the max PWM channel number
        return next_PWM_chan_num++;
    } else {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Error, "Error: out of PWM channels");
        return -1;
    }
}

/*
		Calculate the highest precision of a PWM based on the frequency in bits

		80,000,000 / freq = period
		determine the highest precision where (1 << precision) < period
	*/
uint8_t sys_calc_pwm_precision(uint32_t freq) {
    uint8_t precision = 0;

    // increase the precision (bits) until it exceeds allow by frequency the max or is 16
    while ((1 << precision) < (uint32_t)(80000000 / freq) && precision <= 16) {  // TODO is there a named value for the 80MHz?
        precision++;
    }

    return precision - 1;
}

void __attribute__((weak)) user_defined_macro(uint8_t index) {
    // must be in Idle
    if (sys.state != State::Idle) {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Macro button only permitted in idle");
        return;
    }

    String user_macro;
    char   line[255];
    switch (index) {
        case 0:
            user_macro = user_macro0->get();
            break;
        case 1:
            user_macro = user_macro1->get();
            break;
        case 2:
            user_macro = user_macro2->get();
            break;
        case 3:
            user_macro = user_macro3->get();
            break;
        default:
            return;
    }

    if (user_macro == "") {
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Macro User/Macro%d empty", index);
        return;
    }

    user_macro.replace('&', '\n');
    user_macro.toCharArray(line, 255, 0);
    strcat(line, "\r");
    WebUI::inputBuffer.push(line);
}

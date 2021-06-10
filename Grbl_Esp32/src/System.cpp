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
volatile ExecAlarm     sys_rt_exec_alarm;               // Global realtime executor bitflag variable for setting various alarms.
volatile ExecAccessory sys_rt_exec_accessory_override;  // Global realtime executor bitflag variable for spindle/coolant overrides.
volatile bool          rtStatusReport;
volatile bool          rtCycleStart;
volatile bool          rtFeedHold;
volatile bool          rtReset;
volatile bool          rtSafetyDoor;
volatile bool          rtMotionCancel;
volatile bool          rtSleep;
volatile bool          rtCycleStop;  // For state transitions, instead of bitflag
volatile bool          rtButtonMacro0;
volatile bool          rtButtonMacro1;
volatile bool          rtButtonMacro2;
volatile bool          rtButtonMacro3;
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

void system_reset() {
    // Reset system variables.
    State prior_state = sys.state;
    memset(&sys, 0, sizeof(system_t));  // Clear system struct variable.
    sys.state             = prior_state;
    sys.f_override        = FeedOverride::Default;              // Set to 100%
    sys.r_override        = RapidOverride::Default;             // Set to 100%
    sys.spindle_speed_ovr = SpindleSpeedOverride::Default;      // Set to 100%
    memset(sys_probe_position, 0, sizeof(sys_probe_position));  // Clear probe position.
}

void init_output_pins() {
    //customize pin definition if needed
#if (GRBL_SPI_SS != -1) || (GRBL_SPI_MISO != -1) || (GRBL_SPI_MOSI != -1) || (GRBL_SPI_SCK != -1)
    SPI.begin(GRBL_SPI_SCK, GRBL_SPI_MISO, GRBL_SPI_MOSI, GRBL_SPI_SS);
#endif

    auto userOutputs = config->_userOutputs;

    // Setup M62,M63,M64,M65 pins
    for (int i = 0; i < 4; ++i) {
        myDigitalOutputs[i] = new UserOutput::DigitalOutput(i, userOutputs->_digitalOutput[i]);
    }

    // Setup M67 Pins
    myAnalogOutputs[0] = new UserOutput::AnalogOutput(0, userOutputs->_analogOutput[0], USER_ANALOG_PIN_0_FREQ);
    myAnalogOutputs[1] = new UserOutput::AnalogOutput(1, userOutputs->_analogOutput[1], USER_ANALOG_PIN_1_FREQ);
    myAnalogOutputs[2] = new UserOutput::AnalogOutput(2, userOutputs->_analogOutput[2], USER_ANALOG_PIN_2_FREQ);
    myAnalogOutputs[3] = new UserOutput::AnalogOutput(3, userOutputs->_analogOutput[3], USER_ANALOG_PIN_3_FREQ);
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
        error_serial("Error: out of PWM channels");
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
        info_serial("Macro button only permitted in idle");
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
        info_serial("Macro User/Macro%d empty", index);
        return;
    }

    user_macro.replace('&', '\n');
    user_macro.toCharArray(line, 255, 0);
    strcat(line, "\r");
    WebUI::inputBuffer.push(line);
}

std::map<State, const char*> StateName = {
    { State::Idle, "Idle" },
    { State::Alarm, "Alarm" },
    { State::CheckMode, "CheckMode" },
    { State::Homing, "Homing" },
    { State::Cycle, "Cycle" },
    { State::Hold, "Hold" },
    { State::Jog, "Jog" },
    { State::SafetyDoor, "SafetyDoor" },
    { State::Sleep, "Sleep" },
    { State::ConfigAlarm, "ConfigAlarm" },
};

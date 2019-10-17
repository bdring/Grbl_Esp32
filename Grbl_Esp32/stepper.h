/*
  stepper.h - stepper motor driver: executes motion plans of planner.c using the stepper motors
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud
	
	2018 -	Bart Dring This file was modifed for use on the ESP32
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

#ifndef stepper_h
#define stepper_h

#ifndef SEGMENT_BUFFER_SIZE
  #define SEGMENT_BUFFER_SIZE 6
#endif



#include "grbl.h"
#include "config.h"

// Some useful constants.
#define DT_SEGMENT (1.0/(ACCELERATION_TICKS_PER_SECOND*60.0)) // min/segment
#define REQ_MM_INCREMENT_SCALAR 1.25
#define RAMP_ACCEL 0
#define RAMP_CRUISE 1
#define RAMP_DECEL 2
#define RAMP_DECEL_OVERRIDE 3

#define PREP_FLAG_RECALCULATE bit(0)
#define PREP_FLAG_HOLD_PARTIAL_BLOCK bit(1)
#define PREP_FLAG_PARKING bit(2)
#define PREP_FLAG_DECEL_OVERRIDE bit(3)

// Define Adaptive Multi-Axis Step-Smoothing(AMASS) levels and cutoff frequencies. The highest level
// frequency bin starts at 0Hz and ends at its cutoff frequency. The next lower level frequency bin
// starts at the next higher cutoff frequency, and so on. The cutoff frequencies for each level must
// be considered carefully against how much it over-drives the stepper ISR, the accuracy of the 16-bit
// timer, and the CPU overhead. Level 0 (no AMASS, normal operation) frequency bin starts at the
// Level 1 cutoff frequency and up to as fast as the CPU allows (over 30kHz in limited testing).
// NOTE: AMASS cutoff frequency multiplied by ISR overdrive factor must not exceed maximum step frequency.
// NOTE: Current settings are set to overdrive the ISR to no more than 16kHz, balancing CPU overhead
// and timer accuracy.  Do not alter these settings unless you know what you are doing.
///#ifdef ADAPTIVE_MULTI_AXIS_STEP_SMOOTHING
  #define MAX_AMASS_LEVEL 3
  // AMASS_LEVEL0: Normal operation. No AMASS. No upper cutoff frequency. Starts at LEVEL1 cutoff frequency.
	// Note ESP32 use F_STEPPER_TIMER rather than the AVR F_CPU
  #define AMASS_LEVEL1 (F_STEPPER_TIMER/8000) // Over-drives ISR (x2). Defined as F_CPU/(Cutoff frequency in Hz)
  #define AMASS_LEVEL2 (F_STEPPER_TIMER/4000) // Over-drives ISR (x4)
  #define AMASS_LEVEL3 (F_STEPPER_TIMER/2000) // Over-drives ISR (x8)

  #if MAX_AMASS_LEVEL <= 0
    error "AMASS must have 1 or more levels to operate correctly."
  #endif
//#endif

#define STEP_TIMER_GROUP TIMER_GROUP_0
#define STEP_TIMER_INDEX TIMER_0 

// esp32 work around for diable in main loop
extern uint64_t stepper_idle_counter;
extern bool stepper_idle;

extern uint8_t ganged_mode;

// -- Task handles for use in the notifications
void IRAM_ATTR onSteppertimer();
void IRAM_ATTR onStepperOffTimer();

#ifdef USE_RMT_STEPS
  void initRMT();
#endif

void stepper_init(); 

// Enable steppers, but cycle does not start unless called by motion control or realtime command.
void st_wake_up();

// Immediately disables steppers
void st_go_idle();

// Generate the step and direction port invert masks.
void st_generate_step_dir_invert_masks();

// Reset the stepper subsystem variables
void st_reset();

// Changes the run state of the step segment buffer to execute the special parking motion.
void st_parking_setup_buffer();

// Restores the step segment buffer to the normal run state after a parking motion.
void st_parking_restore_buffer();

// Reloads step segment buffer. Called continuously by realtime execution system.
void st_prep_buffer();

// Called by planner_recalculate() when the executing block is updated by the new plan.
void st_update_plan_block_parameters();

// Called by realtime status reporting if realtime rate reporting is enabled in config.h.
float st_get_realtime_rate();

// disable (or enable) steppers via STEPPERS_DISABLE_PIN
void set_stepper_disable(uint8_t disable);
bool get_stepper_disable(); // returns the state of the pin

void set_step_pin_on(uint8_t axis, uint8_t isOn);
void set_direction_pin_on(uint8_t axis, uint8_t isOn);
void set_stepper_pins_on(uint8_t onMask);
void set_direction_pins_on(uint8_t onMask);

void Stepper_Timer_WritePeriod(uint64_t alarm_val);
void Stepper_Timer_Start();
void Stepper_Timer_Stop();

#endif
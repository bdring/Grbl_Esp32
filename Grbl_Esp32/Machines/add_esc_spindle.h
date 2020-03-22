/*
   add_esc_spindle.h
   Part of Grbl_ESP32

   This is an additional configuration fragment that can be
   included after a base configuration file.  The base file
   establishes most settings and the add-on changes a few things.
   For example, in machines.h, you would write:
   #include "Machines/3axis_v4.h"             // Basic pin assignments
   #include "Machines/add_esc_spindle.h"      // Add-ons for ESC spindle

   This uses a Brushless DC Hobby motor as a spindle motor.  See:
   https://github.com/bdring/Grbl_Esp32/wiki/BESC-Spindle-Feature

   2019  -  Bart Dring
   2020  -  Mitch Bradley

   Grbl_ESP32 is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Grbl is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/


// MACHINE_EXTRA is appended to MACHINE_NAME for startup message display
#define MACHINE_EXTRA "_ESC_SPINDLE"

#define SHOW_EXTENDED_SETTINGS

#define SPINDLE_PWM_BIT_PRECISION 16   // 16 bit recommended for ESC (don't change)

/*
 Important ESC Settings
 $33=50 // Hz this is the typical good frequency for an ESC
 #define DEFAULT_SPINDLE_FREQ 5000.0 // $33 Hz (extended set)

 Determine the typical min and max pulse length of your ESC
  min_pulse is typically 1ms (0.001 sec) or less
  max_pulse is typically 2ms (0.002 sec) or more

 determine PWM_period. It is (1/freq) if freq = 50...period = 0.02

 determine pulse length for min_pulse and max_pulse in percent.

  (pulse / PWM_period)
   min_pulse = (0.001 / 0.02) = 0.05 = 5%  so ... $34 and $35 = 5.0
   max_pulse = (0.002 / .02) = 0.1 = 10%  so ... $36=10
*/

#define DEFAULT_SPINDLE_FREQ            50.0
#define DEFAULT_SPINDLE_OFF_VALUE       5.0
#define DEFAULT_SPINDLE_MIN_VALUE       5.0
#define DEFAULT_SPINDLE_MAX_VALUE       10.0

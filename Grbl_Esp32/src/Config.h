#pragma once

/*
  Config.h - compile time configuration
  Part of Grbl

  Copyright (c) 2012-2016 Sungeun K. Jeon for Gnea Research LLC
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

// This file contains compile-time configurations for Grbl's internal system. For the most part,
// users will not need to directly modify these, but they are here for specific needs, i.e.
// performance tuning or adjusting to non-typical machines.

// IMPORTANT: Any changes here requires a full re-compiling of the source code to propagate them.

/*
ESP 32 Notes

Some features should not be changed. See notes below.

*/

#include <Arduino.h>

#include "NutsBolts.h"

// It is no longer necessary to edit this file to choose
// a machine configuration; edit machine.h instead
// machine.h is #included below, after some definitions
// that the machine file might choose to undefine.

// Note: HOMING_CYCLES are now settings
#define SUPPORT_TASK_CORE 1  // Reference: CONFIG_ARDUINO_RUNNING_CORE = 1

// #define ENABLE_CONTROL_SW_DEBOUNCE     // Default disabled. Uncomment to enable.
#define CONTROL_SW_DEBOUNCE_PERIOD 32  // in milliseconds default 32 microseconds

// Include the file that loads the machine-specific config file.
// machine.h must be edited to choose the desired file.
#include "Machine.h"

// machine_common.h contains settings that do not change
#include "MachineCommon.h"

const int MAX_N_AXIS = 6;

// Serial baud rate
// OK to change, but the ESP32 boot text is 115200, so you will not see that is your
// serial monitor, sender, etc uses a different value than 115200
const int BAUD_RATE = 115200;

//Connect to your local AP with these credentials
//#define CONNECT_TO_SSID  "your SSID"
//#define SSID_PASSWORD  "your SSID password"
//CONFIGURE_EYECATCH_BEGIN (DO NOT MODIFY THIS LINE)

// #define ENABLE_BLUETOOTH  // enable bluetooth -- platformio: pio run -e bt
// #define ENABLE_WIFI       // enable wifi      -- platformio: pio run -e wifi

// Warning! The current authentication implementation is too weak to provide
// security against an attacker, since passwords are stored and transmitted
// "in the clear" over unsecured channels.  It should be treated as a
// "friendly suggestion" to prevent unwitting dangerous actions, rather than
// as effective security against malice.
// #define ENABLE_AUTHENTICATION
// CONFIGURE_EYECATCH_END (DO NOT MODIFY THIS LINE)

#ifdef ENABLE_AUTHENTICATION
const char* const DEFAULT_ADMIN_PWD   = "admin";
const char* const DEFAULT_USER_PWD    = "user";
const char* const DEFAULT_ADMIN_LOGIN = "admin";
const char* const DEFAULT_USER_LOGIN  = "user";
#endif

//Radio Mode
const int ESP_RADIO_OFF = 0;
const int ESP_WIFI_STA  = 1;
const int ESP_WIFI_AP   = 2;
const int ESP_BT        = 3;

//Default mode
#ifdef ENABLE_WIFI
#    ifdef CONNECT_TO_SSID
const int DEFAULT_RADIO_MODE = ESP_WIFI_STA;
#    else
const int DEFAULT_RADIO_MODE = ESP_WIFI_AP;
#    endif  //CONNECT_TO_SSID
#else

// TODO FIXME!
#    ifdef ENABLE_BLUETOOTH
const int DEFAULT_RADIO_MODE = ESP_BT;
#    else
const int DEFAULT_RADIO_MODE = ESP_RADIO_OFF;
#    endif
#endif

// Number of homing cycles performed after when the machine initially jogs to limit switches.
// This help in preventing overshoot and should improve repeatability. This value should be one or
// greater.
static const uint8_t NHomingLocateCycle = 1;  // Integer (1-128)

// Number of blocks Grbl executes upon startup. These blocks are stored in non-volatile storage.
// and addresses are defined in settings.h. With the current settings, up to 2 startup blocks may
// be stored and executed in order. These startup blocks would typically be used to set the GCode
// parser state depending on user preferences.
// #define N_STARTUP_LINE 2  // Integer (1-2)

// Number of floating decimal points printed by Grbl for certain value types. These settings are
// determined by realistic and commonly observed values in CNC machines. For example, position
// values cannot be less than 0.001mm or 0.0001in, because machines can not be physically more
// precise this. So, there is likely no need to change these, but you can if you need to here.
// NOTE: Must be an integer value from 0 to ~4. More than 4 may exhibit round-off errors.
// ESP32 Note: These are mostly hard coded, so these values will not change anything

// Upon a successful probe cycle, this option provides immediately feedback of the probe coordinates
// through an automatically generated message. If disabled, users can still access the last probe
// coordinates through Grbl '$#' print parameters.
const bool MESSAGE_PROBE_COORDINATES = true;  // Enabled by default. Comment to disable.

// Enables a second coolant control pin via the mist coolant GCode command M7 on the Arduino Uno
// analog pin 4. Only use this option if you require a second coolant control pin.
// NOTE: The M8 flood coolant control pin on analog pin 3 will still be functional regardless.
// ESP32 NOTE! This is here for reference only. You enable both M7 and M8 by assigning them a GPIO Pin
// in the machine definition file.
//#define ENABLE_M7 // Don't uncomment...see above!

// When Grbl powers-cycles or is hard reset with the Arduino reset button, Grbl boots up with no ALARM
// by default. This is to make it as simple as possible for new users to start using Grbl. When homing
// is enabled and a user has installed limit switches, Grbl will boot up in an ALARM state to indicate
// Grbl doesn't know its position and to force the user to home before proceeding. This option forces
// Grbl to always initialize into an ALARM state regardless of homing or not. This option is more for
// OEMs and LinuxCNC users that would like this power-cycle behavior.
const bool FORCE_INITIALIZATION_ALARM = false;  // Default disabled. Uncomment to enable.

// ---------------------------------------------------------------------------------------
// ADVANCED CONFIGURATION OPTIONS:

// Configure rapid, feed, and spindle override settings. These values define the max and min
// allowable override values and the coarse and fine increments per command received. Please
// note the allowable values in the descriptions following each define.
namespace FeedOverride {
    const int Default         = 100;  // 100%. Don't change this value.
    const int Max             = 200;  // Percent of programmed feed rate (100-255). Usually 120% or 200%
    const int Min             = 10;   // Percent of programmed feed rate (1-100). Usually 50% or 1%
    const int CoarseIncrement = 10;   // (1-99). Usually 10%.
    const int FineIncrement   = 1;    // (1-99). Usually 1%.
};
namespace RapidOverride {
    const int Default  = 100;  // 100%. Don't change this value.
    const int Medium   = 50;   // Percent of rapid (1-99). Usually 50%.
    const int Low      = 25;   // Percent of rapid (1-99). Usually 25%.
    const int ExtraLow = 5;    // Percent of rapid (1-99). Usually 5%.  Not Supported
};

namespace SpindleSpeedOverride {
    const int Default         = 100;  // 100%. Don't change this value.
    const int Max             = 200;  // Percent of programmed spindle speed (100-255). Usually 200%.
    const int Min             = 10;   // Percent of programmed spindle speed (1-100). Usually 10%.
    const int CoarseIncrement = 10;   // (1-99). Usually 10%.
    const int FineIncrement   = 1;    // (1-99). Usually 1%.
}

// When a M2 or M30 program end command is executed, most GCode states are restored to their defaults.
// This compile-time option includes the restoring of the feed, rapid, and spindle speed override values
// to their default values at program end.
const bool RESTORE_OVERRIDES_AFTER_PROGRAM_END = true;  // Default enabled. Comment to disable.

// Some status report data isn't necessary for realtime, only intermittently, because the values don't
// change often. The following macros configures how many times a status report needs to be called before
// the associated data is refreshed and included in the status report. However, if one of these value
// changes, Grbl will automatically include this data in the next status report, regardless of what the
// count is at the time. This helps reduce the communication overhead involved with high frequency reporting
// and agressive streaming. There is also a busy and an idle refresh count, which sets up Grbl to send
// refreshes more often when its not doing anything important. With a good GUI, this data doesn't need
// to be refreshed very often, on the order of a several seconds.
// NOTE: WCO refresh must be 2 or greater. OVR refresh must be 1 or greater.
const int REPORT_OVR_REFRESH_BUSY_COUNT = 20;  // (1-255)
const int REPORT_OVR_REFRESH_IDLE_COUNT = 10;  // (1-255) Must be less than or equal to the busy count
const int REPORT_WCO_REFRESH_BUSY_COUNT = 30;  // (2-255)
const int REPORT_WCO_REFRESH_IDLE_COUNT = 10;  // (2-255) Must be less than or equal to the busy count

// The temporal resolution of the acceleration management subsystem. A higher number gives smoother
// acceleration, particularly noticeable on machines that run at very high feedrates, but may negatively
// impact performance. The correct value for this parameter is machine dependent, so it's advised to
// set this only as high as needed. Approximate successful values can widely range from 50 to 200 or more.
// NOTE: Changing this value also changes the execution time of a segment in the step segment buffer.
// When increasing this value, this stores less overall time in the segment buffer and vice versa. Make
// certain the step segment buffer is increased/decreased to account for these changes.
const int ACCELERATION_TICKS_PER_SECOND = 100;

// Sets the maximum step rate allowed to be written as a Grbl setting. This option enables an error
// check in the settings module to prevent settings values that will exceed this limitation. The maximum
// step rate is strictly limited by the CPU speed and will change if something other than an AVR running
// at 16MHz is used.
// NOTE: For now disabled, will enable if flash space permits.
// #define MAX_STEP_RATE_HZ 30000 // Hz

// Sets which axis the tool length offset is applied. Assumes the spindle is always parallel with
// the selected axis with the tool oriented toward the negative direction. In other words, a positive
// tool length offset value is subtracted from the current location.
const int TOOL_LENGTH_OFFSET_AXIS = Z_AXIS;  // Default z-axis. Valid values are X_AXIS, Y_AXIS, or Z_AXIS.

// Minimum planner junction speed. Sets the default minimum junction speed the planner plans to at
// every buffer block junction, except for starting from rest and end of the buffer, which are always
// zero. This value controls how fast the machine moves through junctions with no regard for acceleration
// limits or angle between neighboring block line move directions. This is useful for machines that can't
// tolerate the tool dwelling for a split second, i.e. 3d printers or laser cutters. If used, this value
// should not be much greater than zero or to the minimum value necessary for the machine to work.
const double MINIMUM_JUNCTION_SPEED = 0.0;  // (mm/min)

// Sets the minimum feed rate the planner will allow. Any value below it will be set to this minimum
// value. This also ensures that a planned motion always completes and accounts for any floating-point
// round-off errors. Although not recommended, a lower value than 1.0 mm/min will likely work in smaller
// machines, perhaps to 0.1mm/min, but your success may vary based on multiple factors.
const double MINIMUM_FEED_RATE = 1.0;  // (mm/min)

// Number of arc generation iterations by small angle approximation before exact arc trajectory
// correction with expensive sin() and cos() calcualtions. This parameter maybe decreased if there
// are issues with the accuracy of the arc generations, or increased if arc execution is getting
// bogged down by too many trig calculations.
const int N_ARC_CORRECTION = 12;  // Integer (1-255)

// The arc G2/3 GCode standard is problematic by definition. Radius-based arcs have horrible numerical
// errors when arc at semi-circles(pi) or full-circles(2*pi). Offset-based arcs are much more accurate
// but still have a problem when arcs are full-circles (2*pi). This define accounts for the floating
// point issues when offset-based arcs are commanded as full circles, but get interpreted as extremely
// small arcs with around machine epsilon (1.2e-7rad) due to numerical round-off and precision issues.
// This define value sets the machine epsilon cutoff to determine if the arc is a full-circle or not.
// NOTE: Be very careful when adjusting this value. It should always be greater than 1.2e-7 but not too
// much greater than this. The default setting should capture most, if not all, full arc error situations.
const double ARC_ANGULAR_TRAVEL_EPSILON = 5E-7;  // Float (radians)

// Time delay increments performed during a dwell. The default value is set at 50ms, which provides
// a maximum time delay of roughly 55 minutes, more than enough for most any application. Increasing
// this delay will increase the maximum dwell time linearly, but also reduces the responsiveness of
// run-time command executions, like status reports, since these are performed between each dwell
// time step. Also, keep in mind that the Arduino delay timer is not very accurate for long delays.
const int DWELL_TIME_STEP = 50;  // Integer (1-255) (milliseconds)

// Serial send and receive buffer size. The receive buffer is often used as another streaming
// buffer to store incoming blocks to be processed by Grbl when its ready. Most streaming
// interfaces will character count and track each block send to each block response. So,
// increase the receive buffer if a deeper receive buffer is needed for streaming and avaiable
// memory allows. The send buffer primarily handles messages in Grbl. Only increase if large
// messages are sent and Grbl begins to stall, waiting to send the rest of the message.
// NOTE: Grbl generates an average status report in about 0.5msec, but the serial TX stream at
// 115200 baud will take 5 msec to transmit a typical 55 character report. Worst case reports are
// around 90-100 characters. As long as the serial TX buffer doesn't get continually maxed, Grbl
// will continue operating efficiently. Size the TX buffer around the size of a worst-case report.
// #define RX_BUFFER_SIZE 128 // (1-254) Uncomment to override defaults in serial.h
// #define TX_BUFFER_SIZE 100 // (1-254)

// A simple software debouncing feature for hard limit switches. When enabled, the limit
// switch interrupt unblock a waiting task which will recheck the limit switch pins after
// a short delay. Default disabled
//#define ENABLE_SOFTWARE_DEBOUNCE // Default disabled. Uncomment to enable.
const int DEBOUNCE_PERIOD = 32;  // in milliseconds default 32 microseconds

// Configures the position after a probing cycle during Grbl's check mode. Disabled sets
// the position to the probe target, when enabled sets the position to the start position.
// #define SET_CHECK_MODE_PROBE_TO_START // Default disabled. Uncomment to enable.

// Force Grbl to check the state of the hard limit switches when the processor detects a pin
// change inside the hard limit ISR routine. By default, Grbl will trigger the hard limits
// alarm upon any pin change, since bouncing switches can cause a state check like this to
// misread the pin. When hard limits are triggered, they should be 100% reliable, which is the
// reason that this option is disabled by default. Only if your system/electronics can guarantee
// that the switches don't bounce, we recommend enabling this option. This will help prevent
// triggering a hard limit when the machine disengages from the switch.
// NOTE: This option has no effect if SOFTWARE_DEBOUNCE is enabled.
// #define HARD_LIMIT_FORCE_STATE_CHECK // Default disabled. Uncomment to enable.

// Adjusts homing cycle search and locate scalars. These are the multipliers used by Grbl's
// homing cycle to ensure the limit switches are engaged and cleared through each phase of
// the cycle. The search phase uses the axes max-travel setting times the SEARCH_SCALAR to
// determine distance to look for the limit switch. Once found, the locate phase begins and
// uses the homing pull-off distance setting times the LOCATE_SCALAR to pull-off and re-engage
// the limit switch.
// NOTE: Both of these values must be greater than 1.0 to ensure proper function.
// #define HOMING_AXIS_SEARCH_SCALAR  1.5 // Uncomment to override defaults in limits.c.
// #define HOMING_AXIS_LOCATE_SCALAR  10.0 // Uncomment to override defaults in limits.c.

// Additional settings have been added to the original set that you see with the $$ command
// Some senders may not be able to parse anything different from the original set
// You can still set these like $33=5000, but you cannot read them back.
// Default is off to limit support issues...you can enable here or in your machine definition file
// #define SHOW_EXTENDED_SETTINGS

// Writing to non-volatile storage (NVS) can take a long time and interfere with timely instruction
// execution, causing problems for the stepper ISRs and serial comm ISRs and subsequent loss of
// stepper position and serial data. This configuration option forces the planner buffer to completely
// empty whenever the NVS is written, to prevent any chance of lost steps.
// It doesn't prevent loss of serial Rx data, especially if a GUI is premptively filling up the
// serial Rx buffer.  GUIs should detect GCodes that write to NVS - notably G10,G28.1,G30.1 -
// and wait for an 'ok' before sending more data.
// NOTE: Most setting changes - $ commands - are blocked when a job is running. Coordinate setting
// GCode commands (G10,G28/30.1) are not blocked, since they are part of an active streaming job.
// This option forces a planner buffer sync only with such GCode commands.
const bool FORCE_BUFFER_SYNC_DURING_NVS_WRITE = true;  // Default enabled. Comment to disable.

// In Grbl v0.9 and prior, there is an old outstanding bug where the `WPos:` work position reported
// may not correlate to what is executing, because `WPos:` is based on the GCode parser state, which
// can be several motions behind. This option forces the planner buffer to empty, sync, and stop
// motion whenever there is a command that alters the work coordinate offsets `G10,G43.1,G92,G54-59`.
// This is the simplest way to ensure `WPos:` is always correct. Fortunately, it's exceedingly rare
// that any of these commands are used need continuous motions through them.
const bool FORCE_BUFFER_SYNC_DURING_WCO_CHANGE = true;  // Default enabled. Comment to disable.

// By default, Grbl disables feed rate overrides for all G38.x probe cycle commands. Although this
// may be different than some pro-class machine control, it's arguable that it should be this way.
// Most probe sensors produce different levels of error that is dependent on rate of speed. By
// keeping probing cycles to their programmed feed rates, the probe sensor should be a lot more
// repeatable. If needed, you can disable this behavior by uncommenting the define below.
const bool ALLOW_FEED_OVERRIDE_DURING_PROBE_CYCLES = false;

// Configure options for the parking motion, if enabled.
#define PARKING_AXIS Z_AXIS                      // Define which axis that performs the parking motion
const double PARKING_TARGET            = -5.0;   // Parking axis target. In mm, as machine coordinate.
const double PARKING_RATE              = 800.0;  // Parking fast rate after pull-out in mm/min.
const double PARKING_PULLOUT_RATE      = 250.0;  // Pull-out/plunge slow feed rate in mm/min.
const double PARKING_PULLOUT_INCREMENT = 5.0;    // Spindle pull-out and plunge distance in mm. Incremental distance.
// Must be positive value or equal to zero.

// Enables a piecewise linear model of the spindle PWM/speed output. Requires a solution by the
// 'fit_nonlinear_spindle.py' script in the /doc/script folder of the repo. See file comments
// on how to gather spindle data and run the script to generate a solution.
// #define ENABLE_PIECEWISE_LINEAR_SPINDLE  // Default disabled. Uncomment to enable.

// N_PIECES, RPM_MAX, RPM_MIN, RPM_POINTxx, and RPM_LINE_XX constants are all set and given by
// the 'fit_nonlinear_spindle.py' script solution. Used only when ENABLE_PIECEWISE_LINEAR_SPINDLE
// is enabled. Make sure the constant values are exactly the same as the script solution.
// NOTE: When N_PIECES < 4, unused RPM_LINE and RPM_POINT defines are not required and omitted.
/*
#define N_PIECES 4  // Integer (1-4). Number of piecewise lines used in script solution.
#define RPM_MAX  11686.4  // Max RPM of model. $30 > RPM_MAX will be limited to RPM_MAX.
#define RPM_MIN  202.5    // Min RPM of model. $31 < RPM_MIN will be limited to RPM_MIN.
*/

const int N_PIECES = 3;

const double RPM_MAX = 23935.2;
const double RPM_MIN = 2412.2;

/*
  Grbl.cpp - Initialization and main loop for Grbl
  Part of Grbl
  Copyright (c) 2014-2016 Sungeun K. Jeon for Gnea Research LLC

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

#include "Grbl.h"
#include "MachineConfig.h"

#include <WiFi.h>
#include <SPIFFS.h>

extern void make_grbl_commands();

void grbl_init() {
    try {
        client_init();  // Setup serial baud rate and interrupts

        debug_serial("Initializing WiFi...");
        WiFi.persistent(false);
        WiFi.disconnect(true);
        WiFi.enableSTA(false);
        WiFi.enableAP(false);
        WiFi.mode(WIFI_OFF);

        display_init();
        info_serial("Grbl_ESP32 Ver %s Date %s", GRBL_VERSION, GRBL_VERSION_BUILD);  // print grbl_esp32 verion info
        info_serial("Compiled with ESP32 SDK:%s", ESP.getSdkVersion());              // print the SDK version
                                                                                     // show the map name at startup

#ifdef MACHINE_NAME
        report_machine_type(CLIENT_SERIAL);
#endif

        if (!SPIFFS.begin(true)) {
            log_error("Cannot mount the local filesystem");
        }

        // Load Grbl settings from non-volatile storage
        debug_serial("Initializing settings...");
        settings_init(); // requires config
        config->load(config_filename->get());
        make_grbl_commands();

        info_serial("Name: %s", config->_name.c_str());
        info_serial("Board: %s", config->_board.c_str());

        if (config->_i2so) {
            info_serial("Initializing I2SO...");
            // The I2S out must be initialized before it can access the expanded GPIO port. Must be initialized _after_ settings!
            i2s_out_init();
        }
        if (config->_spi) {
            info_serial("Initializing SPI...");
            // The SPI must be initialized before we can use it.
            config->_spi->init();

            // Initialize SD card after SPI:
            if (config->_sdCard != nullptr) {
                config->_sdCard->init();
            }
        }

        info_serial("Initializing steppers...");
        stepper_init();  // Configure stepper pins and interrupt timers

        info_serial("Initializing axes...");
        config->_axes->read_settings();
        config->_axes->init();

        config->_control->init();
        init_output_pins();  // Configure pinout pins and pin-change interrupt (Renamed due to conflict with esp32 files)
        memset(sys_position, 0, sizeof(sys_position));  // Clear machine position.

        machine_init();  // user supplied function for special initialization

        // Initialize system state.
        if (sys.state != State::ConfigAlarm) {
#ifdef FORCE_INITIALIZATION_ALARM
            // Force Grbl into an ALARM state upon a power-cycle or hard reset.
            sys.state = State::Alarm;
#else
            sys.state = State::Idle;
#endif
        }
        // Check for power-up and set system alarm if homing is enabled to force homing cycle
        // by setting Grbl's alarm state. Alarm locks out all g-code commands, including the
        // startup scripts, but allows access to settings and internal commands. Only a homing
        // cycle '$H' or kill alarm locks '$X' will disable the alarm.
        // NOTE: The startup script will run after successful completion of the homing cycle, but
        // not after disabling the alarm locks. Prevents motion startup blocks from crashing into
        // things uncontrollably. Very bad.
        if (config->_homingInitLock && homingAxes() && sys.state != State::ConfigAlarm) {
            // If there is an axis with homing configured, enter Alarm state on startup
            sys.state = State::Alarm;
        }
        config->_spindle->init();
        config->_coolant->init();
        limits_init();
        config->_probe->init();

#ifdef ENABLE_WIFI
        info_serial("Initializing WiFi...");
        WebUI::wifi_config.begin();
#endif
#ifdef ENABLE_BLUETOOTH
        if (hasBluetooth()) {
            info_serial("Initializing Bluetooth...");

            config->_comms->_bluetoothConfig->begin();
        }
#endif
        WebUI::inputBuffer.begin();
    } catch (const AssertionFailed& ex) {
        // This means something is terribly broken:
        info_serial("Critical error in run_once: %s", ex.what());
        sys.state = State::ConfigAlarm;
        sleep(10000);
        throw;
    }
}

static void reset_variables() {
    // Reset Grbl primary systems.
    system_reset();
    protocol_reset();
    gc_init();  // Set g-code parser to default state
    config->_spindle->stop();
    plan_reset();  // Clear block buffer and planner variables
    st_reset();    // Clear stepper subsystem variables
    // Sync cleared gcode and planner positions to current system position.
    plan_sync_position();
    gc_sync_position();
    report_init_message(CLIENT_ALL);

    // used to keep track of a jog command sent to mc_line() so we can cancel it.
    // this is needed if a jogCancel comes along after we have already parsed a jog and it is in-flight.
    sys_pl_data_inflight = NULL;
}

void run_once() {
    try {
        reset_variables();
        // Start Grbl main loop. Processes program inputs and executes them.
        // This can exit on a system abort condition, in which case run_once()
        // is re-executed by an enclosing loop.
        protocol_main_loop();
    } catch (const AssertionFailed& ex) {
        // This means something is terribly broken:
        error_all("Critical error in run_once: %s", ex.msg.c_str());
        sleep(10000);
        throw;
    }
}

void __attribute__((weak)) machine_init() {}

void __attribute__((weak)) display_init() {}

void __attribute__((weak)) user_m30() {}

void __attribute__((weak)) user_tool_change(uint8_t new_tool) {}

/*
  setup() and loop() in the Arduino .ino implements this control flow:

  void main() {
     init();          // setup()
     while (1) {      // loop()
         run_once();
     }
  }
*/

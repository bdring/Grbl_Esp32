/*
  serial.cpp - Header for system level commands and real-time processes
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

  What is going on here?

  Original Grbl only supports communication via serial port. That is why this
  file is call serial.cpp. Grbl_ESP32 supports many "clients".

  Clients are sources of commands like the serial port or a bluetooth connection.
  Multiple clients can be active at a time. If a client asks for status, only the client will
  receive the reply to the command.

  The serial port acts as the debugging port because it is always on and does not
  need to be reconnected after reboot. Messages about the configuration and other events
  are sent to the serial port automatically, without a request command. These are in the
  [MSG: xxxxxx] format. Gcode senders are should be OK with this because Grbl has always
  send some messages like this.

  Important: It is up user that the clients play well together. Ideally, if one client
  is sending the gcode, the others should just be doing status, feedhold, etc.

  Clients send gcode, grbl commands ($$, [ESP...], etc) and realtime commands (?,!.~, etc)
  Gcode and Grbl commands are a string of printable characters followed by a '\r' or '\n'
  Realtime commands are single characters with no '\r' or '\n'

  After sending a gcode or grbl command, you must wait for an OK to send another.
  This is because only a certain number of commands can be buffered at a time.
  Grbl will tell you when it is ready for another one with the OK.

  Realtime commands can be sent at any time and will acted upon very quickly.
  Realtime commands can be anywhere in the stream.

  To allow the realtime commands to be randomly mixed in the stream of data, we
  read all clients as fast as possible. The realtime commands are acted upon and the other charcters are
  placed into a client_buffer[client].

  The main protocol loop reads from client_buffer[]


*/

#include "grbl.h"

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

static TaskHandle_t serialCheckTaskHandle = 0;

InputBuffer client_buffer[CLIENT_COUNT];  // create a buffer for each client

// Returns the number of bytes available in a client buffer.
uint8_t serial_get_rx_buffer_available(uint8_t client) {
    return client_buffer[client].availableforwrite();
}

void serial_init() {
    Serial.begin(BAUD_RATE);
    // reset all buffers
    serial_reset_read_buffer(CLIENT_ALL);
    grbl_send(CLIENT_SERIAL, "\r\n");  // create some white space after ESP32 boot info
    serialCheckTaskHandle = 0;
    // create a task to check for incoming data
    xTaskCreatePinnedToCore(serialCheckTask,    // task
                            "serialCheckTask",  // name for task
                            8192,               // size of task stack
                            NULL,               // parameters
                            1,                  // priority
                            &serialCheckTaskHandle,
                            1  // core
    );
}

// this task runs and checks for data on all interfaces
// REaltime stuff is acted upon, then characters are added to the appropriate buffer
void serialCheckTask(void* pvParameters) {
    uint8_t data   = 0;
    uint8_t client = CLIENT_ALL;  // who sent the data
    while (true) {                // run continuously
        while (any_client_has_data()) {
            if (Serial.available()) {
                client = CLIENT_SERIAL;
                data   = Serial.read();
            } else if (inputBuffer.available()) {
                client = CLIENT_INPUT;
                data   = inputBuffer.read();
            } else {
                //currently is wifi or BT but better to prepare both can be live
#ifdef ENABLE_BLUETOOTH
                if (SerialBT.hasClient() && SerialBT.available()) {
                    client = CLIENT_BT;
                    data   = SerialBT.read();
                    //Serial.write(data);  // echo all data to serial
                } else {
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_IN)
                    if (Serial2Socket.available()) {
                        client = CLIENT_WEBUI;
                        data   = Serial2Socket.read();
                    } else {
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)
                        if (telnet_server.available()) {
                            client = CLIENT_TELNET;
                            data   = telnet_server.read();
                        }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_IN)
                    }
#endif
#ifdef ENABLE_BLUETOOTH
                }
#endif
            }
            // Pick off realtime command characters directly from the serial stream. These characters are
            // not passed into the main buffer, but these set system state flag bits for realtime execution.
            if (is_realtime_command(data))
                execute_realtime_command(data, client);
            else {
                vTaskEnterCritical(&myMutex);
                client_buffer[client].write(data);
                vTaskExitCritical(&myMutex);
            }
        }  // if something available
        COMMANDS::handle();
#ifdef ENABLE_WIFI
        wifi_config.handle();
#endif
#ifdef ENABLE_BLUETOOTH
        bt_config.handle();
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_IN)
        Serial2Socket.handle_flush();
#endif
        vTaskDelay(1 / portTICK_RATE_MS);  // Yield to other tasks
    }                                      // while(true)
}

void serial_reset_read_buffer(uint8_t client) {
    for (uint8_t client_num = 0; client_num < CLIENT_COUNT; client_num++) {
        if (client == client_num || client == CLIENT_ALL)
            client_buffer[client_num].begin();
    }
}

// Writes one byte to the TX serial buffer. Called by main program.
void serial_write(uint8_t data) {
    Serial.write((char)data);
}

// Fetches the first byte in the serial read buffer. Called by protocol loop.
uint8_t serial_read(uint8_t client) {
    uint8_t data;
    vTaskEnterCritical(&myMutex);
    if (client_buffer[client].available()) {
        data = client_buffer[client].read();
        vTaskExitCritical(&myMutex);
        //Serial.write((char)data);
        return data;
    } else {
        vTaskExitCritical(&myMutex);
        return SERIAL_NO_DATA;
    }
}

bool any_client_has_data() {
    return (Serial.available() || inputBuffer.available()
#ifdef ENABLE_BLUETOOTH
            || (SerialBT.hasClient() && SerialBT.available())
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_IN)
            || Serial2Socket.available()
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)
            || telnet_server.available()
#endif
    );
}

// checks to see if a character is a realtime character
bool is_realtime_command(uint8_t data) {
    return (data == CMD_RESET || data == CMD_STATUS_REPORT || data == CMD_CYCLE_START || data == CMD_FEED_HOLD || data > 0x7F);
}

// Act upon a realtime character
void execute_realtime_command(uint8_t command, uint8_t client) {
    switch (command) {
        case CMD_RESET:
            mc_reset();  // Call motion control reset routine.
            break;
        case CMD_STATUS_REPORT:
            report_realtime_status(client);  // direct call instead of setting flag
            break;
        case CMD_CYCLE_START:
            system_set_exec_state_flag(EXEC_CYCLE_START);  // Set as true
            break;
        case CMD_FEED_HOLD:
            system_set_exec_state_flag(EXEC_FEED_HOLD);  // Set as true
            break;
        case CMD_SAFETY_DOOR: system_set_exec_state_flag(EXEC_SAFETY_DOOR); break;  // Set as true
        case CMD_JOG_CANCEL:
            if (sys.state & STATE_JOG)  // Block all other states from invoking motion cancel.
                system_set_exec_state_flag(EXEC_MOTION_CANCEL);
            break;
#ifdef DEBUG
        case CMD_DEBUG_REPORT: {
            uint8_t sreg = SREG;
            cli();
            bit_true(sys_rt_exec_debug, EXEC_DEBUG_REPORT);
            SREG = sreg;
        } break;
#endif
        case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
        case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
        case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
        case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
        case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
        case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
        case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
        case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
        case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
        case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
        case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
        case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
        case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
        case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
#ifdef COOLANT_FLOOD_PIN
        case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
#endif
#ifdef COOLANT_MIST_PIN
        case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
#endif
    }
}

/*
  Serial.cpp - Header for system level commands and real-time processes
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

#include "Grbl.h"

// Define this to use the Arduino serial (UART) driver instead
// of the one in Uart.cpp, which uses the ESP-IDF UART driver.
// This is for regression testing, and can be removed after
// testing is complete.
// #define REVERT_TO_ARDUINO_SERIAL

portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;

static TaskHandle_t clientCheckTaskHandle = 0;

WebUI::InputBuffer client_buffer[CLIENT_COUNT];  // create a buffer for each client

// Returns the number of bytes available in a client buffer.
uint8_t client_get_rx_buffer_available(uint8_t client) {
#ifdef REVERT_TO_ARDUINO_SERIAL
    return 128 - Serial.available();
#else
    return 128 - Uart0.available();
#endif
    //    return client_buffer[client].availableforwrite();
}

void heapCheckTask(void* pvParameters) {
    static uint32_t heapSize = 0;
    while (true) {
        uint32_t newHeapSize = xPortGetFreeHeapSize();
        if (newHeapSize != heapSize) {
            heapSize = newHeapSize;
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "heap %d", heapSize);
        }
        vTaskDelay(3000 / portTICK_RATE_MS);  // Yield to other tasks

        static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#endif
    }
}

void client_init() {
#ifdef DEBUG_REPORT_HEAP_SIZE
    // For a 2000-word stack, uxTaskGetStackHighWaterMark reports 288 words available
    xTaskCreatePinnedToCore(heapCheckTask, "heapTask", 2000, NULL, 1, NULL, 1);
#endif

#ifdef REVERT_TO_ARDUINO_SERIAL
    Serial.begin(BAUD_RATE, SERIAL_8N1, 3, 1, false);
    client_reset_read_buffer(CLIENT_ALL);
    Serial.write("\r\n");  // create some white space after ESP32 boot info
#else
    Uart0.setPins(1, 3);  // Tx 1, Rx 3 - standard hardware pins
    Uart0.begin(BAUD_RATE, Uart::Data::Bits8, Uart::Stop::Bits1, Uart::Parity::None);

    client_reset_read_buffer(CLIENT_ALL);
    Uart0.write("\r\n");  // create some white space after ESP32 boot info
#endif
    clientCheckTaskHandle = 0;
    // create a task to check for incoming data
    // For a 4096-word stack, uxTaskGetStackHighWaterMark reports 244 words available
    // after WebUI attaches.
    xTaskCreatePinnedToCore(clientCheckTask,    // task
                            "clientCheckTask",  // name for task
                            8192,               // size of task stack
                            NULL,               // parameters
                            1,                  // priority
                            &clientCheckTaskHandle,
                            SUPPORT_TASK_CORE  // must run the task on same core
                                               // core
    );
}

static uint8_t getClientChar(uint8_t* data) {
    int res;
#ifdef REVERT_TO_ARDUINO_SERIAL
    if (client_buffer[CLIENT_SERIAL].availableforwrite() && (res = Serial.read()) != -1) {
#else
    if (client_buffer[CLIENT_SERIAL].availableforwrite() && (res = Uart0.read()) != -1) {
#endif
        *data = res;
        return CLIENT_SERIAL;
    }
    if (WebUI::inputBuffer.available()) {
        *data = WebUI::inputBuffer.read();
        return CLIENT_INPUT;
    }
    //currently is wifi or BT but better to prepare both can be live
#ifdef ENABLE_BLUETOOTH
    if (WebUI::SerialBT.hasClient()) {
        if ((res = WebUI::SerialBT.read()) != -1) {
            *data = res;
            return CLIENT_BT;
        }
    }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_IN)
    if (WebUI::Serial2Socket.available()) {
        *data = WebUI::Serial2Socket.read();
        return CLIENT_WEBUI;
    }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)
    if (WebUI::telnet_server.available()) {
        *data = WebUI::telnet_server.read();
        return CLIENT_TELNET;
    }
#endif
    return CLIENT_ALL;
}

// this task runs and checks for data on all interfaces
// REaltime stuff is acted upon, then characters are added to the appropriate buffer
void clientCheckTask(void* pvParameters) {
    uint8_t            data = 0;
    uint8_t            client;  // who sent the data
    static UBaseType_t uxHighWaterMark = 0;
    while (true) {  // run continuously
        while ((client = getClientChar(&data)) != CLIENT_ALL) {
            // Pick off realtime command characters directly from the serial stream. These characters are
            // not passed into the main buffer, but these set system state flag bits for realtime execution.
            if (is_realtime_command(data)) {
                execute_realtime_command(static_cast<Cmd>(data), client);
            } else {
#if defined(ENABLE_SD_CARD)
                if (get_sd_state(false) < SDState::Busy) {
#endif  //ENABLE_SD_CARD
                    vTaskEnterCritical(&myMutex);
                    client_buffer[client].write(data);
                    vTaskExitCritical(&myMutex);
#if defined(ENABLE_SD_CARD)
                } else {
                    if (data == '\r' || data == '\n') {
                        grbl_sendf(client, "error %d\r\n", Error::AnotherInterfaceBusy);
                        grbl_msg_sendf(client, MsgLevel::Info, "SD card job running");
                    }
                }
#endif  //ENABLE_SD_CARD
            }
        }  // if something available
        WebUI::COMMANDS::handle();
#ifdef ENABLE_WIFI
        WebUI::wifi_config.handle();
#endif
#ifdef ENABLE_BLUETOOTH
        WebUI::bt_config.handle();
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_IN)
        WebUI::Serial2Socket.handle_flush();
#endif
        vTaskDelay(1 / portTICK_RATE_MS);  // Yield to other tasks

        static UBaseType_t uxHighWaterMark = 0;
#ifdef DEBUG_TASK_STACK
        reportTaskStackSize(uxHighWaterMark);
#endif
    }
}

void client_reset_read_buffer(uint8_t client) {
    for (uint8_t client_num = 0; client_num < CLIENT_COUNT; client_num++) {
        if (client == client_num || client == CLIENT_ALL) {
            client_buffer[client_num].begin();
        }
    }
}

// Fetches the first byte in the client read buffer. Called by protocol loop.
int client_read(uint8_t client) {
    vTaskEnterCritical(&myMutex);
    int data = client_buffer[client].read();
    vTaskExitCritical(&myMutex);
    return data;
}

// checks to see if a character is a realtime character
bool is_realtime_command(uint8_t data) {
    if (data >= 0x80) {
        return true;
    }
    auto cmd = static_cast<Cmd>(data);
    return cmd == Cmd::Reset || cmd == Cmd::StatusReport || cmd == Cmd::CycleStart || cmd == Cmd::FeedHold;
}

// Act upon a realtime character
void execute_realtime_command(Cmd command, uint8_t client) {
    switch (command) {
        case Cmd::Reset:
            grbl_msg_sendf(CLIENT_ALL, MsgLevel::Debug, "Cmd::Reset");
            mc_reset();  // Call motion control reset routine.
            break;
        case Cmd::StatusReport:
            report_realtime_status(client);  // direct call instead of setting flag
            break;
        case Cmd::CycleStart:
            sys_rt_exec_state.bit.cycleStart = true;
            break;
        case Cmd::FeedHold:
            sys_rt_exec_state.bit.feedHold = true;
            break;
        case Cmd::SafetyDoor:
            sys_rt_exec_state.bit.safetyDoor = true;
            break;
        case Cmd::JogCancel:
            if (sys.state == State::Jog) {  // Block all other states from invoking motion cancel.
                sys_rt_exec_state.bit.motionCancel = true;
            }
            break;
        case Cmd::DebugReport:
#ifdef DEBUG
            sys_rt_exec_debug = true;
#endif
            break;
        case Cmd::SpindleOvrStop:
            sys_rt_exec_accessory_override.bit.spindleOvrStop = 1;
            break;
        case Cmd::FeedOvrReset:
            sys_rt_f_override = FeedOverride::Default;
            break;
        case Cmd::FeedOvrCoarsePlus:
            sys_rt_f_override += FeedOverride::CoarseIncrement;
            if (sys_rt_f_override > FeedOverride::Max) {
                sys_rt_f_override = FeedOverride::Max;
            }
            break;
        case Cmd::FeedOvrCoarseMinus:
            sys_rt_f_override -= FeedOverride::CoarseIncrement;
            if (sys_rt_f_override < FeedOverride::Min) {
                sys_rt_f_override = FeedOverride::Min;
            }
            break;
        case Cmd::FeedOvrFinePlus:
            sys_rt_f_override += FeedOverride::FineIncrement;
            if (sys_rt_f_override > FeedOverride::Max) {
                sys_rt_f_override = FeedOverride::Max;
            }
            break;
        case Cmd::FeedOvrFineMinus:
            sys_rt_f_override -= FeedOverride::FineIncrement;
            if (sys_rt_f_override < FeedOverride::Min) {
                sys_rt_f_override = FeedOverride::Min;
            }
            break;
        case Cmd::RapidOvrReset:
            sys_rt_r_override = RapidOverride::Default;
            break;
        case Cmd::RapidOvrMedium:
            sys_rt_r_override = RapidOverride::Medium;
            break;
        case Cmd::RapidOvrLow:
            sys_rt_r_override = RapidOverride::Low;
            break;
        case Cmd::RapidOvrExtraLow:
            sys_rt_r_override = RapidOverride::ExtraLow;
            break;
        case Cmd::SpindleOvrReset:
            sys_rt_s_override = SpindleSpeedOverride::Default;
            break;
        case Cmd::SpindleOvrCoarsePlus:
            sys_rt_s_override += SpindleSpeedOverride::CoarseIncrement;
            if (sys_rt_s_override > SpindleSpeedOverride::Max) {
                sys_rt_s_override = SpindleSpeedOverride::Max;
            }
            break;
        case Cmd::SpindleOvrCoarseMinus:
            sys_rt_s_override -= SpindleSpeedOverride::CoarseIncrement;
            if (sys_rt_s_override < SpindleSpeedOverride::Min) {
                sys_rt_s_override = SpindleSpeedOverride::Min;
            }
            break;
        case Cmd::SpindleOvrFinePlus:
            sys_rt_s_override += SpindleSpeedOverride::FineIncrement;
            if (sys_rt_s_override > SpindleSpeedOverride::Max) {
                sys_rt_s_override = SpindleSpeedOverride::Max;
            }
            break;
        case Cmd::SpindleOvrFineMinus:
            sys_rt_s_override -= SpindleSpeedOverride::FineIncrement;
            if (sys_rt_s_override < SpindleSpeedOverride::Min) {
                sys_rt_s_override = SpindleSpeedOverride::Min;
            }
            break;
        case Cmd::CoolantFloodOvrToggle:
            sys_rt_exec_accessory_override.bit.coolantFloodOvrToggle = 1;
            break;
        case Cmd::CoolantMistOvrToggle:
            sys_rt_exec_accessory_override.bit.coolantMistOvrToggle = 1;
            break;
    }
}

void client_write(uint8_t client, const char* text) {
    if (client == CLIENT_INPUT) {
        return;
    }
#ifdef ENABLE_BLUETOOTH
    if (WebUI::SerialBT.hasClient() && (client == CLIENT_BT || client == CLIENT_ALL)) {
        WebUI::SerialBT.print(text);
        //delay(10); // possible fix for dropped characters
    }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_HTTP) && defined(ENABLE_SERIAL2SOCKET_OUT)
    if (client == CLIENT_WEBUI || client == CLIENT_ALL) {
        WebUI::Serial2Socket.write((const uint8_t*)text, strlen(text));
    }
#endif
#if defined(ENABLE_WIFI) && defined(ENABLE_TELNET)
    if (client == CLIENT_TELNET || client == CLIENT_ALL) {
        WebUI::telnet_server.write((const uint8_t*)text, strlen(text));
    }
#endif
    if (client == CLIENT_SERIAL || client == CLIENT_ALL) {
#ifdef REVERT_TO_ARDUINO_SERIAL
        Serial.write(text);
#else
        Uart0.write(text);
#endif
    }
}

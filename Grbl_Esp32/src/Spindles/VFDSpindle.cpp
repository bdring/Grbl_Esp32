/*
    VFDSpindle.cpp

    This is for a VFD based spindles via RS485 Modbus. The details of the
    VFD protocol heavily depend on the VFD in question here. We have some
    implementations, but if yours is not here, the place to start is the
    manual. This VFD class implements the modbus functionality.

    Part of Grbl_ESP32
    2020 -  Bart Dring
    2020 -  Stefan de Bruijn

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

                         WARNING!!!!
    VFDs are very dangerous. They have high voltages and are very powerful
    Remove power before changing bits.

    TODO:
      - We can report spindle_state and rpm better with VFD's that support
        either mode, register RPM or actual RPM.
      - Destructor should break down the task.
      - Move min/max RPM to protected members.

*/
#include "VFDSpindle.h"

#include <freertos/task.h>

// Timing and modbus... The manual states that between communications, we should respect a
// silent interval of 3,5 characters. If we received communications between these times, we
// have to assume that the message is broken. We use a poll rate of 250 ms here, which should
// be plenty: assuming 9600 8N1, that's roughly 250 chars. A message of 2x16 chars with 4x4
// chars buffering is just 40 chars.

const int        VFD_RS485_UART_PORT  = 2;  // hard coded for this port right now
const int        VFD_RS485_BUF_SIZE   = 127;
const int        VFD_RS485_QUEUE_SIZE = 10;                                         // numv\ber of commands that can be queued up.
const int        RESPONSE_WAIT_MILLIS = 1000;                                       // how long to wait for a response in milliseconds
const int        VFD_RS485_POLL_RATE  = 250;                                        // in milliseconds between commands
const TickType_t response_ticks       = RESPONSE_WAIT_MILLIS / portTICK_PERIOD_MS;  // in milliseconds between commands

// OK to change these
// #define them in your machine definition file if you want different values
#ifndef VFD_RS485_ADDR
#    define VFD_RS485_ADDR 0x01
#endif

namespace Spindles {
    Uart          _uart(VFD_RS485_UART_PORT);
    QueueHandle_t VFD::vfd_cmd_queue     = nullptr;
    TaskHandle_t  VFD::vfd_cmdTaskHandle = nullptr;

    VFD::VFD() :
        _txd_pin(
#ifdef VFD_RS485_TXD_PIN
            VFD_RS485_TXD_PIN
#else
            -1
#endif
            ),
        _rxd_pin(
#ifdef VFD_RS485_RXD_PIN
            VFD_RS485_RXD_PIN
#else
            -1
#endif
            ),
        _rts_pin(
#ifdef VFD_RS485_RTS_PIN
            VFD_RS485_RTS_PIN
#else
            -1
#endif
            ),
        _baudrate(
#ifdef VFD_RS485_BAUD_RATE
            VFD_RS485_BAUD_RATE
#else
            9600
#endif
            ),
        _dataBits(Uart::Data::Bits8), _stopBits(Uart::Stop::Bits1), _parity(
#ifdef VFD_RS485_PARITY
                                                                        VFD_RS485_PARITY
#else
                                                                        Uart::Parity::None
#endif
                                                                    ) {
    }

    // The communications task
    void VFD::vfd_cmd_task(void* pvParameters) {
        static bool unresponsive = false;  // to pop off a message once each time it becomes unresponsive
        static int  pollidx      = -1;     // -1 starts the VFD initialization sequence

        VFD*          instance = static_cast<VFD*>(pvParameters);
        ModbusCommand next_cmd;
        uint8_t       rx_message[VFD_RS485_MAX_MSG_SIZE];
        bool          safetyPollingEnabled = instance->safety_polling();

        while (true) {
            response_parser parser = nullptr;

            next_cmd.msg[0] = VFD_RS485_ADDR;  // Always default to this

            // First check if we should ask the VFD for the max RPM value as part of the initialization. We
            // should also query this is max_rpm is 0, because that means a previous initialization failed:
            if ((pollidx < 0 || instance->_max_rpm == 0) && (parser = instance->initialization_sequence(pollidx, next_cmd)) != nullptr) {
                next_cmd.critical = false;
            } else {
                pollidx           = 1;  // Done with initialization. Main sequence.
                next_cmd.critical = false;
            }

            // If we don't have a parser, the queue goes first. During idle, we can grab a parser.
            if (parser == nullptr && xQueueReceive(vfd_cmd_queue, &next_cmd, 0) != pdTRUE) {
                // We poll in a cycle. Note that the switch will fall through unless we encounter a hit.
                // The weakest form here is 'get_status_ok' which should be implemented if the rest fails.
                if (instance->_syncing) {
                    parser = instance->get_current_rpm(next_cmd);
                } else if (safetyPollingEnabled) {
                    switch (pollidx) {
                        case 1:
                            parser = instance->get_current_rpm(next_cmd);
                            if (parser) {
                                pollidx = 2;
                                break;
                            }
                            // fall through intentionally:
                        case 2:
                            parser = instance->get_current_direction(next_cmd);
                            if (parser) {
                                pollidx = 3;
                                break;
                            }
                            // fall through intentionally:
                        case 3:
                        default:
                            parser  = instance->get_status_ok(next_cmd);
                            pollidx = 1;

                            // we could complete this in case parser == nullptr with some ifs, but let's
                            // just keep it easy and wait an iteration.
                            break;
                    }
                }

                // If we have no parser, that means get_status_ok is not implemented (and we have
                // nothing resting in our queue). Let's fall back on a simple continue.
                if (parser == nullptr) {
                    vTaskDelay(VFD_RS485_POLL_RATE / portTICK_PERIOD_MS);
                    continue;  // main while loop
                }
            }

            {
                // Grabbed the command. Add the CRC16 checksum:
                auto crc16 = ModRTU_CRC(next_cmd.msg, next_cmd.tx_length);

                next_cmd.tx_length += 2;
                next_cmd.rx_length += 2;

                // add the calculated Crc to the message
                next_cmd.msg[next_cmd.tx_length - 1] = (crc16 & 0xFF00) >> 8;
                next_cmd.msg[next_cmd.tx_length - 2] = (crc16 & 0xFF);

#ifdef VFD_DEBUG_MODE2
                if (parser == nullptr) {
                    report_hex_msg(next_cmd.msg, "RS485 Tx: ", next_cmd.tx_length);
                }
#endif
            }

            // Assume for the worst, and retry...
            int retry_count = 0;
            for (; retry_count < MAX_RETRIES; ++retry_count) {
                // Flush the UART:
                _uart.flush();

                // Write the data:
                _uart.write(reinterpret_cast<const char*>(next_cmd.msg), next_cmd.tx_length);
                _uart.flushTxTimed(response_ticks);

                // Read the response
                uint16_t read_length  = 0;
                uint16_t current_read = _uart.readBytes(rx_message, next_cmd.rx_length, response_ticks);
                read_length += current_read;

                // Apparently some Huanyang report modbus errors in the correct way, and the rest not. Sigh.
                // Let's just check for the condition, and truncate the first byte.
                if (read_length > 0 && VFD_RS485_ADDR != 0 && rx_message[0] == 0) {
                    memmove(rx_message + 1, rx_message, read_length - 1);
                }

                while (read_length < next_cmd.rx_length && current_read > 0) {
                    // Try to read more; we're not there yet...
                    current_read = _uart.readBytes(rx_message + read_length, next_cmd.rx_length - read_length, response_ticks);
                    read_length += current_read;
                }
                if (current_read < 0) {
                    read_length = 0;
                }

                // Generate crc16 for the response:
                auto crc16response = ModRTU_CRC(rx_message, next_cmd.rx_length - 2);

                if (read_length == next_cmd.rx_length &&                             // check expected length
                    rx_message[0] == VFD_RS485_ADDR &&                               // check address
                    rx_message[read_length - 1] == (crc16response & 0xFF00) >> 8 &&  // check CRC byte 1
                    rx_message[read_length - 2] == (crc16response & 0xFF)) {         // check CRC byte 1

                    // Success
                    unresponsive = false;
                    retry_count  = MAX_RETRIES + 1;  // stop retry'ing

                    // Should we parse this?
                    if (parser != nullptr) {
                        if (parser(rx_message, instance)) {
                            // If we're initializing, move to the next initialization command:
                            if (pollidx < 0) {
                                --pollidx;
                            }
                        } else {
                            // If we were initializing, move back to where we started.
#ifdef VFD_DEBUG_MODE
                            // Parsing failed
                            report_hex_msg(next_cmd.msg, "RS485 Tx: ", next_cmd.tx_length);
                            report_hex_msg(rx_message, "RS485 Rx: ", read_length);
#endif

                            // Not succesful! Now what?
                            unresponsive = true;
                            pollidx      = -1;  // Re-initializing the VFD seems like a plan
                            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Spindle RS485 did not give a satisfying response");
                        }
                    }
                } else {
#ifdef VFD_DEBUG_MODE
                    report_hex_msg(next_cmd.msg, "RS485 Tx: ", next_cmd.tx_length);
                    report_hex_msg(rx_message, "RS485 Rx: ", read_length);

                    if (read_length != 0) {
                        if (rx_message[0] != VFD_RS485_ADDR) {
                            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RS485 received message from other modbus device");
                        } else if (read_length != next_cmd.rx_length) {
                            grbl_msg_sendf(CLIENT_SERIAL,
                                           MsgLevel::Info,
                                           "RS485 received message of unexpected length; expected %d, got %d",
                                           int(next_cmd.rx_length),
                                           int(read_length));
                        } else {
                            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RS485 CRC check failed");
                        }
                    } else {
                        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RS485 No response");
                    }
#endif

                    // Wait a bit before we retry. Set the delay to poll-rate. Not sure
                    // if we should use a different value...
                    vTaskDelay(VFD_RS485_POLL_RATE / portTICK_PERIOD_MS);

                    static UBaseType_t uxHighWaterMark = 0;
                    reportTaskStackSize(uxHighWaterMark);
                }
            }

            if (retry_count == MAX_RETRIES) {
                if (!unresponsive) {
                    grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "Spindle RS485 Unresponsive %d", next_cmd.rx_length);
                    unresponsive = true;
                    pollidx      = -1;
                }
                if (next_cmd.critical) {
                    grbl_msg_sendf(CLIENT_ALL, MsgLevel::Error, "Critical Spindle RS485 Unresponsive");
                    mc_reset();
                    sys_rt_exec_alarm = ExecAlarm::SpindleControl;
                }
            }

            vTaskDelay(VFD_RS485_POLL_RATE / portTICK_PERIOD_MS);
        }
    }

    // ================== Class methods ==================================

    void VFD::init() {
        vfd_ok    = false;  // initialize
        _sync_rpm = 0;
        _syncing  = false;

        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Initializing RS485 VFD spindle");

        // fail if required items are not defined
        if (!get_pins_and_settings()) {
            vfd_ok = false;
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RS485 VFD spindle errors");
            return;
        }

        // this allows us to init() again later.
        // If you change certain settings, init() gets called agian
        // uart_driver_delete(VFD_RS485_UART_PORT);

        if (_uart.setPins(_txd_pin, _rxd_pin, _rts_pin)) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RS485 VFD uart pin config failed");
            return;
        }

        _uart.begin(_baudrate, _dataBits, _stopBits, _parity);

        if (_uart.setHalfDuplex()) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "RS485 VFD uart set half duplex failed");
            return;
        }

        // We have to initialize the constants before starting the task:
        is_reversable = true;  // these VFDs are always reversable
        use_delays    = true;
        vfd_ok        = true;

        // Initially we initialize this to 0; over time, we might poll better information from the VFD.
        _current_rpm   = 0;
        _current_state = SpindleState::Disable;

        // Initialization is complete, so now it's okay to run the queue task:
        if (!_task_running) {  // init can happen many times, we only want to start one task
            vfd_cmd_queue = xQueueCreate(VFD_RS485_QUEUE_SIZE, sizeof(ModbusCommand));
            xTaskCreatePinnedToCore(vfd_cmd_task,         // task
                                    "vfd_cmdTaskHandle",  // name for task
                                    2048,                 // size of task stack
                                    this,                 // parameters
                                    1,                    // priority
                                    &vfd_cmdTaskHandle,
                                    SUPPORT_TASK_CORE  // core
            );
            _task_running = true;
        }

        config_message();
    }

    // Checks for all the required pin definitions
    // It returns a message for each missing pin
    // Returns true if all pins are defined.
    bool VFD::get_pins_and_settings() {
        bool pins_settings_ok = true;

        if (_txd_pin == -1) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Undefined VFD_RS485_TXD_PIN");
            pins_settings_ok = false;
        }

        if (_rxd_pin == -1) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Undefined VFD_RS485_RXD_PIN");
            pins_settings_ok = false;
        }

        if (_rts_pin == -1) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Undefined VFD_RS485_RTS_PIN");
            pins_settings_ok = false;
        }

        if (laser_mode->get()) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "VFD spindle disabled in laser mode. Set $GCode/LaserMode=Off and restart");
            pins_settings_ok = false;
        }

        // Use config for initial RPM values:
        _min_rpm = rpm_min->get();
        _max_rpm = rpm_max->get();

        _spinup_delay   = spindle_delay_spinup->get() * 1000.0;
        _spindown_delay = spindle_delay_spindown->get() * 1000.0;

        return pins_settings_ok;
    }

    void VFD::config_message() {
        grbl_msg_sendf(CLIENT_SERIAL,
                       MsgLevel::Info,
                       "VFD RS485  Tx:%s Rx:%s RTS:%s",
                       pinName(_txd_pin).c_str(),
                       pinName(_rxd_pin).c_str(),
                       pinName(_rts_pin).c_str());
    }

    void VFD::set_state(SpindleState state, uint32_t rpm) {
        if (sys.abort) {
            return;  // Block during abort.
        }

        bool shouldWait = state != _current_state || state != SpindleState::Disable;
        bool critical   = (sys.state == State::Cycle || state != SpindleState::Disable);

        int32_t delayMillis = 1000;

        if (_current_state != state) {  // already at the desired state. This function gets called a lot.
            set_mode(state, critical);  // critical if we are in a job

            if (rpm != 0 && (rpm < _min_rpm || rpm > _max_rpm)) {
                grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "VFD: Requested speed %d outside range:(%d,%d)", rpm, _min_rpm, _max_rpm);
            }

            set_rpm(rpm);

            if (state == SpindleState::Disable) {
                sys.spindle_speed = 0;
                delayMillis       = _spindown_delay;
                rpm               = 0;

            } else {
                delayMillis = _spinup_delay;
            }

            if (_current_state != state && !supports_actual_rpm()) {
                delay(delayMillis);
            }
        } else {
            if (_current_rpm != rpm) {
                if (rpm != 0 && (rpm < _min_rpm || rpm > _max_rpm)) {
                    grbl_msg_sendf(CLIENT_ALL, MsgLevel::Info, "VFD: Requested speed %d outside range:(%d,%d)", rpm, _min_rpm, _max_rpm);
                }

                set_rpm(rpm);

                if (rpm > _current_rpm) {
                    delayMillis = _spinup_delay;
                } else {
                    delayMillis = _spindown_delay;
                }
            }
        }

        if (shouldWait) {
            if (supports_actual_rpm()) {
                _syncing = true;

                // Allow 2.5% difference from what we asked for. Should be fine.
                uint32_t drpm = (_max_rpm - _min_rpm) / 40;
                if (drpm < 100) {
                    drpm = 100;
                }  // Just a sanity check

                auto minRpmAllowed = _current_rpm > drpm ? (_current_rpm - drpm) : 0;
                auto maxRpmAllowed = _current_rpm + drpm;

                int       unchanged = 0;
                const int limit     = 20;  // 20 * 0.5s = 10 sec
                auto      last      = _sync_rpm;

                while ((_sync_rpm < minRpmAllowed || _sync_rpm > maxRpmAllowed) && unchanged < limit) {
#ifdef VFD_DEBUG_MODE
                    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Syncing RPM. Requested %d, current %d", int(rpm), int(_sync_rpm));
#endif
                    if (!mc_dwell(500)) {
                        // Something happened while we were dwelling, like a safety door.
                        unchanged = limit;
                        last      = _sync_rpm;
                        break;
                    }

                    if (_sync_rpm == last) {
                        ++unchanged;
                    } else {
                        unchanged = 0;
                    }
                    last = _sync_rpm;
                }

                if (unchanged == limit) {
                    grbl_msg_sendf(CLIENT_ALL,
                                   MsgLevel::Error,
                                   "Critical Spindle RS485 did not reach speed %d. Reported speed is %d rpm.",
                                   rpm,
                                   _sync_rpm);
                    mc_reset();
                    sys_rt_exec_alarm = ExecAlarm::SpindleControl;
                }

                _syncing = false;
            } else {
                delay(delayMillis);
            }
        }

        _current_state         = state;  // store locally for faster get_state()
        sys.report_ovr_counter = 0;      // Set to report change immediately

        return;
    }

    bool VFD::set_mode(SpindleState mode, bool critical) {
        if (!vfd_ok) {
            return false;
        }

        ModbusCommand mode_cmd;
        mode_cmd.msg[0] = VFD_RS485_ADDR;

        direction_command(mode, mode_cmd);

        if (mode == SpindleState::Disable) {
            if (!xQueueReset(vfd_cmd_queue)) {
                grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "VFD spindle off, queue could not be reset");
            }
        }

        mode_cmd.critical = critical;
        _current_state    = mode;

        if (xQueueSend(vfd_cmd_queue, &mode_cmd, 0) != pdTRUE) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "VFD Queue Full");
        }

        return true;
    }

    uint32_t VFD::set_rpm(uint32_t rpm) {
        if (!vfd_ok) {
            return 0;
        }

        // apply override
        rpm = rpm * sys.spindle_speed_ovr / 100;  // Scale by spindle speed override value (uint8_t percent)

        if (rpm != 0 && (rpm < _min_rpm || rpm > _max_rpm)) {
            // NOTE: Don't add a info message here; this method is called from the stepper_pulse_func ISR method, so
            // emitting debug information could crash the ESP32.

            rpm = constrain(rpm, _min_rpm, _max_rpm);
        }

        // apply limits
        // if ((_min_rpm >= _max_rpm) || (rpm >= _max_rpm)) {
        //     rpm = _max_rpm;
        // } else if (rpm != 0 && rpm <= _min_rpm) {
        //     rpm = _min_rpm;
        // }

        sys.spindle_speed = rpm;

        if (rpm == _current_rpm) {  // prevent setting same RPM twice
            return rpm;
        }

#ifdef VFD_DEBUG_MODE2
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Setting spindle speed to %d rpm (%d, %d)", int(rpm), int(_min_rpm), int(_max_rpm));
#endif

        _current_rpm = rpm;

        // TODO add the speed modifiers override, linearization, etc.

        ModbusCommand rpm_cmd;
        rpm_cmd.msg[0] = VFD_RS485_ADDR;

        set_speed_command(rpm, rpm_cmd);

        // Sometimes sync_rpm is retained between different set_speed_command's. We don't want that - we want 
        // spindle sync to kick in after we set the speed. This forces that.
        _sync_rpm = UINT32_MAX;

        rpm_cmd.critical = (rpm == 0);

        if (xQueueSend(vfd_cmd_queue, &rpm_cmd, 0) != pdTRUE) {
            grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "VFD Queue Full");
        }

        return rpm;
    }

    void VFD::stop() {
#ifdef VFD_DEBUG_MODE
        grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Debug, "VFD::stop()");
#endif
        set_mode(SpindleState::Disable, true);
    }

    // state is cached rather than read right now to prevent delays
    SpindleState VFD::get_state() { return _current_state; }

    // Calculate the CRC on all of the byte except the last 2
    // It then added the CRC to those last 2 bytes
    // full_msg_len This is the length of the message including the 2 crc bytes
    // Source: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
    uint16_t VFD::ModRTU_CRC(uint8_t* buf, int msg_len) {
        uint16_t crc = 0xFFFF;
        for (int pos = 0; pos < msg_len; pos++) {
            crc ^= uint16_t(buf[pos]);  // XOR byte into least sig. byte of crc.

            for (int i = 8; i != 0; i--) {  // Loop over each bit
                if ((crc & 0x0001) != 0) {  // If the LSB is set
                    crc >>= 1;              // Shift right and XOR 0xA001
                    crc ^= 0xA001;
                } else {        // Else LSB is not set
                    crc >>= 1;  // Just shift right
                }
            }
        }

        return crc;
    }
}

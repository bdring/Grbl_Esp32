/*
	oled_basic.cpp
	Part of Grbl_ESP32

	copyright (c) 2018 -	Bart Dring This file was modified for use on the ESP32
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

	--------------------------------------------------------------

    This is a minimal implentation of a display as a test project.
    It is designed to be used with a machine that has no easily accessible serial connection
    It shows basic status and connection information.

    When in alarm mode it will show the current Wifi/BT paramaters and status
    Most machines will start in alarm mode (needs homing)
    If the machine is running a job from SD it will show the progress
    In other modes it will show state and 3 axis DROs
    Thats All! 

    Library Infor:
        https://github.com/ThingPulse/esp8266-oled-ssd1306

    Install to PlatformIO with this typed at the terminal
        platformio lib install 562

    Add this to your machine definition file
        #define DISPLAY_CODE_FILENAME "Custom/oled_basic.cpp"
*/

// Include the correct display library

#include "SSD1306Wire.h"
#include "../src/WebUI/WebSettings.h"

#ifndef OLED_ADDRESS
#    define OLED_ADDRESS 0x3c
#endif

#ifndef OLED_SDA
#    define OLED_SDA GPIO_NUM_14
#endif

#ifndef OLED_SCL
#    define OLED_SCL GPIO_NUM_13
#endif

#ifndef OLED_GEOMETRY
#    define OLED_GEOMETRY GEOMETRY_128_64
#endif

SSD1306Wire display(OLED_ADDRESS, OLED_SDA, OLED_SCL, OLED_GEOMETRY);

static TaskHandle_t displayUpdateTaskHandle = 0;

// This displays the status of the ESP32 Radios...BT, WiFi, etc
void displayRadioInfo() {
    String radio_addr   = "";
    String radio_name   = "";
    String radio_status = "";

#ifdef ENABLE_BLUETOOTH
    if (WebUI::wifi_radio_mode->get() == ESP_BT) {
        radio_name = String("BT: ") + WebUI::bt_name->get();
    }
#endif
#ifdef ENABLE_WIFI
    if ((WiFi.getMode() == WIFI_MODE_STA) || (WiFi.getMode() == WIFI_MODE_APSTA)) {
        radio_name = "STA: " + WiFi.SSID();
        radio_addr = WiFi.localIP().toString();
    } else if ((WiFi.getMode() == WIFI_MODE_AP) || (WiFi.getMode() == WIFI_MODE_APSTA)) {
        radio_name = String("AP:") + WebUI::wifi_ap_ssid->get();
        radio_addr = WiFi.softAPIP().toString();
    }
#endif

#ifdef WIFI_OR_BLUETOOTH
    if (WebUI::wifi_radio_mode->get() == ESP_RADIO_OFF) {
        radio_name = "Radio Mode: None";
    }
#else
    radio_name = "Radio Mode:Disabled";
#endif

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    if (sys.state == State::Alarm) {  // print below Alarm:
        display.drawString(0, 18, radio_name);
        display.drawString(0, 30, radio_addr);

    } else {  // print next to status
        if (WebUI::wifi_radio_mode->get() == ESP_BT) {
            display.drawString(55, 2, radio_name);
        } else {
            display.drawString(55, 2, radio_addr);
        }
    }
}
// Here changes begin  Here changes begin Here changes begin Here changes begin Here changes begin

void draw_checkbox(int16_t x, int16_t y, int16_t width, int16_t height, bool checked) {
    if (checked)
        display.fillRect(x, y, width, height);  // If log.0
    else
        display.drawRect(x, y, width, height);  // If log.1
}

void displayDRO() {
    uint8_t oled_y_pos;
    float   print_position[MAX_N_AXIS];
    //float   wco[MAX_N_AXIS];

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    char axisVal[20];

    display.drawString(80, 14, "L");  // Limit switch

    auto        n_axis         = number_axis->get();
    AxisMask    lim_pin_state  = limits_get_state();
    ControlPins ctrl_pin_state = system_control_get_state();
    bool        prb_pin_state  = probe_get_state();

    if (bit_istrue(status_mask->get(), RtStatus::Position)) {
        calc_mpos(print_position);
    } else {
        calc_wpos(print_position);
    }

    for (uint8_t axis = X_AXIS; axis < n_axis; axis++) {
        oled_y_pos = 24 + (axis * 10);

        String axis_letter = String(report_get_axis_letter(axis));
        axis_letter += ":";
        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.drawString(0, oled_y_pos, axis_letter);  // String('X') + ":");

        display.setTextAlignment(TEXT_ALIGN_RIGHT);
        snprintf(axisVal, 20 - 1, "%.3f", print_position[axis]);
        display.drawString(60, oled_y_pos, axisVal);

        if (limitsSwitchDefined(axis, 0)) {  // olny draw the box if a switch has been defined
            draw_checkbox(80, 27 + (axis * 10), 7, 7, bit_istrue(lim_pin_state, bit(axis)));
        }
    }

    oled_y_pos = 14;

    if (PROBE_PIN != UNDEFINED_PIN) {
        display.drawString(110, oled_y_pos, "P");
        draw_checkbox(120, oled_y_pos + 3, 7, 7, prb_pin_state);
        oled_y_pos += 10;
    }

#ifdef CONTROL_FEED_HOLD_PIN
    display.drawString(110, oled_y_pos, "H");
    draw_checkbox(120, oled_y_pos + 3, 7, 7, ctrl_pin_state.bit.feedHold);
    oled_y_pos += 10;
#endif

#ifdef CONTROL_CYCLE_START_PIN
    display.drawString(110, oled_y_pos, "S");
    draw_checkbox(120, oled_y_pos + 3, 7, 7, ctrl_pin_state.bit.cycleStart);
    oled_y_pos += 10;
#endif

#ifdef CONTROL_RESET_PIN
    display.drawString(110, oled_y_pos, "R");
    draw_checkbox(120, oled_y_pos + 3, 7, 7, ctrl_pin_state.bit.reset);
    oled_y_pos += 10;
#endif

#ifdef CONTROL_SAFETY_DOOR_PIN
    display.drawString(110, oled_y_pos, "D");
    draw_checkbox(120, oled_y_pos + 3, 7, 7, ctrl_pin_state.bit.safetyDoor);
#endif
}

void displayUpdate(void* pvParameters) {
    TickType_t       xLastWakeTime;
    const TickType_t xDisplayFrequency = 100;                  // in ticks (typically ms)
    xLastWakeTime                      = xTaskGetTickCount();  // Initialise the xLastWakeTime variable with the current time.

    vTaskDelay(2500);
    uint16_t sd_file_ticker = 0;

    display.init();
    display.flipScreenVertically();

    while (true) {
        display.clear();

        String state_string = "";

        display.setTextAlignment(TEXT_ALIGN_LEFT);
        display.setFont(ArialMT_Plain_16);
        display.drawString(0, 0, report_state_text());

        if (get_sd_state(false) == SDState::BusyPrinting) {
            display.clear();
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.setFont(ArialMT_Plain_10);
            state_string = "SD File";
            for (int i = 0; i < sd_file_ticker % 10; i++) {
                state_string += ".";
            }
            sd_file_ticker++;
            display.drawString(63, 0, state_string);

            char path[50];
            sd_get_current_filename(path);
            display.drawString(63, 12, path);

            int progress = sd_report_perc_complete();
            // draw the progress bar
            display.drawProgressBar(0, 45, 120, 10, progress);

            // draw the percentage as String
            display.setFont(ArialMT_Plain_10);
            display.setTextAlignment(TEXT_ALIGN_CENTER);
            display.drawString(64, 25, String(progress) + "%");

        } else if (sys.state == State::Alarm) {
            displayRadioInfo();
        } else {
            displayDRO();
            displayRadioInfo();
        }

        display.display();

        vTaskDelayUntil(&xLastWakeTime, xDisplayFrequency);
    }
}

void display_init() {
    // Initialising the UI will init the display too.
    grbl_msg_sendf(CLIENT_SERIAL, MsgLevel::Info, "Init Basic OLED SDA:%s SCL:%s", pinName(OLED_SDA), pinName(OLED_SCL));
    display.init();

    display.flipScreenVertically();

    display.clear();

    display.setTextAlignment(TEXT_ALIGN_CENTER);
    display.setFont(ArialMT_Plain_10);

    String mach_name = MACHINE_NAME;
    // remove characters from the end until the string fits
    while (display.getStringWidth(mach_name) > 128) {
        mach_name = mach_name.substring(0, mach_name.length() - 1);
    }
    display.drawString(63, 0, mach_name);

    display.display();

    xTaskCreatePinnedToCore(displayUpdate,        // task
                            "displayUpdateTask",  // name for task
                            4096,                 // size of task stack
                            NULL,                 // parameters
                            1,                    // priority
                            &displayUpdateTaskHandle,
                            CONFIG_ARDUINO_RUNNING_CORE  // must run the task on same core
                                                         // core
    );
}

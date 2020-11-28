/*
  BTConfig.cpp -  Bluetooth functions class

  Copyright (c) 2014 Luc Lebosse. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "../Grbl.h"

#ifdef ENABLE_BLUETOOTH
#    include <BluetoothSerial.h>
#    include "BTConfig.h"

namespace WebUI {
    BTConfig        bt_config;
    BluetoothSerial SerialBT;
#    ifdef __cplusplus
    extern "C" {
#    endif
    const uint8_t* esp_bt_dev_get_address(void);
#    ifdef __cplusplus
    }
#    endif

    String BTConfig::_btname   = "";
    String BTConfig::_btclient = "";

    BTConfig::BTConfig() {}

    static void my_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
        switch (event) {
            case ESP_SPP_SRV_OPEN_EVT: {  //Server connection open
                char str[18];
                str[17]       = '\0';
                uint8_t* addr = param->srv_open.rem_bda;
                sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
                BTConfig::_btclient = str;
                grbl_sendf(CLIENT_ALL, "[MSG:BT Connected with %s]\r\n", str);
            } break;
            case ESP_SPP_CLOSE_EVT:  //Client connection closed
                grbl_send(CLIENT_ALL, "[MSG:BT Disconnected]\r\n");
                BTConfig::_btclient = "";
                break;
            default:
                break;
        }
    }

    const char* BTConfig::info() {
        static String result;
        String        tmp;
        result = "[MSG:";
        if (Is_BT_on()) {
            result += "Mode=BT:Name=";
            result += _btname;
            result += "(";
            result += device_address();
            result += "):Status=";
            if (SerialBT.hasClient()) {
                result += "Connected with " + _btclient;
            } else {
                result += "Not connected";
            }
        } else {
            result += "No BT";
        }
        result += "]\r\n";
        return result.c_str();
    }
    /**
     * Check if BlueTooth string is valid
     */

    bool BTConfig::isBTnameValid(const char* hostname) {
        //limited size
        if (!hostname) {
            return true;
        }
        char c;
        // length is checked automatically by string setting
        //only letter and digit
        for (int i = 0; i < strlen(hostname); i++) {
            c = hostname[i];
            if (!(isdigit(c) || isalpha(c) || c == '_')) {
                return false;
            }
        }
        return true;
    }

    const char* BTConfig::device_address() {
        const uint8_t* point = esp_bt_dev_get_address();
        static char    str[18];
        str[17] = '\0';
        sprintf(
            str, "%02X:%02X:%02X:%02X:%02X:%02X", (int)point[0], (int)point[1], (int)point[2], (int)point[3], (int)point[4], (int)point[5]);
        return str;
    }

    /**
     * begin WiFi setup
     */
    void BTConfig::begin() {
        //stop active services
        end();
        _btname = bt_name->get();
        if (wifi_radio_mode->get() == ESP_BT) {
            if (!SerialBT.begin(_btname)) {
                report_status_message(Error::BtFailBegin, CLIENT_ALL);
            } else {
                SerialBT.register_callback(&my_spp_cb);
                grbl_sendf(CLIENT_ALL, "[MSG:BT Started with %s]\r\n", _btname.c_str());
            }
        } else {
            end();
        }
    }

    /**
     * End WiFi
     */
    void BTConfig::end() { SerialBT.end(); }

    /**
     * Reset ESP
     */
    void BTConfig::reset_settings() {
        bt_name->setDefault();
        wifi_radio_mode->setDefault();
        grbl_send(CLIENT_ALL, "[MSG:BT reset done]\r\n");
    }

    /**
     * Check if BT is on and working
     */
    bool BTConfig::Is_BT_on() { return btStarted(); }

    /**
     * Handle not critical actions that must be done in sync environement
     */
    void BTConfig::handle() {
        //If needed
        COMMANDS::wait(0);
    }

    BTConfig::~BTConfig() { end(); }
}
#endif  // ENABLE_BLUETOOTH

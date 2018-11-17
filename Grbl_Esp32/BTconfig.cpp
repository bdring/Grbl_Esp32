/*
  BTconfig.cpp -  Bluetooth functions class

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

#ifdef ARDUINO_ARCH_ESP32

#include "config.h"

#ifdef ENABLE_BLUETOOTH
#include <Preferences.h>
#include "BluetoothSerial.h"
#include "BTconfig.h"
#include "commands.h"
#include "report.h"

BTConfig bt_config;
BluetoothSerial SerialBT;
#ifdef __cplusplus
extern "C" {
#endif
const uint8_t *esp_bt_dev_get_address(void);
#ifdef __cplusplus
}
#endif

String BTConfig::_btname = "";

BTConfig::BTConfig(){
}
    
BTConfig::~BTConfig(){
    end();
}

const char *BTConfig::info(){
    static String result;
    String tmp;
    result = "[MSG:";
    if(Is_BT_on()) {
        result += "Mode=BT:Name=";
        result += _btname;
        result += "(";
        result += device_address();
        result += "):Status=";
        if (SerialBT.hasClient()){
            result += "Connected";
        } else result += "Not connected";
    } 
    else result+="No BT";
    result+= "]\r\n";
    return result.c_str();
}
/**
 * Check if BlueTooth string is valid
 */

bool BTConfig::isBTnameValid (const char * hostname){
    //limited size
    char c;
    if (strlen (hostname) > MAX_BTNAME_LENGTH || strlen (hostname) < MIN_BTNAME_LENGTH) {
        return false;
    }
    //only letter and digit
    for (int i = 0; i < strlen (hostname); i++) {
        c = hostname[i];
        if (! (isdigit (c) || isalpha (c) || c == '_') ) {
            return false;
        }
    }
    return true;
}

const char* BTConfig::device_address(){
    const uint8_t* point = esp_bt_dev_get_address();
    static char str[18];
    str[17]='\0';
    sprintf(str, "%02X:%02X:%02X:%02X:%02X:%02X", (int)point[0], (int)point[1], (int)point[2], (int)point[3], (int)point[4], (int)point[5]);
    return str;
}

/**
 * begin WiFi setup
 */
void BTConfig::begin() {
    Preferences prefs;
    //stop active services
    end();
    prefs.begin(NAMESPACE, true);
    //Get hostname
    String defV = DEFAULT_BT_NAME;
    _btname = prefs.getString(BT_NAME_ENTRY, defV);
    int8_t wifiMode = prefs.getChar(ESP_RADIO_MODE, DEFAULT_RADIO_MODE);
    prefs.end();
    if (wifiMode == ESP_BT) {
        if (!SerialBT.begin(_btname))
            {		
            report_status_message(STATUS_BT_FAIL_BEGIN, CLIENT_ALL);		
        } else grbl_sendf(CLIENT_ALL,"[MSG:BT Started with %s]\r\n", _btname.c_str());
        
   }else end();
     
}

/**
 * End WiFi 
 */
void BTConfig::end() {
    SerialBT.end();
}

/**
 * Reset ESP
 */
void BTConfig::reset_settings(){
    Preferences prefs;
    prefs.begin(NAMESPACE, false);
    String sval;
    int8_t bbuf;
    bool error = false;
    sval = DEFAULT_BT_NAME;
    if (prefs.putString(BT_NAME_ENTRY, sval) == 0){
        error = true;
    }
    bbuf = DEFAULT_RADIO_MODE;
    if (prefs.putChar(ESP_RADIO_MODE, bbuf) ==0 ) {
        error = true;
    }  
    prefs.end();
    if (error) {
        grbl_send(CLIENT_ALL,"[MSG:BT reset error]\r\n");
    } else {
        grbl_send(CLIENT_ALL,"[MSG:BT reset done]\r\n");
    }
}

/**
 * Check if BT is on and working
 */
bool BTConfig::Is_BT_on(){
    return btStarted();
}

/**
 * BT events:
 * BT_EVENT_DISCONNECTED < New connection if none previously
 * BT_EVENT_CONNECTED < Disconnection
 */
void BTConfig::BTEvent(uint8_t event){
    switch (event){
        case BT_EVENT_DISCONNECTED:
            grbl_send(CLIENT_ALL,"[MSG:BT Disconnected]\r\n");
            break;
        case BT_EVENT_CONNECTED:
            grbl_send(CLIENT_ALL,"[MSG:BT Connected]\r\n");
            break;
        default:
        grbl_send(CLIENT_ALL,"[MSG:BT Unknow BT event]\r\n");
    }
}

/**
 * Handle not critical actions that must be done in sync environement
 */
void BTConfig::handle() {
    static bool connected = false;
    //generate event as BT seems not having event, or I am wrong ?
    //currently only connection event but can add more later
    if (Is_BT_on()){
        if (connected != SerialBT.hasClient()) {
            connected = SerialBT.hasClient();
            if (connected) BTEvent(BT_EVENT_CONNECTED);
            else BTEvent(BT_EVENT_DISCONNECTED);
            
        }
    }
}


#endif // ENABLE_BLUETOOTH

#endif // ARDUINO_ARCH_ESP32

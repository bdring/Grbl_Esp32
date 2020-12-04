#pragma once

/*
  BTConfig.h -  Bluetooth functions class

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
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#    error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

//defaults values
const char* const DEFAULT_BT_NAME = "btgrblesp";

#include <BluetoothSerial.h>

namespace WebUI {
    extern BluetoothSerial SerialBT;

    class BTConfig {
        //boundaries
    public:
        static const int MAX_BTNAME_LENGTH = 32;
        static const int MIN_BTNAME_LENGTH = 1;

        BTConfig();

        static const char* info();
        static bool        isBTnameValid(const char* hostname);
        static String      BTname() { return _btname; }
        static const char* device_address();
        static void        begin();
        static void        end();
        static void        handle();
        static void        reset_settings();
        static bool        Is_BT_on();
        static String      _btclient;

        ~BTConfig();

    private:
        static String _btname;
    };

    extern BTConfig bt_config;
}

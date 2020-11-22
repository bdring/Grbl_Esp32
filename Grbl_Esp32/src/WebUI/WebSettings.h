#pragma once

/*
  WebSettings.h - Definitions for WebUI-related settings.

  Copyright (c) 2020 Mitch Bradley
  Copyright (c) 2014 Luc Lebosse. All rights reserved.

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

namespace WebUI {
    extern StringSetting* wifi_sta_ssid;
    extern StringSetting* wifi_sta_password;

#ifdef ENABLE_WIFI
    extern EnumSetting*   wifi_sta_mode;
    extern IPaddrSetting* wifi_sta_ip;
    extern IPaddrSetting* wifi_sta_gateway;
    extern IPaddrSetting* wifi_sta_netmask;

    extern StringSetting* wifi_ap_ssid;
    extern StringSetting* wifi_ap_password;

    extern IPaddrSetting* wifi_ap_ip;

    extern IntSetting* wifi_ap_channel;

    extern StringSetting* wifi_hostname;
    extern EnumSetting*   http_enable;
    extern IntSetting*    http_port;
    extern EnumSetting*   telnet_enable;
    extern IntSetting*    telnet_port;
#endif

#ifdef WIFI_OR_BLUETOOTH
    extern EnumSetting* wifi_radio_mode;
#endif

#ifdef ENABLE_BLUETOOTH
    extern StringSetting* bt_name;
#endif

#ifdef ENABLE_AUTHENTICATION
    extern StringSetting* user_password;
    extern StringSetting* admin_password;
#endif

#ifdef ENABLE_NOTIFICATIONS
    extern EnumSetting*   notification_type;
    extern StringSetting* notification_t1;
    extern StringSetting* notification_t2;
    extern StringSetting* notification_ts;
#endif
}

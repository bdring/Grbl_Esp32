#pragma once

#include <IPAddress.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

#define WiFiEvent_t int
#define WIFI_OFF false

class WiFiStuff {
public:
    void persistent(bool on) {};
    void disconnect(bool on) {}
    void enableSTA(bool on) {}
    void enableAP(bool on) {}
    void mode(bool mode) {}
};

extern WiFiStuff WiFi;

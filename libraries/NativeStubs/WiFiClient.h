#pragma once
#include <IPAddress.h>

class WiFiClient {
public:
    WiFiClient() {}
    WiFiClient(int fd) {}
    ~WiFiClient() {}

    int       available() { return 0; }
    int       read(uint8_t* buf, size_t size) { return -1; }
    void      stop() {}
    size_t    write(const uint8_t* buf, size_t size) { return 0; }
    uint8_t   connected() { return 0; }
    IPAddress remoteIP() const { return IPAddress(uint32_t(0)); }

    operator bool() { return connected(); }

    // WiFiClient& operator=(const WiFiClient& other);
    //     bool operator==(const bool value) { return bool() == value; }
    // bool operator!=(const bool value) { return bool() != value; }
    //    bool        operator==(const WiFiClient&);
    // bool operator!=(const WiFiClient& rhs) { return !this->operator==(rhs); }
};

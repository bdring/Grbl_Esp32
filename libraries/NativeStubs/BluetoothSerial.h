#pragma once
#include "Arduino.h"
#include "Stream.h"

inline bool btStarted() {
    return false;
}

typedef int esp_spp_cb_event_t;

typedef struct {
    struct {
        uint8_t* rem_bda;
    } srv_open;
} esp_spp_cb_param_t;
enum { ESP_SPP_SRV_OPEN_EVT, ESP_SPP_SRV_CLOSE_EVT, ESP_SPP_CLOSE_EVT };

class BluetoothSerial : public Stream {
public:
    BluetoothSerial(void)  = default;
    ~BluetoothSerial(void) = default;

    bool begin(String localName = String(), bool isMaster = false) { return false; }
    void register_callback(void (*callback)(esp_spp_cb_event_t event, esp_spp_cb_param_t* param)) {}
    bool hasClient(void) { return false; }
    void end(void) {}

    size_t write(uint8_t) override { return 0; }
    int    available() override { return 0; }
    int    read() override { return -1; }
    int    peek() override { return -1; }
};

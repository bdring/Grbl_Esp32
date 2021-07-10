#pragma once
#include <stdint.h>
#include "binary.h"

// The native compiler might not support __attribute__ ((weak))
#define WEAK_FUNC

inline int temperatureRead() {
    return 27;
}

inline long long esp_timer_get_time() {
    return 0LL;
}

typedef int esp_err_t;

extern "C" {
inline esp_err_t esp_task_wdt_reset() {}
}

const esp_err_t ESP_ERR_NVS_INVALID_HANDLE = 3;
const esp_err_t ESP_ERR_NVS_INVALID_NAME   = 2;
const esp_err_t ESP_ERR_NVS_INVALID_LENGTH = 1;
const esp_err_t ESP_OK                     = 0;
inline uint32_t getApbFrequency() {
    return 80000000;
}

// Timer stuff used by Stepper.cpp
typedef struct {
    int  divider;
    int  counter_dir;
    int  counter_en;
    int  alarm_en;
    int  intr_type;
    bool auto_reload;
} timer_config_t;

const int           TIMER_COUNT_UP   = 0;
const int           TIMER_PAUSE      = 0;
const int           TIMER_ALARM_EN   = 0;
const int           TIMER_INTR_LEVEL = 0;
typedef int         timer_group_t;
typedef int         timer_idx_t;
const timer_group_t TIMER_GROUP_0 = 0;
const timer_idx_t   TIMER_0       = 0;

inline void timer_set_counter_value(timer_group_t group, timer_idx_t idx, uint64_t ticks) {}
inline void timer_set_alarm_value(timer_group_t group, timer_idx_t idx, uint64_t ticks) {}
inline void timer_init(timer_group_t group, timer_idx_t idx, timer_config_t* conf) {}
inline void timer_enable_intr(timer_group_t group, timer_idx_t idx) {}
inline void timer_start(timer_group_t group, timer_idx_t idx) {}
inline void timer_pause(timer_group_t group, timer_idx_t idx) {}
inline void timer_isr_register(timer_group_t group, timer_idx_t idx, void (*handler)(void*), void* arg, int a, void* arg1) {}

#define IRAM_ATTR

// Most of the EspClass stuff is used by information reports,
// except for restart()
class EspClass {
public:
    const char*     getSdkVersion() { return "native"; }
    inline void     restart() {}
    inline uint64_t getEfuseMac() { return 0ULL; }
    inline uint32_t getCpuFreqMHz() { return 240000000; }
    inline uint32_t getFreeHeap() { return 30000; }
    inline uint32_t getFlashChipSize() { return 0x400000; }
};
extern EspClass ESP;

typedef int uart_port_t;

#define NO_TASKS

// Stub versions of FreeRTOS functions
typedef int TickType_t;

const int            portMUX_INITIALIZER_UNLOCKED = 0;
const int            portMAX_DELAY                = 0;
const int            portTICK_PERIOD_MS           = 1;
const int            portTICK_RATE_MS             = 1;
typedef int          portMUX_TYPE;
typedef int          BaseType_t;
typedef unsigned int UBaseType_t;
typedef void*        xQueueHandle;
typedef void*        QueueHandle_t;
typedef void*        TaskHandle_t;
typedef int          TickType_t;
const BaseType_t     pdTRUE  = 1;
const BaseType_t     pdFALSE = 0;
const BaseType_t     pdPASS  = 0;
inline void          vTaskDelay(TickType_t ticks) {}
inline BaseType_t    xQueueReceive(QueueHandle_t xQueue, void* pvBuffer, TickType_t xTicksToWait) {}
inline xQueueHandle  xQueueCreate(int n, int len) {}
inline BaseType_t    xQueueReset(QueueHandle_t queue) {
    return pdPASS;
}
inline BaseType_t xQueueSend(QueueHandle_t queue, void* item, TickType_t ticks) {
    return pdTRUE;
}
inline TaskHandle_t xTaskCreate(void (*task)(void*), const char* name, int stacksize, void* arg0, int pri, TaskHandle_t* th) {}
inline TaskHandle_t xTaskCreatePinnedToCore(
    void (*task)(void*), const char* name, int stacksize, void* arg0, int pri, TaskHandle_t* th, int core) {}
inline int xTaskGetTickCount() {
    return 0;
}
inline void vTaskDelayUntil(TickType_t* pxPreviousWakeTime, const TickType_t xTimeIncrement) {}
inline int  xPortGetFreeHeapSize() {
    return 0;
}
inline void vTaskEnterCritical(int* mutex) {}
inline void vTaskExitCritical(int* mutex) {}

inline void attachInterrupt(int pin, void (*isr_limit_switches)(), int change) {}
inline void detachInterrupt(int pin) {}

inline void NOP() {}

// dacWrite stub - used by DacSpindle
inline void dacWrite(int _output_pin, uint8_t duty) {}

inline void ledcSetup(int channel_num, double freq, int bits) {}
inline void ledcWrite(int channel_num, int duty) {}
inline void ledcAttachPin(int pwm_pin, int channel_num) {}

// NVS used by Settings
const int        NVS_READWRITE = 0;
inline esp_err_t nvs_open(const char* s, int mode, int* handle) {
    return 0;
}
inline esp_err_t nvs_get_i32(int _handle, const char* _keyName, int* value) {
    return -1;
}
inline esp_err_t nvs_set_i32(int _handle, const char* _keyName, int value) {
    return -1;
}
inline esp_err_t nvs_get_i8(int _handle, const char* _keyName, signed char* value) {
    return -1;
}
inline esp_err_t nvs_set_i8(int _handle, const char* _keyName, int value) {
    return -1;
}
inline esp_err_t nvs_get_str(int _handle, const char* _keyName, void* p, unsigned int* len) {
    return -1;
}
inline esp_err_t nvs_set_str(int _handle, const char* _keyName, const char* value) {
    return -1;
}
inline void      nvs_erase_key(int _handle, const char* key) {}
inline esp_err_t nvs_get_blob(int handle, const char* _keyName, void* currentValue, size_t* len) {
    return -1;
}
inline esp_err_t nvs_set_blob(int handle, const char* _keyName, void* currentValue, size_t len) {
    return -1;
}

// Unlike the ESP32 Arduino framework, EpoxyDuino does not have contrain() and map()

// Templates don't work because of float/double promotion
#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    const long dividend = out_max - out_min;
    const long divisor  = in_max - in_min;
    const long delta    = x - in_min;
    if (divisor == 0) {
        return -1;  //AVR returns -1, SAM returns 0
    }
    return (delta * dividend + (divisor / 2)) / divisor + out_min;
}

// The IPAddress implementation in EpoxyDuino does not have toString()
// so we include the implementation from the esp32 arduino framework
#include <Stream.h>
#include <WString.h>
#include <Printable.h>
class IPAddress : public Printable {
private:
    union {
        uint8_t  bytes[4];  // IPv4 address
        uint32_t dword;
    } _address;

    // Access the raw byte array containing the address.  Because this returns a pointer
    // to the internal structure rather than a copy of the address this function should only
    // be used when you know that the usage of the returned uint8_t* will be transient and not
    // stored.
    uint8_t* raw_address() { return _address.bytes; }

public:
    // Constructors
    IPAddress();
    IPAddress(uint8_t first_octet, uint8_t second_octet, uint8_t third_octet, uint8_t fourth_octet);
    IPAddress(uint32_t address);
    IPAddress(const uint8_t* address);
    virtual ~IPAddress() {}

    bool fromString(const char* address);
    bool fromString(const String& address) { return fromString(address.c_str()); }

    // Overloaded cast operator to allow IPAddress objects to be used where a pointer
    // to a four-byte uint8_t array is expected
         operator uint32_t() const { return _address.dword; }
    bool operator==(const IPAddress& addr) const { return _address.dword == addr._address.dword; }
    bool operator==(const uint8_t* addr) const;

    // Overloaded index operator to allow getting and setting individual octets of the address
    uint8_t  operator[](int index) const { return _address.bytes[index]; }
    uint8_t& operator[](int index) { return _address.bytes[index]; }

    // Overloaded copy operators to allow initialisation of IPAddress objects from other types
    IPAddress& operator=(const uint8_t* address);
    IPAddress& operator=(uint32_t address);

    virtual size_t printTo(Print& p) const;
    String         toString() const;

#if 0
    friend class EthernetClass;
    friend class UDP;
    friend class Client;
    friend class Server;
    friend class DhcpClass;
    friend class DNSClient;
#endif
};

#define FILE_READ "r"

namespace fs {
    enum SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2 };

    class File : public Stream {
    public:
        File() {}

        size_t write(uint8_t) override;
        size_t write(const uint8_t* buf, size_t size) override;
        int    available() override;
        int    read() override;
        int    peek() override;
        void   flush() override;
        size_t read(uint8_t* buf, size_t size);
        size_t readBytes(char* buffer, size_t length) { return read((uint8_t*)buffer, length); }

        bool        seek(uint32_t pos, SeekMode mode);
        bool        seek(uint32_t pos) { return seek(pos, SeekSet); }
        size_t      position() const;
        size_t      size() const;
        void        close();
                    operator bool() const;
        time_t      getLastWrite();
        const char* name() const;

        bool isDirectory(void);
        File openNextFile(const char* mode = FILE_READ);
        void rewindDirectory(void);
    };
    class FS {
    public:
        File open(const char* path, const char* mode = FILE_READ);
        File open(const String& path, const char* mode = FILE_READ);

        bool exists(const char* path);
        bool exists(const String& path);

        bool remove(const char* path);
        bool remove(const String& path);

        bool rename(const char* pathFrom, const char* pathTo);
        bool rename(const String& pathFrom, const String& pathTo);

        bool mkdir(const char* path);
        bool mkdir(const String& path);

        bool rmdir(const char* path);
        bool rmdir(const String& path);
    };

    class SPIFFSFS : public fs::FS {
    public:
        SPIFFSFS();
        ~SPIFFSFS();
        bool begin(bool formatOnFail = false, const char* basePath = "/spiffs", uint8_t maxOpenFiles = 10, const char* partitionLabel = NULL);
        bool   format();
        size_t totalBytes();
        size_t usedBytes();
        void   end();
    };
}
using fs::File;
using fs::FS;
using fs::SeekCur;
using fs::SeekEnd;
using fs::SeekMode;
using fs::SeekSet;

extern fs::SPIFFSFS SPIFFS;

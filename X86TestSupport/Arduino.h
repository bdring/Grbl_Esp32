#pragma once

#include <cstdint>

#include "esp_err.h"

class SystemRestartException {};

#define IRAM_ATTR

// From Arduino.h:

// Interrupt Modes
#define RISING 0x01
#define FALLING 0x02
#define CHANGE 0x03
// #define ONLOW     0x04 --> Weird. Same as falling / rising?
// #define ONHIGH    0x05
// #define ONLOW_WE  0x0C --> not sure about these
// #define ONHIGH_WE 0x0D

// From esp32-hal-gpio.h:

#define LOW 0x0
#define HIGH 0x1

// GPIO FUNCTIONS
#define INPUT 0x01
#define OUTPUT 0x02
#define PULLUP 0x04
#define INPUT_PULLUP 0x05
#define PULLDOWN 0x08
#define INPUT_PULLDOWN 0x09
// #define OPEN_DRAIN        0x10
// #define OUTPUT_OPEN_DRAIN 0x12

void attachInterrupt(uint8_t pin, void (*)(void), int mode);
void attachInterruptArg(uint8_t pin, void (*)(void*), void* arg, int mode);
void detachInterrupt(uint8_t pin);

extern "C" int  __digitalRead(uint8_t pin);
extern "C" void __pinMode(uint8_t pin, uint8_t mode);
extern "C" void __digitalWrite(uint8_t pin, uint8_t val);

void delay(int ms);

// Get time in microseconds since boot.
int64_t esp_timer_get_time();

// this_thread::yield?
#define NOP()                                                                                                                              \
    do {                                                                                                                                   \
    } while (0);

typedef enum {
    GPIO_NUM_0 = 0,   /*!< GPIO0, input and output */
    GPIO_NUM_1 = 1,   /*!< GPIO1, input and output */
    GPIO_NUM_2 = 2,   /*!< GPIO2, input and output
                             @note There are more enumerations like that
                             up to GPIO39, excluding GPIO20, GPIO24 and GPIO28..31.
                             They are not shown here to reduce redundant information.
                             @note GPIO34..39 are input mode only. */
                      /** @cond */
    GPIO_NUM_3  = 3,  /*!< GPIO3, input and output */
    GPIO_NUM_4  = 4,  /*!< GPIO4, input and output */
    GPIO_NUM_5  = 5,  /*!< GPIO5, input and output */
    GPIO_NUM_6  = 6,  /*!< GPIO6, input and output */
    GPIO_NUM_7  = 7,  /*!< GPIO7, input and output */
    GPIO_NUM_8  = 8,  /*!< GPIO8, input and output */
    GPIO_NUM_9  = 9,  /*!< GPIO9, input and output */
    GPIO_NUM_10 = 10, /*!< GPIO10, input and output */
    GPIO_NUM_11 = 11, /*!< GPIO11, input and output */
    GPIO_NUM_12 = 12, /*!< GPIO12, input and output */
    GPIO_NUM_13 = 13, /*!< GPIO13, input and output */
    GPIO_NUM_14 = 14, /*!< GPIO14, input and output */
    GPIO_NUM_15 = 15, /*!< GPIO15, input and output */
    GPIO_NUM_16 = 16, /*!< GPIO16, input and output */
    GPIO_NUM_17 = 17, /*!< GPIO17, input and output */
    GPIO_NUM_18 = 18, /*!< GPIO18, input and output */
    GPIO_NUM_19 = 19, /*!< GPIO19, input and output */

    GPIO_NUM_21 = 21, /*!< GPIO21, input and output */
    GPIO_NUM_22 = 22, /*!< GPIO22, input and output */
    GPIO_NUM_23 = 23, /*!< GPIO23, input and output */

    GPIO_NUM_25 = 25, /*!< GPIO25, input and output */
    GPIO_NUM_26 = 26, /*!< GPIO26, input and output */
    GPIO_NUM_27 = 27, /*!< GPIO27, input and output */

    GPIO_NUM_32  = 32, /*!< GPIO32, input and output */
    GPIO_NUM_33  = 33, /*!< GPIO33, input and output */
    GPIO_NUM_34  = 34, /*!< GPIO34, input mode only */
    GPIO_NUM_35  = 35, /*!< GPIO35, input mode only */
    GPIO_NUM_36  = 36, /*!< GPIO36, input mode only */
    GPIO_NUM_37  = 37, /*!< GPIO37, input mode only */
    GPIO_NUM_38  = 38, /*!< GPIO38, input mode only */
    GPIO_NUM_39  = 39, /*!< GPIO39, input mode only */
    GPIO_NUM_MAX = 40,
    /** @endcond */
} gpio_num_t;

#define constrain(amt, low, high) ((amt) < (low) ? (low) : ((amt) > (high) ? (high) : (amt)))

// PWM:
uint32_t getApbFrequency();  // In Hz
double   ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits);
void     ledcWrite(uint8_t channel, uint32_t duty);
void     ledcAttachPin(uint8_t pin, uint8_t channel);
void     ledcDetachPin(uint8_t pin);

// Timer:

struct hw_timer_s;
typedef struct hw_timer_s hw_timer_t;

void timerAlarmEnable(hw_timer_t* timer);
void timerAlarmDisable(hw_timer_t* timer);
void timerWrite(hw_timer_t* timer, uint64_t val);
void timerAlarmWrite(hw_timer_t* timer, uint64_t interruptAt, bool autoreload);

hw_timer_t* timerBegin(uint8_t timer, uint16_t divider, bool countUp);
void        timerEnd(hw_timer_t* timer);

void timerAttachInterrupt(hw_timer_t* timer, void (*fn)(void), bool edge);
void timerDetachInterrupt(hw_timer_t* timer);

// Figure this out:
extern "C" {
esp_err_t esp_task_wdt_reset(void);
}

unsigned long micros();
unsigned long millis();
void          delay(uint32_t);
void          delayMicroseconds(uint32_t us);

// ESP...

typedef enum {
    ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
    ESP_RST_POWERON,    //!< Reset due to power-on event
    ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
    ESP_RST_SW,         //!< Software reset via esp_restart
    ESP_RST_PANIC,      //!< Software reset due to exception/panic
    ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
    ESP_RST_WDT,        //!< Reset due to other watchdogs
    ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
    ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
    ESP_RST_SDIO,       //!< Reset over SDIO
} esp_reset_reason_t;

esp_reset_reason_t esp_reset_reason(void);

struct EspClass {
    uint64_t    getEfuseMac();
    uint32_t    getCpuFreqMHz();
    const char* getSdkVersion();
    uint32_t    getFreeHeap();
    uint32_t    getFlashChipSize();

    void restart();
};
extern EspClass ESP;

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int temperatureRead(void);

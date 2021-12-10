#pragma once
#include <ctype.h>
#include <binary.h>

inline bool isPrintable(int c) {
    return (isprint(c) == 0 ? false : true);
}

#define IRAM_ATTR

// The native compiler might not support __attribute__ ((weak))
#define WEAK_FUNC

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

#define M_PI 3.1415926536

#define GPIO_NUM_0 0
#define GPIO_NUM_1 1
#define GPIO_NUM_2 2
#define GPIO_NUM_3 3
#define GPIO_NUM_4 4
#define GPIO_NUM_5 5
#define GPIO_NUM_6 6
#define GPIO_NUM_7 7
#define GPIO_NUM_8 8
#define GPIO_NUM_9 9
#define GPIO_NUM_10 10
#define GPIO_NUM_11 11
#define GPIO_NUM_12 12
#define GPIO_NUM_13 13
#define GPIO_NUM_14 14
#define GPIO_NUM_15 15
#define GPIO_NUM_16 16
#define GPIO_NUM_17 17
#define GPIO_NUM_18 18
#define GPIO_NUM_19 19
#define GPIO_NUM_20 20
#define GPIO_NUM_21 21
#define GPIO_NUM_22 22
#define GPIO_NUM_23 23
#define GPIO_NUM_24 24
#define GPIO_NUM_25 25
#define GPIO_NUM_26 26
#define GPIO_NUM_27 27
#define GPIO_NUM_28 28
#define GPIO_NUM_29 29
#define GPIO_NUM_30 30
#define GPIO_NUM_31 31
#define GPIO_NUM_32 32
#define GPIO_NUM_33 33
#define GPIO_NUM_34 34
#define GPIO_NUM_35 35
#define GPIO_NUM_36 36
#define GPIO_NUM_37 37
#define GPIO_NUM_38 38
#define GPIO_NUM_39 39

#if 0
#    define INPUT 0x0
#    define OUTPUT 0x1
#    define INPUT_PULLUP 0x2

#    define CHANGE 0x03

// Define pins used by I2C and SPI.
static const uint8_t SS   = 1;
static const uint8_t MOSI = 2;
static const uint8_t MISO = 3;
static const uint8_t SCK  = 4;
static const uint8_t SDA  = 5;
static const uint8_t SCL  = 6;
#endif

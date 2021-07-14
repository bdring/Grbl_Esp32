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

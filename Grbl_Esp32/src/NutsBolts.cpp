/*
  NutsBolts.cpp - Shared functions
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

	2018 -	Bart Dring This file was modifed for use on the ESP32
					CPU. Do not use this with Grbl for atMega328P

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

#include "Grbl.h"
#include <cstring>

const int MAX_INT_DIGITS = 8;  // Maximum number of digits in int32 (and float)

// Extracts a floating point value from a string. The following code is based loosely on
// the avr-libc strtod() function by Michael Stumpf and Dmitry Xmelkov and many freely
// available conversion method examples, but has been highly optimized for Grbl. For known
// CNC applications, the typical decimal value is expected to be in the range of E0 to E-4.
// Scientific notation is officially not supported by g-code, and the 'E' character may
// be a g-code word on some CNC systems. So, 'E' notation will not be recognized.
// NOTE: Thanks to Radu-Eosif Mihailescu for identifying the issues with using strtod().
uint8_t read_float(const char* line, uint8_t* char_counter, float* float_ptr) {
    const char*   ptr = line + *char_counter;
    unsigned char c;
    // Grab first character and increment pointer. No spaces assumed in line.
    c = *ptr++;
    // Capture initial positive/minus character
    bool isnegative = false;
    if (c == '-') {
        isnegative = true;
        c          = *ptr++;
    } else if (c == '+') {
        c = *ptr++;
    }

    // Extract number into fast integer. Track decimal in terms of exponent value.
    uint32_t intval    = 0;
    int8_t   exp       = 0;
    uint8_t  ndigit    = 0;
    bool     isdecimal = false;
    while (1) {
        c -= '0';
        if (c <= 9) {
            ndigit++;
            if (ndigit <= MAX_INT_DIGITS) {
                if (isdecimal) {
                    exp--;
                }
                intval = intval * 10 + c;
            } else {
                if (!(isdecimal)) {
                    exp++;  // Drop overflow digits
                }
            }
        } else if (c == (('.' - '0') & 0xff) && !(isdecimal)) {
            isdecimal = true;
        } else {
            break;
        }
        c = *ptr++;
    }
    // Return if no digits have been read.
    if (!ndigit) {
        return false;
    }

    // Convert integer into floating point.
    float fval;
    fval = (float)intval;
    // Apply decimal. Should perform no more than two floating point multiplications for the
    // expected range of E0 to E-4.
    if (fval != 0) {
        while (exp <= -2) {
            fval *= 0.01;
            exp += 2;
        }
        if (exp < 0) {
            fval *= 0.1;
        } else if (exp > 0) {
            do {
                fval *= 10.0;
            } while (--exp > 0);
        }
    }
    // Assign floating point value with correct sign.
    if (isnegative) {
        *float_ptr = -fval;
    } else {
        *float_ptr = fval;
    }
    *char_counter = ptr - line - 1;  // Set char_counter to next statement
    return true;
}

void delay_ms(uint16_t ms) {
    delay(ms);
}

// Non-blocking delay function used for general operation and suspend features.
bool delay_msec(int32_t milliseconds, DwellMode mode) {
    // Note: i must be signed, because of the 'i-- > 0' check below.
    int32_t i         = milliseconds / DWELL_TIME_STEP;
    int32_t remainder = i < 0 ? 0 : (milliseconds - DWELL_TIME_STEP * i);

    while (i-- > 0) {
        if (sys.abort) {
            return false;
        }
        if (mode == DwellMode::Dwell) {
            protocol_execute_realtime();
        } else {  // DwellMode::SysSuspend
            // Execute rt_system() only to avoid nesting suspend loops.
            protocol_exec_rt_system();
            if (sys.suspend.bit.restartRetract) {
                return false;  // Bail, if safety door reopens.
            }
        }
        delay(DWELL_TIME_STEP);  // Delay DWELL_TIME_STEP increment
    }
    delay(remainder);
    return true;
}

// Simple hypotenuse computation function.
float hypot_f(float x, float y) {
    return sqrt(x * x + y * y);
}

float convert_delta_vector_to_unit_vector(float* vector) {
    uint8_t idx;
    float   magnitude = 0.0;
    auto    n_axis    = number_axis->get();
    for (idx = 0; idx < n_axis; idx++) {
        if (vector[idx] != 0.0) {
            magnitude += vector[idx] * vector[idx];
        }
    }
    magnitude           = sqrt(magnitude);
    float inv_magnitude = 1.0 / magnitude;
    for (idx = 0; idx < n_axis; idx++) {
        vector[idx] *= inv_magnitude;
    }
    return magnitude;
}

float limit_acceleration_by_axis_maximum(float* unit_vec) {
    uint8_t idx;
    float   limit_value = SOME_LARGE_VALUE;
    auto    n_axis      = number_axis->get();
    for (idx = 0; idx < n_axis; idx++) {
        if (unit_vec[idx] != 0) {  // Avoid divide by zero.
            limit_value = MIN(limit_value, fabs(axis_settings[idx]->acceleration->get() / unit_vec[idx]));
        }
    }
    // The acceleration setting is stored and displayed in units of mm/sec^2,
    // but used in units of mm/min^2.  It suffices to perform the conversion once on
    // exit, since the limit computation above is independent of units - it simply
    // finds the smallest value.
    return limit_value * SEC_PER_MIN_SQ;
}

float limit_rate_by_axis_maximum(float* unit_vec) {
    uint8_t idx;
    float   limit_value = SOME_LARGE_VALUE;
    auto    n_axis      = number_axis->get();
    for (idx = 0; idx < n_axis; idx++) {
        if (unit_vec[idx] != 0) {  // Avoid divide by zero.
            limit_value = MIN(limit_value, fabs(axis_settings[idx]->max_rate->get() / unit_vec[idx]));
        }
    }
    return limit_value;
}

float map_float(float x, float in_min, float in_max, float out_min, float out_max) {  // DrawBot_Badge
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint32_t map_uint32_t(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float constrain_float(float in, float min, float max) {  // DrawBot_Badge
    if (in < min) {
        return min;
    }
    if (in > max) {
        return max;
    }
    return in;
}

long mapConstrain(long x, long in_min, long in_max, long out_min, long out_max) {
    x = constrain(x, in_min, in_max);
    return map(x, in_min, in_max, out_min, out_max);
}

float mapConstrain(float x, float in_min, float in_max, float out_min, float out_max) {
    x = constrain_float(x, in_min, in_max);
    return map_float(x, in_min, in_max, out_min, out_max);
}

bool char_is_numeric(char value) {
    return value >= '0' && value <= '9';
}

char* trim(char* str) {
    char* end;
    // Trim leading space
    while (::isspace((unsigned char)*str)) {
        str++;
    }
    if (*str == 0) {  // All spaces?
        return str;
    }
    // Trim trailing space
    end = str + ::strlen(str) - 1;
    while (end > str && ::isspace((unsigned char)*end)) {
        end--;
    }
    // Write new null terminator character
    end[1] = '\0';
    return str;
}

// Returns the number of set number of set bits
// Would return 3 for 01100010
int numberOfSetBits(uint32_t i) {
    return __builtin_popcount(i);
}

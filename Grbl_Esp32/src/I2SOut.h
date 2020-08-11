#pragma once

/*
    i2s_out.h
    Part of Grbl_ESP32
    Header for basic GPIO expander using the ESP32 I2S peripheral
    2020    - Michiyasu Odaki
    Grbl_ESP32 is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Grbl is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Grbl_ESP32.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// It should be included at the outset to know the machine configuration.
#include "config.h"

// If USE_I2S_OUT_STREAM is defined
// but the prerequisite USE_I2S_OUT is not defined,
// it is forced to be defined.
#ifdef USE_I2S_OUT_STREAM
#    ifndef USE_I2S_OUT
#        define USE_I2S_OUT
#    endif
#endif

#ifdef USE_I2S_OUT
#    include <stdint.h>

/* Assert */
#    if defined(I2S_OUT_NUM_BITS)
#        if (I2S_OUT_NUM_BITS != 16) && (I2S_OUT_NUM_BITS != 32)
#            error "I2S_OUT_NUM_BITS should be 16 or 32"
#        endif
#    else
#        define I2S_OUT_NUM_BITS 32
#    endif

#    define I2SO(n) (I2S_OUT_PIN_BASE + n)

/* 16-bit mode: 1000000 usec / ((160000000 Hz) / 10 / 2) x 16 bit/pulse x 2(stereo) = 4 usec/pulse */
/* 32-bit mode: 1000000 usec / ((160000000 Hz) /  5 / 2) x 32 bit/pulse x 2(stereo) = 4 usec/pulse */
#    define I2S_OUT_USEC_PER_PULSE 4

#    define I2S_OUT_DMABUF_COUNT 5  /* number of DMA buffers to store data */
#    define I2S_OUT_DMABUF_LEN 2000 /* maximum size in bytes (4092 is DMA's limit) */

#    define I2S_OUT_DELAY_DMABUF_MS (I2S_OUT_DMABUF_LEN / sizeof(uint32_t) * I2S_OUT_USEC_PER_PULSE / 1000)
#    define I2S_OUT_DELAY_MS (I2S_OUT_DELAY_DMABUF_MS * (I2S_OUT_DMABUF_COUNT + 1))

typedef void (*i2s_out_pulse_func_t)(void);

typedef struct {
    /*
        I2S bitstream (32-bits): Transfers from MSB(bit31) to LSB(bit0) in sequence
        ------------------time line------------------------>
             Left Channel                    Right Channel
        ws   ________________________________~~~~...
        bck  _~_~_~_~_~_~_~_~_~_~_~_~_~_~_~_~_~_~...
        data vutsrqponmlkjihgfedcba9876543210
             XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
                                             ^
                                Latches the X bits when ws is switched to High
        If I2S_OUT_PIN_BASE is set to 128,
        bit0:Expanded GPIO 128, 1: Expanded GPIO 129, ..., v: Expanded GPIO 159
    */
    uint8_t              ws_pin;
    uint8_t              bck_pin;
    uint8_t              data_pin;
    i2s_out_pulse_func_t pulse_func;
    uint32_t             pulse_period;  // aka step rate.
    uint32_t             init_val;
} i2s_out_init_t;

/*
  Initialize I2S out by parameters.
  return -1 ... already initialized
*/
int i2s_out_init(i2s_out_init_t& init_param);

/*
  Initialize I2S out by default parameters.
    i2s_out_init_t default_param = {
        .ws_pin = I2S_OUT_WS,
        .bck_pin = I2S_OUT_BCK,
        .data_pin = I2S_OUT_DATA,
        .pulse_func = NULL,
        .pulse_period = I2S_OUT_USEC_PER_PULSE,
        .init_val = I2S_OUT_INIT_VAL,
    };
  return -1 ... already initialized
*/
int i2s_out_init();

/*
  Get a bit state from the internal pin state var.
  pin: expanded pin No. (0..31)
*/
uint8_t i2s_out_state(uint8_t pin);

/*
   Set a bit in the internal pin state var. (not written electrically)
   pin: expanded pin No. (0..31)
   val: bit value(0 or not 0)
*/
void i2s_out_write(uint8_t pin, uint8_t val);

/*
    Set current pin state to the I2S bitstream buffer
    (This call will generate a future I2S_OUT_USEC_PER_PULSE μs x N bitstream)
    num: Number of samples to be generated
         The number of samples is limited to (20 / I2S_OUT_USEC_PER_PULSE).
    return: number of puhsed samples
            0 .. no space for push
 */
uint32_t i2s_out_push_sample(uint32_t num);

/*
   Set pulser mode to passtrough
   After this function is called,
   the callback function to generate the pulse data
   will not be called.
 */
int i2s_out_set_passthrough();

/*
   Set pulser mode to stepping
   After this function is called,
   the callback function to generate stepping pulse data
   will be called.
 */
int i2s_out_set_stepping();

/*
  Dynamically delay until the Shift Register Pin changes
  according to the current I2S processing state and mode.
 */
void i2s_out_delay();

/*
   Set the pulse callback period in ISR ticks.
   (same value of the timer period for the ISR)
 */
int i2s_out_set_pulse_period(uint64_t period);

/*
   Register a callback function to generate pulse data
 */
int i2s_out_set_pulse_callback(i2s_out_pulse_func_t func);

/*
   Reset i2s I/O expander
   - Stop ISR/DMA
   - Clear DMA buffer with the current expanded GPIO bits
   - Retart ISR/DMA
 */
int i2s_out_reset();

#endif

/*
   Reference: "ESP32 Technical Reference Manual" by Espressif Systems
     https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
 */

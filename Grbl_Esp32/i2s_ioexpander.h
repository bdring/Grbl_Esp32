/*
    i2s_ioexpander.h
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
#ifndef i2s_ioexpander_h
#define i2s_ioexpander_h

#include "config.h" // I2S_IOEXP_PIN_BASE

#ifdef USE_I2S_IOEXPANDER
#include <stdint.h>

/* Assert */
#if defined(I2S_IOEXP_NUM_BITS)
  #if (I2S_IOEXP_NUM_BITS != 16) && (I2S_IOEXP_NUM_BITS != 32)
    #error "I2S_IOEXP_NUM_BITS should be 16 or 32"
  #endif
#else
  #define I2S_IOEXP_NUM_BITS 32
#endif

#define GPIO_NUM_I2S_IOEXP_0  (I2S_IOEXP_PIN_BASE + 0)
#define GPIO_NUM_I2S_IOEXP_1  (I2S_IOEXP_PIN_BASE + 1)
#define GPIO_NUM_I2S_IOEXP_2  (I2S_IOEXP_PIN_BASE + 2)
#define GPIO_NUM_I2S_IOEXP_3  (I2S_IOEXP_PIN_BASE + 3)
#define GPIO_NUM_I2S_IOEXP_4  (I2S_IOEXP_PIN_BASE + 4)
#define GPIO_NUM_I2S_IOEXP_5  (I2S_IOEXP_PIN_BASE + 5)
#define GPIO_NUM_I2S_IOEXP_6  (I2S_IOEXP_PIN_BASE + 6)
#define GPIO_NUM_I2S_IOEXP_7  (I2S_IOEXP_PIN_BASE + 7)
#define GPIO_NUM_I2S_IOEXP_8  (I2S_IOEXP_PIN_BASE + 8)
#define GPIO_NUM_I2S_IOEXP_9  (I2S_IOEXP_PIN_BASE + 9)
#define GPIO_NUM_I2S_IOEXP_10  (I2S_IOEXP_PIN_BASE + 10)
#define GPIO_NUM_I2S_IOEXP_11  (I2S_IOEXP_PIN_BASE + 11)
#define GPIO_NUM_I2S_IOEXP_12  (I2S_IOEXP_PIN_BASE + 12)
#define GPIO_NUM_I2S_IOEXP_13  (I2S_IOEXP_PIN_BASE + 13)
#define GPIO_NUM_I2S_IOEXP_14  (I2S_IOEXP_PIN_BASE + 14)
#define GPIO_NUM_I2S_IOEXP_15  (I2S_IOEXP_PIN_BASE + 15)

#if I2S_IOEXP_NUM_BITS > 16
#define GPIO_NUM_I2S_IOEXP_16  (I2S_IOEXP_PIN_BASE + 16)
#define GPIO_NUM_I2S_IOEXP_17  (I2S_IOEXP_PIN_BASE + 17)
#define GPIO_NUM_I2S_IOEXP_18  (I2S_IOEXP_PIN_BASE + 18)
#define GPIO_NUM_I2S_IOEXP_19  (I2S_IOEXP_PIN_BASE + 19)
#define GPIO_NUM_I2S_IOEXP_20  (I2S_IOEXP_PIN_BASE + 20)
#define GPIO_NUM_I2S_IOEXP_21  (I2S_IOEXP_PIN_BASE + 21)
#define GPIO_NUM_I2S_IOEXP_22  (I2S_IOEXP_PIN_BASE + 22)
#define GPIO_NUM_I2S_IOEXP_23  (I2S_IOEXP_PIN_BASE + 23)
#define GPIO_NUM_I2S_IOEXP_24  (I2S_IOEXP_PIN_BASE + 24)
#define GPIO_NUM_I2S_IOEXP_25  (I2S_IOEXP_PIN_BASE + 25)
#define GPIO_NUM_I2S_IOEXP_26  (I2S_IOEXP_PIN_BASE + 26)
#define GPIO_NUM_I2S_IOEXP_27  (I2S_IOEXP_PIN_BASE + 27)
#define GPIO_NUM_I2S_IOEXP_28  (I2S_IOEXP_PIN_BASE + 28)
#define GPIO_NUM_I2S_IOEXP_29  (I2S_IOEXP_PIN_BASE + 29)
#define GPIO_NUM_I2S_IOEXP_30  (I2S_IOEXP_PIN_BASE + 30)
#define GPIO_NUM_I2S_IOEXP_31  (I2S_IOEXP_PIN_BASE + 31)
#endif

/* 1000000 usec / ((160000000 Hz) / 5 / 2) x 32 bit/pulse x 2(stereo) = 4 usec/pulse */
#define I2S_IOEXP_USEC_PER_PULSE 4

#define I2S_IOEXP_DMABUF_COUNT 5     /* number of DMA buffers to store data */
#define I2S_IOEXP_DMABUF_LEN   2000  /* maximum size in bytes (4092 is DMA's limit) */

#define I2S_IOEXP_DELAY_MS    (I2S_IOEXP_DMABUF_LEN / sizeof(uint32_t) * (I2S_IOEXP_DMABUF_COUNT + 1) * I2S_IOEXP_USEC_PER_PULSE / 1000)

#define IS_I2S_IOEXP_PIN(IO) (((IO) >= I2S_IOEXP_PIN_BASE) && ((IO) < I2S_IOEXP_PIN_BASE + I2S_IOEXP_NUM_BITS))
#define I2S_IOEXP_PIN_INDEX(IO) ((IO) - I2S_IOEXP_PIN_BASE)

typedef void (*i2s_ioexpander_pulse_phase_func_t)(void);

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

        bit0:Expanded GPIO 128, 1: Expanded GPIO 129, ..., v: Expanded GPIO 159
    */
    uint8_t ws_pin;
    uint8_t bck_pin;
    uint8_t data_pin;
    i2s_ioexpander_pulse_phase_func_t pulse_phase_func;
    uint32_t pulse_period; // aka step rate.
} i2s_ioexpander_init_t;

/*
  Initialize I2S and DMA for the stepper bitstreamer
  use I2S0, I2S0 isr, DMA, and FIFO(xQueue).

  return -1 ... already initialized
*/
int i2s_ioexpander_init(i2s_ioexpander_init_t &init_param);

/*
  Get a bit state from the internal pin state var.

  pin: expanded pin No. (0..31)
*/
uint8_t i2s_ioexpander_state(uint8_t pin);

/*
   Set a bit in the internal pin state var. (not written electrically)

   pin: expanded pin No. (0..31)
   val: bit value(0 or not 0)
*/
void i2s_ioexpander_write(uint8_t pin, uint8_t val);

/*
    Set current pin state to the I2S bitstream buffer
    (This call will generate a future I2S_IOEXP_USEC_PER_PULSE μs x N bitstream)

    num: Number of samples to be generated
         The number of samples is limited to (20 / I2S_IOEXP_USEC_PER_PULSE).

    return: number of puhsed samples
            0 .. no space for push
 */
uint32_t i2s_ioexpander_push_sample(uint32_t num);

/*
   Set pulser mode to passtrough

   After this function is called,
   the callback function to generate the pulse data
   will not be called.
 */
int i2s_ioexpander_set_passthrough();

/*
   Set pulser mode to stepping

   After this function is called,
   the callback function to generate stepping pulse data
   will be called.
 */
int i2s_ioexpander_set_stepping();

/*
   Set the pulse callback period in microseconds
   (like the timer period for the ISR)
 */
int i2s_ioexpander_set_pulse_period(uint32_t period);

/*
   Register a callback function to generate pulse data
 */
int i2s_ioexpander_register_pulse_callback(i2s_ioexpander_pulse_phase_func_t func);


/*
   Reset i2s I/O expander
   - Stop ISR/DMA
   - Clear DMA buffer with the current expanded GPIO bits
   - Retart ISR/DMA
 */
int i2s_ioexpander_reset();

#endif

/*
   Reference: "ESP32 Technical Reference Manual" by Espressif Systems
     https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf
 */
#endif

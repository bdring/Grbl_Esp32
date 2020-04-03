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

#ifdef USE_I2S_IOEXPANDER
#include <stdint.h>

#define I2S_IOEXP_USEC_PER_PULSE 4 // 1000 x 1000 / ((160 x 1000 x 1000) / 10 / 2) x 32 bit = 4 usec/pulse

typedef uint64_t (*i2s_pulse_phase_isr_t)(void);
typedef void (*i2s_block_phase_isr_t)(uint64_t start_time_usec);

typedef struct {
    uint8_t ws_pin;
    uint8_t bck_pin;
    uint8_t data_pin;
    i2s_pulse_phase_isr_t pulse_func;
    i2s_block_phase_isr_t block_func;
} i2s_ioexpander_init_t;

int i2s_ioexpander_init(i2s_ioexpander_init_t &init_param);
uint8_t i2s_ioexpander_state(uint8_t pin);
void i2s_ioexpander_write(uint8_t pin, uint8_t val);
void i2s_ioexpander_push_sample();
#endif

#endif

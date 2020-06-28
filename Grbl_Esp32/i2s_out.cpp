/*
    i2s_out.cpp
    Part of Grbl_ESP32

    Basic GPIO expander using the ESP32 I2S peripheral (output)

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
#include "config.h"

#ifdef USE_I2S_OUT

#include <FreeRTOS.h>
#include <driver/periph_ctrl.h>
#include <rom/lldesc.h>
#include <soc/i2s_struct.h>
#include <freertos/queue.h>

#include <stdatomic.h>

#include "Pins.h"
#include "i2s_out.h"

//
// Configrations for DMA connected I2S
//
// One DMA buffer transfer takes about 2 ms
//   I2S_OUT_DMABUF_LEN / I2S_SAMPLE_SIZE x I2S_OUT_USEC_PER_PULSE
//   = 2000 / 4 x 4
//   = 2000us = 2ms
// If I2S_OUT_DMABUF_COUNT is 5, it will take about 10 ms for all the DMA buffer transfers to finish.
//
// Increasing I2S_OUT_DMABUF_COUNT has the effect of preventing buffer underflow,
// but on the other hand, it leads to a delay with pulse and/or non-pulse-generated I/Os.
// The number of I2S_OUT_DMABUF_COUNT should be chosen carefully.
//
// Reference information:
//   FreeRTOS task time slice = portTICK_PERIOD_MS = 1 ms (ESP32 FreeRTOS port)
//
#define I2S_SAMPLE_SIZE   4     /* 4 bytes, 32 bits per sample */
#define DMA_SAMPLE_COUNT I2S_OUT_DMABUF_LEN / I2S_SAMPLE_SIZE /* number of samples per buffer */
#define SAMPLE_SAFE_COUNT (20/I2S_OUT_USEC_PER_PULSE) /* prevent buffer overrun (GRBL's $0 should be less than or equal 20) */

#ifdef USE_I2S_OUT_STREAM
typedef struct {
  uint32_t     **buffers;
  uint32_t     *current;
  uint32_t     rw_pos;
  lldesc_t     **desc;
  xQueueHandle queue;
} i2s_out_dma_t;

static i2s_out_dma_t o_dma;
static intr_handle_t i2s_out_isr_handle;
#endif

// output value
static atomic_uint_least32_t i2s_out_port_data = ATOMIC_VAR_INIT(0);

// inner lock
static portMUX_TYPE i2s_out_spinlock = portMUX_INITIALIZER_UNLOCKED;
#define I2S_OUT_ENTER_CRITICAL()  portENTER_CRITICAL(&i2s_out_spinlock)
#define I2S_OUT_EXIT_CRITICAL()   portEXIT_CRITICAL(&i2s_out_spinlock)
#define I2S_OUT_ENTER_CRITICAL_ISR()  portENTER_CRITICAL_ISR(&i2s_out_spinlock)
#define I2S_OUT_EXIT_CRITICAL_ISR()   portEXIT_CRITICAL_ISR(&i2s_out_spinlock)

static int i2s_out_initialized = 0;

#ifdef USE_I2S_OUT_STREAM
static volatile uint32_t i2s_out_pulse_period;
static uint32_t i2s_out_remain_time_until_next_pulse; // Time remaining until the next pulse (μsec)
static volatile i2s_out_pulse_func_t i2s_out_pulse_func;
#endif

static uint8_t i2s_out_ws_pin = 255;
static uint8_t i2s_out_bck_pin = 255;
static uint8_t i2s_out_data_pin = 255;

enum i2s_out_pulser_status_t {
  PASSTHROUGH = 0,  // Static I2S mode.The i2s_out_write() reflected with very little delay
  STEPPING,         // Streaming step data.
  WAITING,          // Waiting for the step DMA completion
};
static volatile i2s_out_pulser_status_t i2s_out_pulser_status = PASSTHROUGH;

// outer lock
static portMUX_TYPE i2s_out_pulser_spinlock = portMUX_INITIALIZER_UNLOCKED;
#define I2S_OUT_PULSER_ENTER_CRITICAL()  portENTER_CRITICAL(&i2s_out_pulser_spinlock)
#define I2S_OUT_PULSER_EXIT_CRITICAL()   portEXIT_CRITICAL(&i2s_out_pulser_spinlock)
#define I2S_OUT_PULSER_ENTER_CRITICAL_ISR()  portENTER_CRITICAL_ISR(&i2s_out_pulser_spinlock)
#define I2S_OUT_PULSER_EXIT_CRITICAL_ISR()   portEXIT_CRITICAL_ISR(&i2s_out_pulser_spinlock)

//
// Internal functions
//
static inline void gpio_matrix_out_check(uint8_t gpio, uint32_t signal_idx, bool out_inv, bool oen_inv) {
  //if pin == 255, do not need to configure
  if (gpio != 255) {
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction((gpio_num_t)gpio, (gpio_mode_t)GPIO_MODE_DEF_OUTPUT);
    gpio_matrix_out(gpio, signal_idx, out_inv, oen_inv);
  }
}

static inline void i2s_out_single_data() {
#if I2S_OUT_NUM_BITS == 16
  uint32_t port_data = atomic_load(&i2s_out_port_data);
  port_data <<= 16; // Shift needed. This specification is not spelled out in the manual.
  I2S0.conf_single_data = port_data; // Apply port data in real-time (static I2S)
#else
  I2S0.conf_single_data = atomic_load(&i2s_out_port_data); // Apply port data in real-time (static I2S)
#endif
}

static inline void i2s_out_reset_fifo_without_lock() {
  I2S0.conf.rx_fifo_reset = 1;
  I2S0.conf.rx_fifo_reset = 0;
  I2S0.conf.tx_fifo_reset = 1;
  I2S0.conf.tx_fifo_reset = 0;
}

static void IRAM_ATTR i2s_out_reset_fifo() {
  I2S_OUT_ENTER_CRITICAL();
  i2s_out_reset_fifo_without_lock();
  I2S_OUT_EXIT_CRITICAL();
}

#ifdef USE_I2S_OUT_STREAM
static int IRAM_ATTR i2s_clear_dma_buffer(lldesc_t *dma_desc, uint32_t port_data) {

  uint32_t *buf = (uint32_t*)dma_desc->buf;
  for (int i = 0; i < DMA_SAMPLE_COUNT; i++) {
    buf[i] = port_data;
  }
  // Restore the buffer length.
  // The length may have been changed short when the data was filled in to prevent buffer overrun.
  dma_desc->length = I2S_OUT_DMABUF_LEN;
  return 0;
}

static int IRAM_ATTR i2s_clear_o_dma_buffers(uint32_t port_data) {
  for (int buf_idx = 0; buf_idx < I2S_OUT_DMABUF_COUNT; buf_idx++) {
    // Initialize DMA descriptor
    o_dma.desc[buf_idx]->owner = 1;
    o_dma.desc[buf_idx]->eof = 1; // set to 1 will trigger the interrupt
    o_dma.desc[buf_idx]->sosf = 0;
    o_dma.desc[buf_idx]->length = I2S_OUT_DMABUF_LEN;
    o_dma.desc[buf_idx]->size = I2S_OUT_DMABUF_LEN;
    o_dma.desc[buf_idx]->buf = (uint8_t *) o_dma.buffers[buf_idx];
    o_dma.desc[buf_idx]->offset = 0;
    o_dma.desc[buf_idx]->qe.stqe_next = (lldesc_t *)((buf_idx < (I2S_OUT_DMABUF_COUNT - 1)) ? (o_dma.desc[buf_idx + 1]) : o_dma.desc[0]);
    i2s_clear_dma_buffer(o_dma.desc[buf_idx], port_data);
  }
  return 0;
}
#endif

static int IRAM_ATTR i2s_out_gpio_attach(uint8_t ws, uint8_t bck, uint8_t data) {
  // Route the i2s pins to the appropriate GPIO
  gpio_matrix_out_check(data, I2S0O_DATA_OUT23_IDX, 0, 0);
  gpio_matrix_out_check(bck, I2S0O_BCK_OUT_IDX, 0, 0);
  gpio_matrix_out_check(ws, I2S0O_WS_OUT_IDX, 0, 0);
  return 0;
}

#define I2S_OUT_DETACH_PORT_IDX   0x100

static int IRAM_ATTR i2s_out_gpio_detach(uint8_t ws, uint8_t bck, uint8_t data) {
  // Route the i2s pins to the appropriate GPIO
  gpio_matrix_out_check(ws, I2S_OUT_DETACH_PORT_IDX, 0, 0);
  gpio_matrix_out_check(bck, I2S_OUT_DETACH_PORT_IDX, 0, 0);
  gpio_matrix_out_check(data, I2S_OUT_DETACH_PORT_IDX, 0, 0);
  return 0;
}

static int IRAM_ATTR i2s_out_gpio_shiftout(uint32_t port_data) {
  __digitalWrite(i2s_out_ws_pin, LOW);
  for (int i = 0; i <I2S_OUT_NUM_BITS; i++) {
    __digitalWrite(i2s_out_data_pin, !!(port_data & (1 << (I2S_OUT_NUM_BITS-1 - i))));
    __digitalWrite(i2s_out_bck_pin, HIGH);
    __digitalWrite(i2s_out_bck_pin, LOW);
  }
  __digitalWrite(i2s_out_ws_pin, HIGH); // Latch
  return 0;
}

static int IRAM_ATTR i2s_out_stop() {
  I2S_OUT_ENTER_CRITICAL();
#ifdef USE_I2S_OUT_STREAM
  // Stop FIFO DMA
  I2S0.out_link.stop = 1;

  // Disconnect DMA from FIFO
  I2S0.fifo_conf.dscr_en = 0; //Unset this bit to disable I2S DMA mode. (R/W)
#endif
  // stop TX module
  I2S0.conf.tx_start = 0;

  // Force WS to LOW before detach
  // This operation prevents unintended WS edge trigger when detach
  __digitalWrite(i2s_out_ws_pin, LOW);

  // Now, detach GPIO pin from I2S
  i2s_out_gpio_detach(i2s_out_ws_pin, i2s_out_bck_pin, i2s_out_data_pin);

  // Force BCK to LOW
  // After the TX module is stopped, BCK always seems to be in LOW.
  // However, I'm going to do it manually to ensure the BCK's LOW.
  __digitalWrite(i2s_out_bck_pin, LOW);

  // Transmit recovery data to 74HC595
  uint32_t port_data = atomic_load(&i2s_out_port_data); // current expanded port value
  i2s_out_gpio_shiftout(port_data);

#ifdef USE_I2S_OUT_STREAM
  //clear pending interrupt
  I2S0.int_clr.val = I2S0.int_st.val;
#endif
  I2S_OUT_EXIT_CRITICAL();
  return 0;
}

static int IRAM_ATTR i2s_out_start() {
  if (!i2s_out_initialized) {
    return -1;
  }

  I2S_OUT_ENTER_CRITICAL();
  // Transmit recovery data to 74HC595
  uint32_t port_data = atomic_load(&i2s_out_port_data); // current expanded port value
  i2s_out_gpio_shiftout(port_data);

  // Attach I2S to specified GPIO pin
  i2s_out_gpio_attach(i2s_out_ws_pin, i2s_out_bck_pin, i2s_out_data_pin);
  //start DMA link
  i2s_out_reset_fifo_without_lock();

#ifdef USE_I2S_OUT_STREAM
  if (i2s_out_pulser_status == PASSTHROUGH) {
    I2S0.conf_chan.tx_chan_mod = 3; // 3:right+constant 4:left+constant (when tx_msb_right = 1)
    I2S0.conf_single_data = port_data;
  } else {
    I2S0.conf_chan.tx_chan_mod = 4; // 3:right+constant 4:left+constant (when tx_msb_right = 1)
    I2S0.conf_single_data = 0;
  }
#endif

#ifdef USE_I2S_OUT_STREAM
  //reset DMA
  I2S0.lc_conf.in_rst = 1;
  I2S0.lc_conf.in_rst = 0;
  I2S0.lc_conf.out_rst = 1;
  I2S0.lc_conf.out_rst = 0;

  I2S0.out_link.addr = (uint32_t)o_dma.desc[0];
#endif

  I2S0.conf.tx_reset = 1;
  I2S0.conf.tx_reset = 0;
  I2S0.conf.rx_reset = 1;
  I2S0.conf.rx_reset = 0;

  I2S0.conf1.tx_stop_en = 1; // BCK and WCK are suppressed while FIFO is empty

#ifdef USE_I2S_OUT_STREAM
  // Connect DMA to FIFO
  I2S0.fifo_conf.dscr_en = 1; // Set this bit to enable I2S DMA mode. (R/W)

  I2S0.int_clr.val = 0xFFFFFFFF;
  I2S0.out_link.start = 1;
#endif
  I2S0.conf.tx_start = 1;
  // Wait for the first FIFO data to prevent the unintentional generation of 0 data
  ets_delay_us(20);
  I2S0.conf1.tx_stop_en = 0; // BCK and WCK are generated regardless of the FIFO status

  I2S_OUT_EXIT_CRITICAL();

  return 0;
}

#ifdef USE_I2S_OUT_STREAM

static int IRAM_ATTR i2s_fillout_dma_buffer(lldesc_t *dma_desc) {
  uint32_t *buf = (uint32_t*)dma_desc->buf;
  o_dma.rw_pos = 0;
  // It reuses the oldest (just transferred) buffer with the name "current"
  // and fills the buffer for later DMA.
  I2S_OUT_PULSER_ENTER_CRITICAL(); // Lock pulser status
  if (i2s_out_pulser_status == STEPPING) {
    //
    // Fillout the buffer for pulse
    //
    // To avoid buffer overflow, all of the maximum pulse width (normaly about 10us)
    // is adjusted to be in a single buffer.
    // DMA_SAMPLE_SAFE_COUNT is referred to as the margin value.
    // Therefore, if a buffer is close to full and it is time to generate a pulse,
    // the generation of the buffer is interrupted (the buffer length is shortened slightly)
    // and the pulse generation is postponed until the next buffer is filled.
    //
    o_dma.rw_pos = 0;
    while (o_dma.rw_pos < (DMA_SAMPLE_COUNT - SAMPLE_SAFE_COUNT)) {
        // no data to read (buffer empty)
        if (i2s_out_remain_time_until_next_pulse < I2S_OUT_USEC_PER_PULSE) {
          // pulser status may change in pulse phase func, so I need to check it every time.
          if (i2s_out_pulser_status == STEPPING) {
            // fillout future DMA buffer (tail of the DMA buffer chains)
            if (i2s_out_pulse_func != NULL) {
              I2S_OUT_PULSER_EXIT_CRITICAL(); // Temporarily unlocked status lock as it may be locked in pulse callback.
              (*i2s_out_pulse_func)(); // should be pushed into buffer max DMA_SAMPLE_SAFE_COUNT
              I2S_OUT_PULSER_ENTER_CRITICAL(); // Lock again.
              i2s_out_remain_time_until_next_pulse = i2s_out_pulse_period;
              if (i2s_out_pulser_status == WAITING) {
                // i2s_out_set_passthrough() has called from the pulse function.
                // It needs to go into pass-through mode.
                // This DMA descriptor must be a tail of the chain.
                dma_desc->qe.stqe_next = NULL; // Cut the DMA descriptor ring. This allow us to identify the tail of the buffer.
              } else if (i2s_out_pulser_status == PASSTHROUGH) {
                // i2s_out_reset() has called during the execution of the pulse function.
                // I2S has already in static mode, and buffers has cleared to zero.
                // To prevent the pulse function from being called back,
                // we assume that the buffer is already full.
                i2s_out_remain_time_until_next_pulse = 0; // There is no need to fill the current buffer.
                o_dma.rw_pos = DMA_SAMPLE_COUNT; // The buffer is full.
                break;
              }
              continue;
            }
          }
        }
        // no pulse data in push buffer (pulse off or idle or callback is not defined)
        buf[o_dma.rw_pos++] = atomic_load(&i2s_out_port_data);
        if (i2s_out_remain_time_until_next_pulse >= I2S_OUT_USEC_PER_PULSE) {
          i2s_out_remain_time_until_next_pulse -= I2S_OUT_USEC_PER_PULSE;
        } else {
          i2s_out_remain_time_until_next_pulse = 0;
        }
    }
    // set filled length to the DMA descriptor
    dma_desc->length = o_dma.rw_pos * I2S_SAMPLE_SIZE;
  } else if (i2s_out_pulser_status == WAITING) {
    i2s_clear_dma_buffer(dma_desc, 0); // Essentially, no clearing is required. I'll make sure I know when I've written something.
    o_dma.rw_pos = 0; // If someone calls i2s_out_push_sample, make sure there is no buffer overflow
    dma_desc->qe.stqe_next = NULL; // Cut the DMA descriptor ring. This allow us to identify the tail of the buffer.
  } else {
    // Stepper paused (passthrough state, static I2S control mode)
    // In the passthrough mode, there is no need to fill the buffer with port_data.
    i2s_clear_dma_buffer(dma_desc, 0); // Essentially, no clearing is required. I'll make sure I know when I've written something.
    o_dma.rw_pos = 0; // If someone calls i2s_out_push_sample, make sure there is no buffer overflow
  }
  I2S_OUT_PULSER_EXIT_CRITICAL(); // Unlock pulser status

  return 0;
}

//
// I2S out DMA Interrupts handler
//
static void IRAM_ATTR i2s_out_intr_handler(void *arg) {
  lldesc_t *finish_desc;
  portBASE_TYPE high_priority_task_awoken = pdFALSE;

  if (I2S0.int_st.out_eof || I2S0.int_st.out_total_eof) {
    if (I2S0.int_st.out_total_eof) {
      // This is tail of the DMA descriptors
      I2S_OUT_ENTER_CRITICAL_ISR();
      // Stop FIFO DMA
      I2S0.out_link.stop = 1;
      // Disconnect DMA from FIFO
      I2S0.fifo_conf.dscr_en = 0; //Unset this bit to disable I2S DMA mode. (R/W)
      // Stop TX module
      I2S0.conf.tx_start = 0;
      I2S_OUT_EXIT_CRITICAL_ISR();
    }
    // Get the descriptor of the last item in the linkedlist
    finish_desc = (lldesc_t*) I2S0.out_eof_des_addr;

    // If the queue is full it's because we have an underflow,
    // more than buf_count isr without new data, remove the front buffer
    if (xQueueIsQueueFullFromISR(o_dma.queue)) {
      lldesc_t *front_desc;
      // Remove a descriptor from the DMA complete event queue
      xQueueReceiveFromISR(o_dma.queue, &front_desc, &high_priority_task_awoken);
      I2S_OUT_PULSER_ENTER_CRITICAL_ISR();
      uint32_t port_data = 0;
      if (i2s_out_pulser_status == STEPPING) {
        port_data = atomic_load(&i2s_out_port_data);
      }
      I2S_OUT_PULSER_EXIT_CRITICAL_ISR();
      for (int i = 0; i < DMA_SAMPLE_COUNT; i++) {
        front_desc->buf[i] = port_data;
      }
      front_desc->length = I2S_OUT_DMABUF_LEN;
    }

    // Send a DMA complete event to the I2S bitstreamer task with finished buffer
    xQueueSendFromISR(o_dma.queue, &finish_desc, &high_priority_task_awoken);
  }

  if (high_priority_task_awoken == pdTRUE) portYIELD_FROM_ISR();

  // clear interrupt
  I2S0.int_clr.val = I2S0.int_st.val; //clear pending interrupt
}

//
// I2S bitstream generator task
//
static void IRAM_ATTR i2sOutTask(void* parameter) {
  lldesc_t *dma_desc;
  while (1) {
    // Wait a DMA complete event from I2S isr
    // (Block until a DMA transfer has complete)
    xQueueReceive(o_dma.queue, &dma_desc, portMAX_DELAY);
    o_dma.current = (uint32_t*)(dma_desc->buf);
    // It reuses the oldest (just transferred) buffer with the name "current"
    // and fills the buffer for later DMA.
    I2S_OUT_PULSER_ENTER_CRITICAL(); // Lock pulser status
    if (i2s_out_pulser_status == STEPPING) {
      //
      // Fillout the buffer for pulse
      //
      // To avoid buffer overflow, all of the maximum pulse width (normaly about 10us)
      // is adjusted to be in a single buffer.
      // DMA_SAMPLE_SAFE_COUNT is referred to as the margin value.
      // Therefore, if a buffer is close to full and it is time to generate a pulse,
      // the generation of the buffer is interrupted (the buffer length is shortened slightly)
      // and the pulse generation is postponed until the next buffer is filled.
      //
      i2s_fillout_dma_buffer(dma_desc);
      dma_desc->length = o_dma.rw_pos * I2S_SAMPLE_SIZE;
    } else if (i2s_out_pulser_status == WAITING) {
      if (dma_desc->qe.stqe_next == NULL) {
        // Tail of the DMA descriptor found
        // I2S TX module has alrewdy stopped by ISR
        i2s_out_stop();
        i2s_clear_o_dma_buffers(0); // 0 for static I2S control mode (right ch. data is always 0)
        // You need to set the status before calling i2s_out_start()
        // because the process in i2s_out_start() is different depending on the status.
        i2s_out_pulser_status = PASSTHROUGH;
        i2s_out_start();
      } else {
        // Processing a buffer slightly ahead of the tail buffer.
        // We don't need to fill up the buffer by port_data any more.
        i2s_clear_dma_buffer(dma_desc, 0); // Essentially, no clearing is required. I'll make sure I know when I've written something.
        o_dma.rw_pos = 0; // If someone calls i2s_out_push_sample, make sure there is no buffer overflow
        dma_desc->qe.stqe_next = NULL; // Cut the DMA descriptor ring. This allow us to identify the tail of the buffer.
      }
    } else {
        // Stepper paused (passthrough state, static I2S control mode)
        // In the passthrough mode, there is no need to fill the buffer with port_data.
        i2s_clear_dma_buffer(dma_desc, 0); // Essentially, no clearing is required. I'll make sure I know when I've written something.
        o_dma.rw_pos = 0; // If someone calls i2s_out_push_sample, make sure there is no buffer overflow
    }
    I2S_OUT_PULSER_EXIT_CRITICAL(); // Unlock pulser status
  }
}
#endif

//
// External funtions
//
void IRAM_ATTR i2s_out_delay() {
#ifdef USE_I2S_OUT_STREAM
 I2S_OUT_PULSER_ENTER_CRITICAL();
  if (i2s_out_pulser_status == PASSTHROUGH) {
    // Depending on the timing, it may not be reflected immediately,
    // so wait twice as long just in case.
    ets_delay_us(I2S_OUT_USEC_PER_PULSE * 2);
  } else {
    // Just wait until the data now registered in the DMA descripter
    // is reflected in the I2S TX module via FIFO.
    delay(I2S_OUT_DELAY_MS);
  }
 I2S_OUT_PULSER_EXIT_CRITICAL();
#else
  ets_delay_us(I2S_OUT_USEC_PER_PULSE * 2);
#endif
}

void IRAM_ATTR i2s_out_write(uint8_t pin, uint8_t val) {
  uint32_t bit = 1UL << pin;
  if (val) {
    atomic_fetch_or(&i2s_out_port_data, bit);
  } else {
    atomic_fetch_and(&i2s_out_port_data, ~bit);
  }
#ifdef USE_I2S_OUT_STREAM
  // It needs a lock for access, but I've given up because I need speed.
  // This is not a problem as long as there is no overlap between the status change and digitalWrite().
  if (i2s_out_pulser_status == PASSTHROUGH) {
    i2s_out_single_data();
  }
#else
  i2s_out_single_data();
#endif
}

uint8_t IRAM_ATTR i2s_out_state(uint8_t pin) {
  uint32_t port_data = atomic_load(&i2s_out_port_data);
  return (!!(port_data & (1UL << pin)));
}

uint32_t IRAM_ATTR i2s_out_push_sample(uint32_t num) {
#ifdef USE_I2S_OUT_STREAM
  if (num > SAMPLE_SAFE_COUNT) {
    return 0;
  }
  // push at least one sample (even if num is zero)
  uint32_t port_data = atomic_load(&i2s_out_port_data);
  uint32_t n = 0;
  do {
    o_dma.current[o_dma.rw_pos++] = port_data;
    n++;
  } while(n < num);
  return n;
#else
  return 0;
#endif
}

int IRAM_ATTR i2s_out_set_passthrough() {
  I2S_OUT_PULSER_ENTER_CRITICAL();
#ifdef USE_I2S_OUT_STREAM
  if (i2s_out_pulser_status == STEPPING) {
    i2s_out_pulser_status = WAITING; // Start stopping the pulser
  }
#else
  i2s_out_pulser_status = PASSTHROUGH;
#endif
  I2S_OUT_PULSER_EXIT_CRITICAL();
  return 0;
}

int IRAM_ATTR i2s_out_set_stepping() {
  I2S_OUT_PULSER_ENTER_CRITICAL();
#ifdef USE_I2S_OUT_STREAM
  if (i2s_out_pulser_status == STEPPING) {
    // Re-entered (fail safe)
    I2S_OUT_PULSER_EXIT_CRITICAL();
    return 0;
  }

  if (i2s_out_pulser_status == WAITING) {
    // Wait for complete DMAs
    for(;;) {
      I2S_OUT_PULSER_EXIT_CRITICAL();
      delay(I2S_OUT_DELAY_DMABUF_MS);
      I2S_OUT_PULSER_ENTER_CRITICAL();
      if (i2s_out_pulser_status == WAITING) {
        continue;
      }
      if (i2s_out_pulser_status == PASSTHROUGH) {
        // DMA completed
        break;
      }
      // Another function change the I2S state to STEPPING
      I2S_OUT_PULSER_EXIT_CRITICAL();
      return 0;
    }
  }

  // Change I2S state from PASSTHROUGH to STEPPING
  i2s_out_stop();
  uint32_t port_data = atomic_load(&i2s_out_port_data);
  i2s_clear_o_dma_buffers(port_data);

  // You need to set the status before calling i2s_out_start()
  // because the process in i2s_out_start() is different depending on the status.
  i2s_out_pulser_status = STEPPING;
  i2s_out_start();
#else
  i2s_out_pulser_status = STEPPING;
#endif
  I2S_OUT_PULSER_EXIT_CRITICAL();
  return 0;
}

int IRAM_ATTR i2s_out_set_pulse_period(uint32_t period) {
#ifdef USE_I2S_OUT_STREAM
  i2s_out_pulse_period = period;
#endif
  return 0;
}

int IRAM_ATTR i2s_out_set_pulse_callback(i2s_out_pulse_func_t func) {
#ifdef USE_I2S_OUT_STREAM
  i2s_out_pulse_func = func;
#endif
  return 0;
}

int IRAM_ATTR i2s_out_reset() {
  I2S_OUT_PULSER_ENTER_CRITICAL();
  i2s_out_stop();
#ifdef USE_I2S_OUT_STREAM
  if (i2s_out_pulser_status == STEPPING) {
    uint32_t port_data = atomic_load(&i2s_out_port_data);
    i2s_clear_o_dma_buffers(port_data);
  } else if (i2s_out_pulser_status == WAITING) {
    i2s_clear_o_dma_buffers(0);
    i2s_out_pulser_status = PASSTHROUGH;
  }
#endif
  // You need to set the status before calling i2s_out_start()
  // because the process in i2s_out_start() is different depending on the status.
  i2s_out_start();
  I2S_OUT_PULSER_EXIT_CRITICAL();
  return 0;
}

//
// Initialize funtion (external function)
//
int IRAM_ATTR i2s_out_init(i2s_out_init_t &init_param) {
  if (i2s_out_initialized) {
    // already initialized
    return -1;
  }

  atomic_store(&i2s_out_port_data, init_param.init_val);

  // To make sure hardware is enabled before any hardware register operations.
  periph_module_reset(PERIPH_I2S0_MODULE);
  periph_module_enable(PERIPH_I2S0_MODULE);

  // Route the i2s pins to the appropriate GPIO
  i2s_out_gpio_attach(init_param.ws_pin, init_param.bck_pin, init_param.data_pin);

  /**
   * Each i2s transfer will take
   *   fpll = PLL_D2_CLK      -- clka_en = 0
   *
   *   fi2s = fpll / N + b/a  -- N + b/a = clkm_div_num
   *   fi2s = 160MHz / 2
   *   fi2s = 80MHz
   *
   *   fbclk = fi2s / M   -- M = tx_bck_div_num
   *   fbclk = 80MHz / 2
   *   fbclk = 40MHz
   *
   *   fwclk = fbclk / 32
   *
   *   for fwclk = 250kHz(16-bit: 4µS pulse time), 125kHz(32-bit: 8μS pulse time)
   *      N = 10, b/a = 0
   *      M = 2
   *   for fwclk = 500kHz(16-bit: 2µS pulse time), 250kHz(32-bit: 4μS pulse time)
   *      N = 5, b/a = 0
   *      M = 2
   *   for fwclk = 1000kHz(16-bit: 1µS pulse time), 500kHz(32-bit: 2μS pulse time)
   *      N = 2, b/a = 2/1 (N + b/a = 2.5)
   *      M = 2
   */

#ifdef USE_I2S_OUT_STREAM
  // Allocate the array of pointers to the buffers
  o_dma.buffers = (uint32_t **)malloc(sizeof(uint32_t*) * I2S_OUT_DMABUF_COUNT);
  if (o_dma.buffers == nullptr) return -1;

  // Allocate each buffer that can be used by the DMA controller
  for (int buf_idx = 0; buf_idx < I2S_OUT_DMABUF_COUNT; buf_idx++) {
    o_dma.buffers[buf_idx] = (uint32_t*) heap_caps_calloc(1, I2S_OUT_DMABUF_LEN, MALLOC_CAP_DMA);
    if (o_dma.buffers[buf_idx] == nullptr) return -1;
  }

  // Allocate the array of DMA descriptors
  o_dma.desc = (lldesc_t**) malloc(sizeof(lldesc_t*) * I2S_OUT_DMABUF_COUNT);
  if (o_dma.desc == nullptr) return -1;

  // Allocate each DMA descriptor that will be used by the DMA controller
  for (int buf_idx = 0; buf_idx < I2S_OUT_DMABUF_COUNT; buf_idx++) {
    o_dma.desc[buf_idx] = (lldesc_t*) heap_caps_malloc(sizeof(lldesc_t), MALLOC_CAP_DMA);
    if (o_dma.desc[buf_idx] == nullptr) return -1;
  }

  // Initialize
  i2s_clear_o_dma_buffers(init_param.init_val);
  o_dma.rw_pos = 0;
  o_dma.current = NULL;
  o_dma.queue = xQueueCreate(I2S_OUT_DMABUF_COUNT, sizeof(uint32_t *));

   // Set the first DMA descriptor
  I2S0.out_link.addr = (uint32_t)o_dma.desc[0];
#endif

  // stop i2s
  I2S0.out_link.stop = 1;
  I2S0.conf.tx_start = 0;

  I2S0.int_clr.val = I2S0.int_st.val; //clear pending interrupt

  //
  // i2s_param_config
  //

  // configure I2S data port interface.
  i2s_out_reset_fifo();

  //reset i2s
  I2S0.conf.tx_reset = 1;
  I2S0.conf.tx_reset = 0;
  I2S0.conf.rx_reset = 1;
  I2S0.conf.rx_reset = 0;

  //reset dma
  I2S0.lc_conf.in_rst = 1; // Set this bit to reset in DMA FSM. (R/W)
  I2S0.lc_conf.in_rst = 0;
  I2S0.lc_conf.out_rst = 1; // Set this bit to reset out DMA FSM. (R/W)
  I2S0.lc_conf.out_rst = 0;

  //Enable and configure DMA
  I2S0.lc_conf.check_owner = 0;
  I2S0.lc_conf.out_loop_test = 0;
  I2S0.lc_conf.out_auto_wrback = 0; // Disable auto outlink-writeback when all the data has been transmitted
  I2S0.lc_conf.out_data_burst_en = 0;
  I2S0.lc_conf.outdscr_burst_en = 0;
  I2S0.lc_conf.out_no_restart_clr = 0;
  I2S0.lc_conf.indscr_burst_en = 0;
#ifdef USE_I2S_OUT_STREAM
  I2S0.lc_conf.out_eof_mode = 1; // I2S_OUT_EOF_INT generated when DMA has popped all data from the FIFO;
#endif
  I2S0.conf2.lcd_en = 0;
  I2S0.conf2.camera_en = 0;
  I2S0.pdm_conf.pcm2pdm_conv_en = 0;
  I2S0.pdm_conf.pdm2pcm_conv_en = 0;

  I2S0.fifo_conf.dscr_en = 0;

#ifdef USE_I2S_OUT_STREAM
  if (i2s_out_pulser_status == STEPPING) {
    // Stream output mode
    I2S0.conf_chan.tx_chan_mod = 4; // 3:right+constant 4:left+constant (when tx_msb_right = 1)
    I2S0.conf_single_data = 0;
  } else {
    // Static output mode
    I2S0.conf_chan.tx_chan_mod = 3; // 3:right+constant 4:left+constant (when tx_msb_right = 1)
    I2S0.conf_single_data = init_param.init_val;
  }
#else
  // For the static output mode
  I2S0.conf_chan.tx_chan_mod = 3; // 3:right+constant 4:left+constant (when tx_msb_right = 1)
  I2S0.conf_single_data = init_param.init_val; // initial constant value
#endif
#if I2S_OUT_NUM_BITS == 16
  I2S0.fifo_conf.tx_fifo_mod = 0; // 0: 16-bit dual channel data, 3: 32-bit single channel data
  I2S0.fifo_conf.rx_fifo_mod = 0; // 0: 16-bit dual channel data, 3: 32-bit single channel data
  I2S0.sample_rate_conf.tx_bits_mod = 16; // default is 16-bits
  I2S0.sample_rate_conf.rx_bits_mod = 16; // default is 16-bits
#else
  I2S0.fifo_conf.tx_fifo_mod = 3; // 0: 16-bit dual channel data, 3: 32-bit single channel data
  I2S0.fifo_conf.rx_fifo_mod = 3; // 0: 16-bit dual channel data, 3: 32-bit single channel data
  // Data width is 32-bit. Forgetting this setting will result in a 16-bit transfer.
  I2S0.sample_rate_conf.tx_bits_mod = 32;
  I2S0.sample_rate_conf.rx_bits_mod = 32;
#endif
  I2S0.conf.tx_mono = 0; // Set this bit to enable transmitter’s mono mode in PCM standard mode.

  I2S0.conf_chan.rx_chan_mod = 1; // 1: right+right
  I2S0.conf.rx_mono = 0;

#ifdef USE_I2S_OUT_STREAM
  I2S0.fifo_conf.dscr_en = 1; //connect DMA to fifo
#endif
  I2S0.conf.tx_start = 0;
  I2S0.conf.rx_start = 0;

  I2S0.conf.tx_msb_right = 1; // Set this bit to place right-channel data at the MSB in the transmit FIFO.
  I2S0.conf.tx_right_first = 0; // Setting this bit allows the right-channel data to be sent first.

  I2S0.conf.tx_slave_mod = 0; // Master
#ifdef USE_I2S_OUT_STREAM
  I2S0.fifo_conf.tx_fifo_mod_force_en = 1; //The bit should always be set to 1.
#endif
  I2S0.pdm_conf.rx_pdm_en = 0; // Set this bit to enable receiver’s PDM mode.
  I2S0.pdm_conf.tx_pdm_en = 0; // Set this bit to enable transmitter’s PDM mode.

  // I2S_COMM_FORMAT_I2S_LSB
  I2S0.conf.tx_short_sync = 0; // Set this bit to enable transmitter in PCM standard mode.
  I2S0.conf.rx_short_sync = 0; // Set this bit to enable receiver in PCM standard mode.
  I2S0.conf.tx_msb_shift = 0; // Do not use the Philips standard to avoid bit-shifting
  I2S0.conf.rx_msb_shift = 0; // Do not use the Philips standard to avoid bit-shifting

  //
  // i2s_set_clk
  //

  // set clock (fi2s) 160MHz / 5
  I2S0.clkm_conf.clka_en = 0;       // Use 160 MHz PLL_D2_CLK as reference
  // N + b/a = 0
#if I2S_OUT_NUM_BITS == 16
  // N = 10
  I2S0.clkm_conf.clkm_div_num = 10; // minimum value of 2, reset value of 4, max 256 (I²S clock divider’s integral value)
#else
  // N = 5
  I2S0.clkm_conf.clkm_div_num = 5; // minimum value of 2, reset value of 4, max 256 (I²S clock divider’s integral value)
#endif
  // b/a = 0
  I2S0.clkm_conf.clkm_div_b = 0;    // 0 at reset
  I2S0.clkm_conf.clkm_div_a = 0;    // 0 at reset, what about divide by 0? (not an issue)

  // Bit clock configuration bit in transmitter mode.
  // fbck = fi2s / tx_bck_div_num = (160 MHz / 5) / 2 = 16 MHz
  I2S0.sample_rate_conf.tx_bck_div_num = 2; // minimum value of 2 defaults to 6
  I2S0.sample_rate_conf.rx_bck_div_num = 2;

#ifdef USE_I2S_OUT_STREAM
  // Enable TX interrupts (DMA Interrupts)
  I2S0.int_ena.out_eof = 1; // Triggered when rxlink has finished sending a packet.
  I2S0.int_ena.out_dscr_err = 0; // Triggered when invalid rxlink descriptors are encountered.
  I2S0.int_ena.out_total_eof = 1; // Triggered when all transmitting linked lists are used up.
  I2S0.int_ena.out_done = 0; // Triggered when all transmitted and buffered data have been read.

  // default pulse callback period (μsec)
  i2s_out_pulse_period = init_param.pulse_period;
  i2s_out_pulse_func = init_param.pulse_func;

  // Create the task that will feed the buffer
  xTaskCreatePinnedToCore(i2sOutTask,
                          "I2SOutTask",
                          1024 * 10,
                          NULL,
                          1,
                          nullptr,
                          CONFIG_ARDUINO_RUNNING_CORE  // must run the task on same core
                          );

  // Allocate and Enable the I2S interrupt
  esp_intr_alloc(ETS_I2S0_INTR_SOURCE, 0, i2s_out_intr_handler, nullptr, &i2s_out_isr_handle);
  esp_intr_enable(i2s_out_isr_handle);
#endif

  // Remember GPIO pin numbers
  i2s_out_ws_pin = init_param.ws_pin;
  i2s_out_bck_pin = init_param.bck_pin;
  i2s_out_data_pin = init_param.data_pin;
  i2s_out_initialized = 1;

  // Start the I2S peripheral
  i2s_out_start();

  return 0;
}

#ifndef I2S_OUT_WS
#define I2S_OUT_WS    GPIO_NUM_17
#endif
#ifndef I2S_OUT_BCK
#define I2S_OUT_BCK   GPIO_NUM_22
#endif
#ifndef I2S_OUT_DATA
#define I2S_OUT_DATA  GPIO_NUM_21
#endif
#ifndef I2S_OUT_INIT_VAL
#define I2S_OUT_INIT_VAL  0
#endif
/*
  Initialize I2S out by default parameters.

  return -1 ... already initialized
*/
int IRAM_ATTR i2s_out_init() {
    i2s_out_init_t default_param = {
        .ws_pin = I2S_OUT_WS,
        .bck_pin = I2S_OUT_BCK,
        .data_pin = I2S_OUT_DATA,
        .pulse_func = NULL,
        .pulse_period = I2S_OUT_USEC_PER_PULSE,
        .init_val = I2S_OUT_INIT_VAL,
    };
    return i2s_out_init(default_param);
}

#endif

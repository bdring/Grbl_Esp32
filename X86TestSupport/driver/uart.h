#pragma once

/**
 * @brief UART mode selection
 */
typedef enum {
    UART_MODE_UART                   = 0x00, /*!< mode: regular UART mode*/
    UART_MODE_RS485_HALF_DUPLEX      = 0x01, /*!< mode: half duplex RS485 UART mode control by RTS pin */
    UART_MODE_IRDA                   = 0x02, /*!< mode: IRDA  UART mode*/
    UART_MODE_RS485_COLLISION_DETECT = 0x03, /*!< mode: RS485 collision detection UART mode (used for test purposes)*/
    UART_MODE_RS485_APP_CTRL         = 0x04, /*!< mode: application control RS485 UART mode (used for test purposes)*/
} uart_mode_t;

/**
 * @brief UART word length constants
 */
typedef enum {
    UART_DATA_5_BITS   = 0x0, /*!< word length: 5bits*/
    UART_DATA_6_BITS   = 0x1, /*!< word length: 6bits*/
    UART_DATA_7_BITS   = 0x2, /*!< word length: 7bits*/
    UART_DATA_8_BITS   = 0x3, /*!< word length: 8bits*/
    UART_DATA_BITS_MAX = 0x4,
} uart_word_length_t;

/**
 * @brief UART stop bits number
 */
typedef enum {
    UART_STOP_BITS_1   = 0x1, /*!< stop bit: 1bit*/
    UART_STOP_BITS_1_5 = 0x2, /*!< stop bit: 1.5bits*/
    UART_STOP_BITS_2   = 0x3, /*!< stop bit: 2bits*/
    UART_STOP_BITS_MAX = 0x4,
} uart_stop_bits_t;

/**
 * @brief UART peripheral number
 */
typedef enum {
    UART_NUM_0 = 0x0, /*!< UART base address 0x3ff40000*/
    UART_NUM_1 = 0x1, /*!< UART base address 0x3ff50000*/
    UART_NUM_2 = 0x2, /*!< UART base address 0x3ff6e000*/
    UART_NUM_MAX,
} uart_port_t;

/**
 * @brief UART parity constants
 */
typedef enum {
    UART_PARITY_DISABLE = 0x0, /*!< Disable UART parity*/
    UART_PARITY_EVEN    = 0x2, /*!< Enable UART even parity*/
    UART_PARITY_ODD     = 0x3  /*!< Enable UART odd parity*/
} uart_parity_t;

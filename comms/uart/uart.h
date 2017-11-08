/**
 *
 * \file libesoup/comms/uart.h
 *
 * UART functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */
#ifndef ES_LIB_UART_H
#define ES_LIB_UART_H

/*
 * UART Defs
 */
#define UART_8_DATABITS       8
#define UART_9_DATABITS       9

#define UART_PARITY_NONE      0
#define UART_PARITY_ODD       1
#define UART_PARITY_EVEN      2

#define UART_ONE_STOP_BIT     1
#define UART_TWO_STOP_BITS    2

#define UART_IDLE_LOW         0
#define UART_IDLE_HIGH        1
#if defined(XC16) || defined(__XC8)
#define UART_LITTLE_ENDIAN    0
#define UART_BIG_ENDIAN       1
#endif  // XC16 || __XC8

#define UART_BAD         0xff

struct uart_data {
    uint8_t            tx_pin;
    uint8_t            rx_pin;
    uint8_t            uart;
    uint16_t           uart_mode;
    uint16_t           baud;
    void               (*tx_finished)(void *);
    void               (*process_rx_char)(uint8_t, uint8_t);
};

/**
 * @brief uart_calculate_mode Calculates the bit field for the mode of operation
 *        given the UART configuration options.
 * 
 * @param mode  The calculated bitfield for the required mode of operation. 
 * 
 * @param databits Number of databits required. Either UART_8_DATABITS or 
 * UART_9_DATABITS
 *
 * @param parity Either #define UART_PARITY_NONE, UART_PARITY_ODD or 
 * UART_PARITY_EVEN
 *
 * @param stopbits Either UART_ONE_STOP_BIT or UART_TWO_STOP_BITS
 * 
 * @param rx_idle_level Either UART_IDLE_LOW or UART_IDLE_HIGH
 *
 * @return Either ERR_BAD_INPUT_PARAMETER or SUCCESS 
 */
extern result_t uart_calculate_mode(uint16_t *mode, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level);

extern void     uart_init(void);
extern result_t uart_reserve(struct uart_data *data);
extern result_t uart_release(struct uart_data *data);
extern result_t uart_tx_buffer(struct uart_data *data, uint8_t *buffer, uint16_t len);
extern result_t uart_tx_char(struct uart_data *data, char ch);

#endif // ES_LIB_UART_H

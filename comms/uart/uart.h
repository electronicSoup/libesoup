/**
 *
 * \file es_lib/comms/uart.h
 *
 * UART functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
#define PARITY_NONE                 0
#define PARITY_ODD                  1
#define PARITY_EVEN                 2

#define ONE_STOP_BIT                1
#define TWO_STOP_BITS               2

#define IDLE_LOW                    0
#define IDLE_HIGH                   1
#ifdef MCP
#define LITTLE_ENDIAN               0
#define BIG_ENDIAN                  1
#endif  // ifdef MCP

#define UARTEN_MASK      0x8000
#define USIDL_MASK       0x2000
#define IREN_MASK        0x1000
#define RTSMD_MASK       0x0800
#define UEN1_MASK        0x0200
#define UEN0_MASK        0x0100
#define WAKE_MASK        0x0080
#define LPBACK_MASK      0x0040
#define ABAUD_MASK       0x0020
#define RXINV_MASK       0x0010
#define BRGH_MASK        0x0008
#define PDSEL1_MASK      0x0004
#define PDSEL0_MASK      0x0002
#define STSEL_MASK       0x0001

#define NUM_UARTS        3
#define UART_2           0x00
#define UART_3           0x01
#define UART_4           0x02
#define UART_BAD         0xff

struct uart_data {
    uint8_t            tx_pin;
    uint8_t            rx_pin;
    uint8_t            uart;
    uint16_t           uart_mode;
    uint16_t           baud;
    void        (*tx_finished)(void *);
    void        (*process_rx_char)(uint8_t, uint8_t);
};


extern result_t uart_calculate_mode(uint16_t *, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level);
extern void     uart_init(void);
extern result_t uart_reserve(struct uart_data *data);
extern result_t uart_release(struct uart_data *data);
extern result_t uart_tx_buffer(struct uart_data *data, uint8_t *buffer, uint16_t len);
extern result_t uart_tx_char(struct uart_data *data, char ch);

#endif // ES_LIB_UART_H

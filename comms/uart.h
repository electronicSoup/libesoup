/**
 *
 * \file es_lib/comms/uart.h
 *
 * UART functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */
#ifndef ES_LIB_UART_H
#define ES_LIB_UART_H

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
    u8            tx_pin;
    u8            rx_pin;
    u8            uart;
    u16           uart_mode;
    u16           baud;
    void        (*tx_finished)(u8 channel);
    void        (*process_rx_char)(u8, u8);
};


extern u16      uart_calculate_mode(u8 databits, u8 parity, u8 stopbits, u8 rx_idle_level);
extern void     uart_init(void);
extern result_t uart_reserve(struct uart_data *data);
extern result_t uart_release(struct uart_data *data);
extern result_t uart_tx(struct uart_data *data, u8 *buffer, u16 len);

#endif // ES_LIB_UART_H

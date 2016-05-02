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

typedef struct {
    u8            tx_pin;
    u8            rx_pin;
    u8            uart;
    u16           uart_mode;
    u16           baud;
    void        (*tx_finished)(void);
    void        (*process_rx_char)(u8);
} uart_data;


extern void     uart_init(void);
extern result_t uart_reserve(uart_data *data);
extern result_t uart_release(uart_data *data);
extern result_t uart_tx(uart_data *data, u8 *buffer, u16 len);

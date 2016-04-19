/**
 *
 * \file es_lib/utils/uart_2.h
 *
 * Function prototypes for using a second uart port.
 *
 * The first uart port is used by the logger. See es_lib/logger
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
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
 */

extern void uart_2_init(void (*line_fn)(u8 *line, u16 len));
extern void uart_2_poll(void);
extern void uart_2_putchar(u8 ch);
extern void uart_2_printf(char *string);
extern void uart_2_tx_data(u8 *data, u16 len);

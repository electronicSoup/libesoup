/**
 *
 * \file libesoup/morse/morse.h
 *
 * Timer function prototypes of the electronicSoup Cinnamon Bun
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
 */
#include "libesoup_config.h"

#if defined (MORSE_RX)
extern void morse_rx_init(void (*process_string)(char *));
extern void morse_rx_on(void);
extern void morse_rx_off(void);
#endif

#if defined (MORSE_TX)
extern void morse_tx_init(void (*on)(void), void (*off)(void));
extern void morse_tx(char *);
#endif

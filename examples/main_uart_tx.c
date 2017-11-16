/*
 * File:   main_uart_tx.c
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
 */
#include "libesoup_config.h"

#include "libesoup/comms/uart/uart.h"

static void tx_finished(void *);

int main(void)
{
	result_t         rc;
	struct uart_data uart;
	
	libesoup_init();

	uart.tx_pin = RD0;
	rc = uart_calculate_mode(&uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	uart.baud = 9600;
	uart.tx_finished = tx_finished;

	if(rc != SUCCESS) {
		// Todo - Error condition
	}
	
	/*
	 * Reserve a UART channel for our use
	 */
	rc = uart_reserve(&uart);
		
        while(1) {
        }
        return 0;
}

static void tx_finished(void *data)
{
	
}

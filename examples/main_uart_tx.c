/**
 * File:   main_uart_tx.c
 *
 * Copyright 2017-2018 electronicSoup Limited
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

#include "libesoup/timers/delay.h"
#include "libesoup/comms/uart/uart.h"

static void tx_finished(void *);

int main(void)
{
	result_t           rc;
	struct uart_data   uart;
	uint8_t            buffer[] = "Hello World\n\r";
	
	rc = libesoup_init();

#if defined(__dsPIC33EP256MU806__)	
	uart.tx_pin = RD3;
	uart.rx_pin = INVALID_PIN;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	uart.tx_pin = RB0;
#elif defined(__18F4585)
	/*
	 * There is only one option for the uart tx pin on this uC
	 */
#endif
	rc = uart_calculate_mode(&uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	if(rc < 0) {
		// Todo - Error condition
	}

	uart.baud = 9600;
	uart.tx_finished = tx_finished;
	
	/*
	 * Reserve a UART channel for our use
	 */
	rc = uart_reserve(&uart);
	if(rc < 0) {
		// Todo - Error condition
	}

	delay(Seconds, 10);

	/*
	 * Attempt transmission of a string
	 */
	rc = uart_tx_buffer(&uart, buffer, 13);
	if(rc < 0) {
		// Todo - Error condition
	}

	/*
	 * Release the uart channel we're finished with it
	 */
	rc =  uart_release(&uart);
	if(rc < 0) {
		// Todo - Error condition
	}

        while(1) {
		Nop();
        }
        return 0;
}

static void tx_finished(void *data)
{
	
}

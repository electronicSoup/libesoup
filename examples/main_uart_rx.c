/**
 * @file:   libesoup/examples/main_uart_rx.c
 * 
 * @author John Whitmore
 * 
 * @brief Example main to test UART receive functionality
 *
 * Copyright 2018 electronicSoup Limited
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

#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/delay.h"
#include "libesoup/comms/uart/uart.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MAIN";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

static struct uart_data   uart;

void rx_char(uint8_t ch)
{
	LOG_D("char %c\n\r");
}

int main(void)
{
	result_t           rc;
	uint8_t            buffer[] = "Hello World\n\r";

	/*
	 * Before anything else have to initialise the library
	 */
	rc = libesoup_init();
	
	LOG_D("UART TX\n\r");
	/*
	 * Set our GPIO Pins for the test board
	 */
#if defined(__dsPIC33EP256MU806__)	
	rc = gpio_set(RD0, GPIO_MODE_DIGITAL_OUTPUT, 0);
	rc = gpio_set(RD1, GPIO_MODE_DIGITAL_OUTPUT, 0);
	rc = gpio_set(RD2, GPIO_MODE_DIGITAL_INPUT, 0);
	rc = gpio_set(RD3, GPIO_MODE_DIGITAL_OUTPUT, 0);
	rc = gpio_set(RD4, GPIO_MODE_DIGITAL_INPUT, 0);
#endif
	
	/*
	 * First calculate the mode bits, given the ubitquious 8N1 configuration
	 */
	rc = uart_calculate_mode(&uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	if(rc < 0) {
		// Error condition
		LATDbits.LATD0 = 1;
		return(rc);
	}

	/*
	 * Now fill in the struct uart_data structure with the required 
	 * configuration
	 */
#if defined(__dsPIC33EP256MU806__)	
	uart.tx_pin = RD3;
	uart.rx_pin = RG8;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	uart.tx_pin = RB0;
#elif defined(__18F4585)
	/*
	 * There is only one option for the uart tx pin on this uC
	 */
#endif
	uart.baud = 19200;
	uart.process_rx_char = rx_char;
	
	/*
	 * Reserve a UART channel for our use
	 */
	rc = uart_reserve(&uart);
	if(rc < 0) {
		LATDbits.LATD0 = 1;
		return(rc);
	}

	rc = delay(Seconds, 10);
	if(rc < 0) {
		LATDbits.LATD0 = 1;
		return(rc);
	}

	/*
	 * Attempt transmission of a string
	 */
	rc = uart_tx_buffer(&uart, buffer, 13);
	if(rc < 0) {
		LATDbits.LATD0 = 1;
		return(rc);
	}

        while(1) {
		Nop();
        }
        return(0);
}

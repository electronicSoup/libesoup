/**
 *
 * \file libesoup/logger/serial_port.c
 *
 * Functions for initialisation of the Serial Port.
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
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define DEBUG_FILE TRUE
#define TAG "serial"

#include "libesoup_config.h"

#include "libesoup/logger/serial_log.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

#ifndef SYS_SERIAL_LOGGING_BAUD
#error libesoup_config.h file should define the SYS_SERIAL_LOGGING_BAUD
#endif

#ifndef ES_LINUX
#if defined(__18F2680) || defined(__18F4585)
#ifndef SYS_UART_TX_BUFFER_SIZE
#error libesoup_config.h should define SYS_UART_TX_BUFFER_SIZE (see libesoup/examples/libesoup_config.h)
#endif
#else
#ifndef SERIAL_LOGGING_TX_PIN
#error libesoup_config.h should include a board file which defines SERIAL_LOGGING_TX_PIN (see libesoup/examples/libesoup_config.h)
#endif
#endif

#ifndef SYS_CLOCK_FREQ
#error libesoup_config.h file should define the SYS_CLOCK_FREQ
#endif
#endif

#if defined(XC16) || defined(__XC8)
#include "libesoup/comms/uart/uart.h"
#elif defined (ES_LINUX)
#include <stdio.h>
#endif // if defined (ES_LINUX)

static uint8_t printable(uint8_t ch);

/*
 * Declaration of the data structure being used to manage UART connection.
 * Static to this file only!
 */
static struct uart_data serial_uart;

//#if defined(__18F2680) || defined(__18F4585)
///*
// * Definitions for the Transmit Circular buffer. Calls to putchar will load
// * up this circular buffer and the UASRT serial port will empty it.
// */
//static uint8_t tx_circular_buffer[SYS_UART_TX_BUFFER_SIZE];
//
//static uint16_t tx_write_index = 0;
//static uint16_t tx_read_index = 0;
//static uint16_t tx_buffer_count = 0;
//#endif // (__18F2680) || (__18F4585)

/**
 * \fn serial_logging_init()
 *
 * \brief Initialisation function for serial logging
 *
 * The libesoup_config.h file should define serial port pin orientation, and include the 
 * board file which defines the pins being used by the serial port.
 */
result_t serial_logging_init(void)
{
        result_t rc;
        
#if defined(XC16) || defined(__XC8)
	/*
	 * Serial Port pin configuration should be defined
	 * in include file libesoup_config.h
	 */
#ifdef SERIAL_LOGGING_RX_ENABLE
        SERIAL_LOGGING_RX_DDR = INPUT_PIN;
	UART_1_RX = SERIAL_LOGGING_RX_PIN;
	IEC0bits.U1RXIE = 1;
#endif // SERIAL_LOGGING_RX_ENABLE
        /*
         * Reserve a uart for the RS232 Comms
         */
        serial_uart.baud = SYS_SERIAL_LOGGING_BAUD;
        serial_uart.tx_pin = SERIAL_LOGGING_TX_PIN;
        serial_uart.rx_pin = SERIAL_LOGGING_RX_PIN;
        rc = uart_calculate_mode(&serial_uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
        if(rc != SUCCESS) {
                return(rc);
        }                

        rc =  uart_reserve(&serial_uart);
        if(rc != SUCCESS) {
                return(rc);
        }
#endif // ifdef XC16 || __XC8

        /*
         * Call uart_tx_buffer to clear XC8 compiler warning
         */
//        printf("\n\r\n\r");
        return(SUCCESS);
}

#if defined(XC16) || defined(__XC8)
void serial_log(const char * tag, const char * f, ...)
{
	result_t  rc;
	char     *ptr;

	/*
	 * Print the tag field
	 */
	ptr = (char *)tag;
	
	while(*ptr) {
		if(printable(*ptr)) {
			rc = uart_tx_char(&serial_uart, *ptr);
		}
		
		ptr++;
	}
#if 0
	/*
	 * Next print the format string
	 */
	ptr = (char *)f;
	while(*ptr) {
		
		if(*ptr == '%') {
			/*
			 * Format specifier
			 */
		}
		
		if(printable(*ptr)) {
			rc = uart_tx_char(&serial_uart, *ptr);
		}
		
		ptr++;
	}
#endif	
	
}
#endif // defined(XC16) || defined(__XC8)

result_t serial_logging_exit(void)
{
        return(uart_release(&serial_uart));
}

static uint8_t printable(uint8_t ch)
{
	if(  (ch >= '!' && ch <= '~')
	   ||(ch == '\n')
	   ||(ch == '\r') ) {
		return(TRUE);
	} else {
		return(FALSE);
	}
}

#if defined(__18F2680) || defined(__18F4585)
/**
 * putch: Microchip's printf() implementation calls this function
 *
 * @param character
 */
void putch(char character)
{
        result_t rc = SUCCESS;

        rc = uart_tx_char(&serial_uart, character);
}
#endif // (__18F2680) || (__18F4585)


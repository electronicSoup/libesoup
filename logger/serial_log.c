/**
 *
 * @file libesoup/logger/serial_log.c
 *
 * @author John Whitmore
 *
 * @brief Functions for initialisation of the Logging Serial Port.
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
 * http://www.geeksforgeeks.org/implement-itoa/
 */
#include <stdarg.h>   // Required for vargs
#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING

#define DEBUG_FILE
static const char  __attribute__((unused)) *TAG ="SERIAL_LOG";

#include "libesoup/errno.h"
#include "libesoup/comms/uart/uart.h"
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
#ifndef BRD_SERIAL_LOGGING_TX_PIN
#error libesoup_config.h should include a board file which defines BRD_SERIAL_LOGGING_TX_PIN (see libesoup/examples/libesoup_config.h)
#endif
#endif

#if !(defined(SYS_UART1) || defined(SYS_UART2) || defined(SYS_UART3) || defined(SYS_UART4))
#error libesoup_config.h file should define the SYS_UART required for serial logging!
#endif

#endif

#if defined(XC16) || defined(__XC8)
#include "libesoup/comms/uart/uart.h"
#elif defined (ES_LINUX)
#include <stdio.h>
#endif // if defined (ES_LINUX)

#define LEVEL_STRING_LEN  2
static const char debug_string[LEVEL_STRING_LEN + 1]   = "D-";
static const char info_string[LEVEL_STRING_LEN + 1]    = "I-";
static const char warning_string[LEVEL_STRING_LEN + 1] = "W-";
static const char error_string[LEVEL_STRING_LEN + 1]   = "E-";

/*
 * Local helper functions
 */
#ifdef XC16
static result_t es_printf(const char * fmt, va_list args);
#endif // #ifdef XC16
static uint8_t *itoa(uint16_t num, uint8_t *str, uint8_t base);
#ifdef XC16
static uint8_t *itoa32bit(uint32_t num, uint8_t *str, uint8_t base);
#endif
static uint16_t strlen(char *string);
static void reverse(uint8_t str[], uint16_t length);

/*
 * Declaration of the data structure being used to manage UART connection.
 * Static to this file only! A pointer will be passed to uart module.
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
	uint16_t delay;

#if defined(XC16) || defined(__XC8)
	/*
	 * Serial Port pin configuration should be defined
	 * in include file libesoup_config.h
	 *
         * Reserve a uart for the RS232 Comms
         */
        serial_uart.baud = SYS_SERIAL_LOGGING_BAUD;
        serial_uart.tx_pin = BRD_SERIAL_LOGGING_TX_PIN;
#ifdef SYS_SERIAL_LOGGING_RX_ENABLE
        serial_uart.rx_pin = BRD_SERIAL_LOGGING_RX_PIN;
#else
        serial_uart.rx_pin = INVALID_GPIO_PIN;
#endif
	/*
	 * Get the UART Module to calculate the mode of operation required for
	 * the settings we want. This mode is filled into the uart request
	 * data structure.
	 */
	rc = uart_calculate_mode(&serial_uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	RC_CHECK

        rc =  uart_reserve(&serial_uart);
	RC_CHECK
#endif // ifdef XC16 || __XC8

	/*
	 * Short delay to allow the line to stabilise before sending the
	 * first character on the channel
	 */
	for (delay = 0; delay < 0x100; delay++) Nop();
#ifdef XC16
	rc = serial_printf("\n\r\n\r");
	RC_CHECK
#endif
        return(0);
}

#ifdef SYS_TEST_BUILD
uint16_t serial_buffer_count(void)
{
	return(uart_tx_buffer_count(&serial_uart));
}
#endif

#if defined(XC16)
result_t serial_log(uint8_t level, const char *tag, const char *fmt, ...)
{
	result_t  rc;
	va_list   args;

	va_start(args, fmt);

	/*
	 * Print the log level
	 */
	switch(level) {
	case LOG_DEBUG:
		rc = serial_printf(debug_string);
		RC_CHECK
		break;

	case LOG_INFO:
		rc = serial_printf(info_string);
		RC_CHECK
		break;

	case LOG_WARNING:
		rc = serial_printf(warning_string);
		RC_CHECK
		break;

	case LOG_ERROR:
		rc = serial_printf(error_string);
		RC_CHECK
		break;
	}

	/*
	 * Print the tag field
	 */
	rc = serial_printf("%s:", tag);
	RC_CHECK

	rc = es_printf(fmt, args);
	RC_CHECK
	va_end(args);

	return(0);
}
#elif defined(__XC8)
result_t serial_log(const char* fmt, ...)
{
	result_t  rc;
	char     *ptr;
	va_list   args;
	uint16_t  i;
	uint8_t   buf[256];
	uint8_t  *string;

	va_start(args, fmt);

	/*
	 * print the format string
	 */
	ptr = (char *)fmt;
	while(*ptr) {

		if(*ptr != '%') {
			rc = uart_tx_char(&serial_uart, *ptr);
			RC_CHECK
		} else {
			/*
			 * Format specifier
			 */
			switch(*++ptr) {
			case '%' :
				rc = uart_tx_char(&serial_uart, *ptr);
				RC_CHECK
				break;

			case 'd':
				i = va_arg(args, uint16_t);
				string = itoa(i, buf, 10);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 's':
				string = va_arg(args, uint8_t *);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 'x':
				i = va_arg(args, uint16_t);
				string = itoa(i, buf, 16);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 'l':
				switch(*++ptr) {
				case 'd':
					i = va_arg(args, uint32_t);
					string = itoa(i, buf, 10);
					rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
					RC_CHECK
					break;

				case 'x':
					i = va_arg(args, uint32_t);
					string = itoa(i, buf, 16);
					rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
					RC_CHECK
					break;
				}
				break;
			}
		}
		ptr++;
	}
}
#endif // defined(XC16) || defined(__XC8)

#if defined(XC16)
result_t serial_printf(const char * fmt, ...)
{
	result_t  rc;
	char     *ptr;
	va_list   args;
	uint16_t  i;
	uint32_t  li;
	uint8_t   buf[256];
	uint8_t  *string;

	va_start(args, fmt);

	ptr = (char *)fmt;

	while(*ptr) {

		if(*ptr != '%') {
			rc = uart_tx_char(&serial_uart, *ptr);
			RC_CHECK
		} else {
			/*
			 * Format specifier
			 */
			switch(*++ptr) {
			case '%' :
				rc = uart_tx_char(&serial_uart, *ptr);
				RC_CHECK
				break;

			case 'd':
				i = va_arg(args, uint16_t);
				string = itoa((uint32_t)i, buf, 10);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 's':
				string = va_arg(args, uint8_t *);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 'x':
				i = va_arg(args, uint16_t);
				string = itoa(i, buf, 16);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 'l':
				switch(*++ptr) {
				case 'd':
					li = va_arg(args, uint32_t);
					*buf = 0;
					string = itoa32bit(li, buf, 10);
					rc = uart_tx_buffer(&serial_uart, buf, strlen((char*)buf));
					RC_CHECK
					break;

				case 'x':
					li = va_arg(args, uint32_t);
					*buf = 0;
					string = itoa32bit(li, buf, 16);
					rc = uart_tx_buffer(&serial_uart, buf, strlen((char*)buf));
					RC_CHECK
					break;
				}
				break;
			}
		}
		ptr++;
	}

	va_end(args);
	return(0);
}
#endif // #if defined(XC16)

#ifdef XC16
static result_t es_printf(const char * fmt, va_list args)
{
	result_t  rc;
	char     *ptr;
	uint16_t  i;
	uint32_t  li;
	uint8_t   buf[256];
	uint8_t  *string = NULL;

	ptr = (char *)fmt;

	while(*ptr) {

		if(*ptr != '%') {
			rc = uart_tx_char(&serial_uart, *ptr);
			RC_CHECK
		} else {
			/*
			 * Format specifier
			 */
			switch(*++ptr) {
			case '%' :
				rc = uart_tx_char(&serial_uart, *ptr);
				RC_CHECK
				break;

			case 'c':
				i = va_arg(args, uint8_t);
				string[0] = i;
				string[1] = 0;
				rc = uart_tx_buffer(&serial_uart, string, 1);
				RC_CHECK
				break;

			case 'd':
				i = va_arg(args, uint16_t);
				string = itoa((uint32_t)i, buf, 10);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 's':
				string = va_arg(args, uint8_t *);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 'x':
				i = va_arg(args, uint16_t);
				string = itoa(i, buf, 16);
				rc = uart_tx_buffer(&serial_uart, string, strlen((char*)string));
				RC_CHECK
				break;

			case 'l':
				switch(*++ptr) {
				case 'd':
					li = va_arg(args, uint32_t);
					*buf = 0;
					string = itoa32bit(li, buf, 10);
					rc = uart_tx_buffer(&serial_uart, buf, strlen((char*)buf));
					RC_CHECK
					break;

				case 'x':
					li = va_arg(args, uint32_t);
					*buf = 0;
					string = itoa32bit(li, buf, 16);
					rc = uart_tx_buffer(&serial_uart, buf, strlen((char*)buf));
					RC_CHECK
					break;
				}
				break;
			}
		}
		ptr++;
	}
	return(0);
}
#endif // #if defined(XC16)

result_t serial_logging_exit(void)
{
        return(uart_release(&serial_uart));
}

static uint8_t *itoa(uint16_t num, uint8_t *str, uint8_t base)
{
	uint16_t rem;                 // successive remainder
	uint16_t i = 0;               // Index into the resulting string
	boolean isNegative = FALSE;

	/* Handle 0 explicitely, otherwise empty string is printed for 0 */
	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	// Negative numbers are handled only with base 10.
	if (num < 0 && base == 10) {
		isNegative = TRUE;
		num = -num;          // Process number as positive add negative at the end.
	}

	// Process individual digits
	while (num != 0) {
		rem = num % base;
		str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
		num = num/base;
	}

	// If number is negative, append '-'
	if (isNegative)
		str[i++] = '-';

	str[i] = '\0'; // Append string terminator

	// Reverse the string
	reverse(str, i);

	return str;
}

#ifdef XC16
static uint8_t *itoa32bit(uint32_t num, uint8_t *str, uint8_t base)
{
	uint32_t rem;                // successive remainder
	uint16_t i = 0;              // Index into the output string
	boolean isNegative = FALSE;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	// Negative numbers are handled only in base 10.
	if (num < 0 && base == 10) {
		isNegative = TRUE;
		num = -num;            // Process as positive add negative at the end.
	}

	// Process individual digits
	while (num != 0) {
		rem = num % base;
		str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0';
		num = num/base;
	}

	// If number is negative, append '-'
	if (isNegative)
		str[i++] = '-';

	str[i] = '\0'; // Append string terminator

	// Reverse the string
	reverse(str, i);

	return str;
}
#endif // XC16

static uint16_t strlen(char *string)
{
	uint16_t len = 0;

	while(*string++) len++;
	return(len);
}

static void reverse(uint8_t str[], uint16_t length)
{
	uint8_t tmp;
	uint16_t start = 0;
	uint16_t end = length -1;
	while (start < end) {
		tmp = str[end];
		str[end] = str[start];
		str[start] = tmp;
		start++;
		end--;
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
        result_t rc;

        rc = uart_tx_char(&serial_uart, character);
}
#endif // (__18F2680) || (__18F4585)

#endif // SYS_SERIAL_LOGGING

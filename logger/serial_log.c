/**
 *
 * \file es_lib/logger/serial_port.c
 *
 * Functions for initialisation of the Serial Port.
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
#include <stdarg.h>
#include <stdio.h>

#define DEBUG_FILE TRUE
#define TAG "serial"

#include "system.h"
#include "es_lib/logger/serial_log.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif



#ifndef SYS_SERIAL_LOGGING_BAUD
#error system.h file should define the SYS_SERIAL_LOGGING_BAUD
#endif

#ifndef ES_LINUX
#if defined(__18F2680) || defined(__18F4585)
#ifndef SYS_USART_TX_BUFFER_SIZE
#error system.h should define SYS_USART_TX_BUFFER_SIZE (see es_lib/examples/system.h)
#endif
#else
#ifndef SERIAL_LOGGING_TX_DDR
#error system.h should include a board file which defines SERIAL_LOGGING_TX_DDR (see es_lib/examples/system.h)
#endif
#endif

#ifndef SYS_CLOCK_FREQ
#error system.h file should define the SYS_CLOCK_FREQ
#endif
#endif

#if defined (MCP)
#include "es_lib/comms/uart/uart.h"
#elif defined (ES_LINUX)
#include <stdio.h>
#endif // if defined (ES_LINUX)

//static struct uart_data serial_uart;


#if defined(__18F2680) || defined(__18F4585)
/*
 * Definitions for the Transmit Circular buffer. Calls to putchar will load
 * up this circular buffer and the UASRT serial port will empty it.
 */
static uint8_t tx_circular_buffer[SYS_USART_TX_BUFFER_SIZE];

static uint16_t tx_write_index = 0;
static uint16_t tx_read_index = 0;
static uint16_t tx_buffer_count = 0;
#endif // (__18F2680) || (__18F4585)

/**
 * \fn _U1RXInterrupt()
 *
 * \brief Interrupt Service Routine for received characters from UART 1
 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
void __attribute__((__interrupt__, __no_auto_psv__)) _U1RXInterrupt(void)
#elif defined(__dsPIC33EP256MU806__)
void _ISR __attribute__((__no_auto_psv__)) _U1RXInterrupt(void)
#endif
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
{
	uint8_t ch;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "_U1RXInterrupt\n\r");
#endif
	while (U1STAbits.URXDA) {
		ch = U1RXREG;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Rx*0x%x*\n\r", ch);
#endif
	}

	IFS0bits.U1RXIF = 0;
}
#endif

/**
 * \fn serial_init()
 *
 * \brief Initialisation function for serial logging
 *
 * The system.h file should define serial port pin orientation, and include the 
 * board file which defines the pins being used by the serial port.
 */
void serial_logging_init(void)
{
#ifdef MCP
	/*
	 * CinnamonBun is running a PIC24FJ256GB106 or dsPIC33 processor
	 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
	/*
	 * Serial Port pin configuration should be defined
	 * in include file system.h
	 */
#ifdef SERIAL_LOGGING_RX_ENABLE
        SERIAL_LOGGING_RX_DDR = INPUT_PIN;
	UART_1_RX = SERIAL_LOGGING_RX_PIN;
	IEC0bits.U1RXIE = 1;
#endif
        /*
         * The system.h file should define the Serial Logging pin orientation
         * (either SYS_SERIAL_PORT_GndTxRx or SYS_SERIAL_PORT_GndRxTx) which is
         * then used in the board header file included from the system.h file.
         * The board file defines the pins used by the serial port here. 
         */
        SERIAL_LOGGING_TX_DDR = OUTPUT_PIN;
	SERIAL_LOGGING_TX = UART_1_TX;
	U1MODE = 0x8800;
	U1STA = 0x0410;

	/*
	 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
	 *
	 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
	 *
	 * UxBRG = ((CLOCK/SERIAL_BAUD)/16) -1
	 *
	 */
	U1BRG = ((SYS_CLOCK_FREQ / SYS_SERIAL_LOGGING_BAUD) / 16) - 1;

#elif defined(__18F2680) || defined(__18F4585)
	/*
	 * Analogue Guage is running a PIC18F2680 processor
	 */
	uint8_t baud;

	/*
	 * Initialise the TX Circular buffer
	 */
	tx_write_index = 0;
	tx_read_index = 0;
	tx_buffer_count = 0;
        
	TRISCbits.TRISC6 = 0;
#if defined(ENABLE_USART_RX)
	TRISCbits.TRISC7 = 1;
#endif
	TXSTAbits.TXEN = 1;    // Transmitter enabled
	TXSTAbits.SYNC = 0;    // Asynchronous mode
	TXSTAbits.BRGH = 0;    // High Baud Rate Select bit

#if defined(ENABLE_USART_RX)
	RCSTAbits.CREN = 1;    // Enable the Receiver
#else
	RCSTAbits.CREN = 0;    // Disable the Receiver
#endif
	RCSTAbits.SPEN = 1;

	BAUDCONbits.BRG16 = 0; // 16-bit Baud Rate Register Enable bit

	baud = ((SYS_CLOCK_FREQ / SYS_SERIAL_LOGGING_BAUD) / 64 ) - 1;

	SPBRG = baud;

	PIE1bits.TXIE = 0;
	PIR1bits.TXIF = 0;
#if defined(ENABLE_USART_RX)
	PIR1bits.RCIF = 0;
	PIE1bits.RCIE = 1;
#endif // ENABLE_EUSART_RX

	RCSTAbits.SPEN = 1;
#endif // (__18F2680) || (__18F4585)
#endif // ifdef MCP

        printf("\n\r\n\r");
}

#if defined(__18F2680) || defined(__18F4585)
void pic18f_serial_isr(void)
{
#if defined(ENABLE_USART_RX)
	uint8_t data;
#endif // (ENABLE_UASAT_RX)

	if(PIR1bits.TXIF) {
		/*
		 * The TXIF Interrupt is cleared by writing to TXREG it
		 * cannot be cleared by SW directly.
		 */
		if(tx_buffer_count > 0) {
			TXREG = tx_circular_buffer[tx_read_index];
			tx_read_index = ++tx_read_index % SYS_USART_TX_BUFFER_SIZE;
			tx_buffer_count--;
		} else {
			PIE1bits.TXIE = 0;
		}
	}
#if defined(ENABLE_USART_RX)
	if(PIR1bits.RCIF) {
		data = RCREG;
	}
#endif
}
#endif // (__18F2680) || (__18F4585)

#if defined(__18F2680) || defined(__18F4585)
/**
 * putch: Microchip's printf() implementation calls this function
 *
 * @param character
 */
void putch(char character)
{
	if(tx_buffer_count < SYS_USART_TX_BUFFER_SIZE) {
		tx_circular_buffer[tx_write_index] = character;
		tx_write_index = ++tx_write_index % SYS_USART_TX_BUFFER_SIZE;
		tx_buffer_count++;
		PIE1bits.TXIE = 1;
	}
}
#endif // (__18F2680) || (__18F4585)

void es_printf(const char *fmt, ...)
{
#if (SYS_LOG_LEVEL != NO_LOGGING)
//        result_t  rc;
        const char     *ptr;
        
        ptr = fmt;
        
        while(*ptr) {
//                rc = uart_tx_char(&serial_uart, *ptr++);
        }
#endif
}

void log_d(const char *tag, const char *fmt, ...)
{
        va_list arguments;           

        /* Initializing arguments to store all values after num */
        va_start ( arguments, fmt );
#if defined(MCP)
        es_printf("D :%s ", tag);
        es_printf(fmt, arguments);
#elif defined(ES_LINUX)
        printf("D :%s ", tag);
        printf(fmt, arguments);
#endif
}

void log_i(const char *tag, const char *fmt, ...)
{
        va_list arguments;                     

        /* Initializing arguments to store all values after num */
        va_start ( arguments, fmt );
#if defined(MCP)
        es_printf("I :%s ", tag);
        es_printf(fmt, arguments);
#elif defined(ES_LINUX)
        printf("I :%s ", tag);
        printf(fmt, arguments);
#endif
}

void log_w(const char *tag, const char *fmt, ...)
{
        va_list arguments;                     

        /* Initializing arguments to store all values after num */
        va_start ( arguments, fmt );
#if defined(MCP)
        es_printf("W :%s ", tag);
        es_printf(fmt, arguments);
#elif defined(ES_LINUX)
        printf("W :%s ", tag);
        printf(fmt, arguments);
#endif
}

void log_e(const char *tag, const char *fmt, ...)
{
        va_list arguments;                     

        /* Initializing arguments to store all values after num */
        va_start ( arguments, fmt );
#if defined(MCP)
        es_printf("E :%s ", tag);
        es_printf(fmt, arguments);
#elif defined(ES_LINUX)
        printf("E :%s ", tag);
        printf(fmt, arguments);
#endif
}

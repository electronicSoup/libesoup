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
#define DEBUG_FILE
#define TAG "serial"

#include "system.h"

#include "es_lib/logger/serial_port.h"
#include "es_lib/logger/serial_log.h"


#if defined(__18F2680) || defined(__18F4585)
/*
 * Definitions for the Transmit Circular buffer. Calls to putchar will load
 * up this circular buffer and the UASRT serial port will empty it.
 */
static u8 tx_circular_buffer[USART_TX_BUFFER_SIZE];

static u16 tx_write_index = 0;
static u16 tx_read_index = 0;
static u16 tx_buffer_count = 0;
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
	u8 ch;
	LOG_D("_U1RXInterrupt\n\r");
	while (U1STAbits.URXDA) {
		ch = U1RXREG;

		LOG_D("Rx*0x%x*\n\r", ch);
	}

	IFS0bits.U1RXIF = 0;
}
#endif

/**
 * \fn serial_init()
 *
 * \brief Initialisation function for serial logging
 *
 * Initialised the processor registers for serial logging on UART 1
 */
void serial_init(void)
{
	/*
	 * CinnamonBun is running a PIC24FJ256GB106 processor
	 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
	/*
	 * Serial Port pin configuration should be defined
	 * in include file system.h
	 */
#ifdef USART_RX_ENABLE
        SERIAL_LOGGING_RX_DDR = INPUT_PIN;
	UART_1_RX = SERIAL_LOGGING_RX_PIN;
	IEC0bits.U1RXIE = 1;
#endif
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
	U1BRG = ((CLOCK_FREQ / SERIAL_LOGGING_BAUD) / 16) - 1;

#elif defined(__18F2680) || defined(__18F4585)
	/*
	 * Analogue Guage is running a PIC18F2680 processor
	 */
	u8 baud;

	/*
	 * Initialise the TX Circular buffer
	 */
	tx_write_index = 0;
	tx_read_index = 0;
	tx_buffer_count = 0;

	TRISCbits.TRISC6 = 0;
	TRISCbits.TRISC7 = 1;

	TXSTAbits.TXEN = 1;    // Transmitter enabled
	TXSTAbits.SYNC = 0;    // Asynchronous mode
	TXSTAbits.BRGH = 0;    // High Baud Rate Select bit

#if defined(ENABLE_USART_RX)
	RCSTAbits.CREN = 1;    // Enable the Receiver
#endif
	RCSTAbits.SPEN = 1;

	BAUDCONbits.BRG16 = 0; // 16-bit Baud Rate Register Enable bit

	baud = ((CLOCK_FREQ / SERIAL_LOGGING_BAUD) / 64 ) - 1;

	SPBRG = baud;

	PIE1bits.TXIE = 0;
	PIR1bits.TXIF = 0;
#if defined(ENABLE_USART_RX)
	PIR1bits.RCIF = 0;
	PIE1bits.RCIE = 1;
#endif // ENABLE_EUSART_RX

	RCSTAbits.SPEN = 1;
#endif // (__18F2680) || (__18F4585)
}

#if defined(__18F2680) || defined(__18F4585)
void pic18f_serial_isr(void)
{
#if defined(ENABLE_USART_RX)
	u8 data;
#endif // (ENABLE_UASAT_RX)

	if(PIR1bits.TXIF) {
		/*
		 * The TXIF Interrupt is cleared by writing to TXREG it
		 * cannot be cleared by SW directly.
		 */
		if(tx_buffer_count > 0) {
			TXREG = tx_circular_buffer[tx_read_index];
			tx_read_index = ++tx_read_index % USART_TX_BUFFER_SIZE;
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
	if(tx_buffer_count < USART_TX_BUFFER_SIZE) {
		tx_circular_buffer[tx_write_index] = character;
		tx_write_index = ++tx_write_index % USART_TX_BUFFER_SIZE;
		tx_buffer_count++;
		PIE1bits.TXIE = 1;
	}
}
#endif // (__18F2680) || (__18F4585)

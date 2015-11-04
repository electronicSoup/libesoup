/**
 *
 * \file es_lib/utils/serial_2.c
 *
 * Functions for using a second serial port.
 *
 * The first serial port is used by the logger. See es_lib/logger
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
#include "system.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "Serial-2"

void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
{
	u8 ch;

	if (U2STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
	}

	while (U2STAbits.URXDA) {
		ch = U2RXREG;
		LOG_D("U2 RX Interrupt %c\n\r", ch);
	}

	if (U2STAbits.OERR) {
		U2STAbits.OERR = 0;   /* Clear the error flag */
	}

	IFS1bits.U2RXIF = 0;
}

void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
{
	LOG_D("U2 TX Interrupt\n\r");
}

void serial_2_init()
{
	/*
	 * CinnamonBun is running a PIC24FJ256GB106 processor
	 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	/*
	 * Serial Port pin configuration should be defined
	 * in include file system.h
	 */
#if defined(SERIAL_2_GndTxRx)
	RPOR0bits.RP0R = 5;
	RPINR19bits.U2RXR = 1;
#elif defined(SERIAL_2_GndRxTx)
	RPINR19bits.U2RXR = 0;
	RPOR0bits.RP1R = 5;
#endif
	U2MODE = 0x8800;
	
	U2STA = 0x0410;

//	U2MODEbits.LPBACK = 1;

	IEC1bits.U2RXIE = 1;
//	IEC1bits.U2TXIE = 1;
	/*
	 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
	 *
	 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
	 *
	 * UxBRG = ((CLOCK/SERIAL_BAUD)/16) -1
	 *
	 */
	U2BRG = ((CLOCK_FREQ / SERIAL_2_BAUD) / 16) - 1;


#endif

	/*
	 * Analogue Guage is running a PIC18F2680 processor
	 */
#if defined(__18F2680) || defined(__18F4585)
//	UINT8 baud;
//
//	/*
//	 * Initialise the TX Circular buffer
//	 */
//	tx_write_index = 0;
//	tx_read_index = 0;
//	tx_buffer_count = 0;
//
//	TRISCbits.TRISC6 = 0;
//	TRISCbits.TRISC7 = 1;
//
//	TXSTAbits.TXEN = 1;    // Transmitter enabled
//	TXSTAbits.SYNC = 0;    // Asynchronous mode
//	TXSTAbits.BRGH = 0;    // High Baud Rate Select bit
//
//#if defined(ENABLE_USART_RX)
//	RCSTAbits.CREN = 1;    // Enagle the Receiver
//#endif
//	RCSTAbits.SPEN = 1;
//
//	BAUDCONbits.BRG16 = 0; // 16-bit Baud Rate Register Enable bit
//
//	baud = ((CLOCK_FREQ / SERIAL_BAUD) / 64 ) - 1;
//
//	SPBRG = baud;
//
//	PIR1bits.TXIF = 0;
//#if defined(ENABLE_USART_RX)
//	PIR1bits.RCIF = 0;
//	PIE1bits.RCIE = 1;
//#endif // ENABLE_EUSART_RX
//
//	RCSTAbits.SPEN = 1;
#endif // (__18F2680) || (__18F4585)
}

void serial_2_putchar(char ch)
{
	if (U2STAbits.UTXBF) {
		LOG_E("Transmit buffer full\n\r");
	} else {
		U2TXREG = ch;
	}
}

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
	IFS1bits.U2TXIF = 0;
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

	U2MODE = 0x8800;
	
	U2STA  = 0x8410;

	IEC1bits.U2RXIE = 1;
	IEC1bits.U2TXIE = 1;

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
}

void serial_2_putchar(char ch)
{
	if (U2STAbits.UTXBF) {
		LOG_E("Transmit buffer full\n\r");
	} else {
		U2TXREG = ch;
	}
}

void serial_2_printf(char *string)
{
	char *ptr;

	LOG_D("serial_2_printf(%s)\n\r", string);

	ptr = string;

	while (*ptr) {
		serial_2_putchar(*ptr++);
	}
}

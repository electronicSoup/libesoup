/**
 *
 * \file es_lib/utils/sleep.c
 *
 * Functionality for sleep
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#define TAG "SLEEP"

#include "es_lib/logger/serial_log.h"

static volatile u8 sleep_over;

#ifdef MCP
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
void _ISR __attribute__((__no_auto_psv__)) _T3Interrupt(void)
{
	IFS0bits.T3IF = 0;
	sleep_over = TRUE;

	/*
	 * Timer is off for the moment.
	 */
	IEC0bits.T3IE = 0;
	T2CONbits.TON = 0;
}
#endif //__PIC24FJ256GB106__

void sleep_seconds(u16 seconds)
{
	u32 timer;

	/*
	 * Initialise Timer 2 for use as the 1.5 Character timer
	 */
	T2CONbits.T32 = 1;      // 16 Bit Timer
	T2CONbits.TCS = 0;      // Internal FOSC/2
	T2CONbits.TCKPS1 = 1;   // Divide by 256
	T2CONbits.TCKPS0 = 1;

	timer = ((u32)((CLOCK_FREQ / 256) * seconds) - 1) ;
	PR2 = (u16)(timer & 0xffff);
	PR3 = (u16)((timer >> 16) & 0xffff);
	TMR2 = 0x00;
	TMR3 = 0x00;

	IEC0bits.T3IE = 1;
	T2CONbits.TON = 1;

	sleep_over = FALSE;

	while(!sleep_over) {
		asm ("CLRWDT");
	}
}
#endif

/**
 *
 * \file es_lib/timers/hw_timers.c
 *
 * Hardware Timer functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
 *******************************************************************************
 *
 */
#define DEBUG_FILE
#define TAG "HW_TIMERS"

#include "system.h"
#include "es_lib/timers/hw_timers.h"
#include "es_lib/logger/serial_log.h"

static void hw_timer_isr(u8 timer);

void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;

	/*
	 * Timer is off for the moment.
	 */
	IEC0bits.T1IE = 0;
	T1CONbits.TON = 0;

	hw_timer_isr(TIMER_1);
}

void _ISR __attribute__((__no_auto_psv__)) _T2Interrupt(void)
{
	IFS0bits.T2IF = 0;

	/*
	 * Timer is off for the moment.
	 */
	IEC0bits.T2IE = 0;
	T2CONbits.TON = 0;

	hw_timer_isr(TIMER_2);
}

void _ISR __attribute__((__no_auto_psv__)) _T3Interrupt(void)
{
	IFS0bits.T3IF = 0;

	/*
	 * Timer is off for the moment.
	 */
	IEC0bits.T3IE = 0;
	T3CONbits.TON = 0;

	hw_timer_isr(TIMER_3);
}

void _ISR __attribute__((__no_auto_psv__)) _T4Interrupt(void)
{
	IFS1bits.T4IF = 0;

	/*
	 * Timer is off for the moment.
	 */
	IEC1bits.T4IE = 0;
	T4CONbits.TON = 0;

	hw_timer_isr(TIMER_4);
}

void _ISR __attribute__((__no_auto_psv__)) _T5Interrupt(void)
{
	IFS1bits.T5IF = 0;

	/*
	 * Timer is off for the moment.
	 */
	IEC1bits.T5IE = 0;
	T5CONbits.TON = 0;

	hw_timer_isr(TIMER_5);
}

static void hw_timer_isr(u8 timer)
{

}

u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void))
{
	LOG_D("start_hw_timer()\n\r");
}


void hw_timer_cancel(u8 timer)
{

}

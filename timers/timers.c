/**
 *
 * \file es_lib/timers/timers.c
 *
 * Timer functionalty for the electronicSoup Cinnamon Bun
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
 *******************************************************************************
 *
 * This file contains the code for creating timers on an electronicSoup 
 * Cinnaom Bun. The API for creating and canceling timers is modeled on the 
 * API used in Linux to that source code can be compiled for both the 
 * Cinnamon Bun and Linux based systems.
 *
 * Linux refers to the interval of time as Jiffies where as in this code it's 
 * refered to as a "tick". This tick interval is defined in the core.h file
 * by the SYSTEM_TICK_ms Macro and is currently set to 5 milli Second. 
 *
 * core.h also includes two macros for calculating real time intervals in 
 * timer tick intervals:
 *
 * #define SECONDS_TO_TICKS(s)  ((s) * (1000 / SYSTEM_TICK_ms))
 * #define MILLI_SECONDS_TO_TICKS(ms) ((ms < SYSTEM_TICK_ms) ? 1 : (ms / SYSTEM_TICK_ms))
 * Both of these utilities should be used for portability, just in case the 
 * timer tick period is changed.
 *
 * To use these timer funcionality the initilaisation function timer_init()
 * must be called and at regular intervals the macro CHECK_TIMERS() must be
 * called. So your main() might look something like:
 *
 * void main(void)
 * {
 *     timer_init();
 *
 *     // Other Initialisation
 *
 *     while(1) {
 *         CHECK_TIMERS()
 *
 *         // Main control loop Code.
 *     }
 * } 
 *
 *
 *
 */
#include "es_lib/core.h"
#include "system.h"
#include <stdio.h>

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "TIMERS"

static UINT16 timer_counter = 0;

volatile BOOL timer_tick = FALSE;

typedef struct {
	BOOL active;
	UINT16 expiry_count;
	expiry_function function;
	union sigval expiry_data;
} sys_timer_t;

#pragma udata
sys_timer_t timers[NUMBER_OF_TIMERS];

void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	TMR1 = 0x00;

	timer_tick = TRUE;
}

void timer_init(void)
{
	BYTE loop;

	/*
	 * Initialise our Data Structures
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		timers[loop].active = FALSE;
		timers[loop].expiry_count = 0;
		timers[loop].function = (expiry_function)NULL;
	}

	// Initialise Timer 1 for use as the 5mS timer
	T1CONbits.TCS = 0;      // Internal FOSC/2
	T1CONbits.TCKPS1 = 0;   // Divide by 8
	T1CONbits.TCKPS0 = 1;

	PR1 = ((CLOCK_FREQ / 8) / 1000) * SYSTEM_TICK_ms;

	TMR1 = 0x00;
	IEC0bits.T1IE = 1;
    
	T1CONbits.TON = 1;
}

void tick(void)
{
	BYTE loop;
	expiry_function function;
	union sigval data;

	timer_tick = FALSE;
	timer_counter++;

	/*
	 * Check for expired timers
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		if (  (timers[loop].active)
		    &&(timers[loop].expiry_count == timer_counter) ) {
			timers[loop].active = FALSE;

			function = timers[loop].function;
			data = timers[loop].expiry_data;

			timers[loop].expiry_count = 0;
			timers[loop].function = (expiry_function)NULL;
			function(loop, data);
		}
	}
}

result_t timer_start(UINT16 ticks,
		     expiry_function function,
		     union sigval data,
		     es_timer *timer)
{
	timer_t loop;

	if(timer->status != INACTIVE) {
		LOG_E("Timer already Active\n\r");
		return(ERR_TIMER_ACTIVE);
	}

	/*
	 * Find the First empty timer
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		if (!timers[loop].active) {
			timers[loop].active = TRUE;

			if( (0xFFFF - timer_counter) > ticks) {
				timers[loop].expiry_count = timer_counter + ticks;
			} else {
				timers[loop].expiry_count = ticks - (0xFFFF - timer_counter);
			}
			timers[loop].function = function;
			timers[loop].expiry_data = data;

			timer->status = ACTIVE;
			timer->timer_id = loop;
			return(SUCCESS);
		}
	}
	LOG_E("No Timers Free");

	return(ERR_NO_RESOURCES);
}

result_t timer_cancel(es_timer *tmr)
{
	if(tmr->status == ACTIVE) {
		if(timers[tmr->timer_id].active) {
			timers[tmr->timer_id].active = FALSE;
			timers[tmr->timer_id].expiry_count = 0;
			timers[tmr->timer_id].function = (expiry_function) NULL;
		}
		tmr->status = INACTIVE;
	}
	return(SUCCESS);
}

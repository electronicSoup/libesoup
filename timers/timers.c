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
 * timers API used in Linux so that source code can be compiled for both the 
 * Cinnamon Bun and Linux based systems.
 *
 * The timer tick is set using the First Timer Interrupt on the PIC24FJ256GB106
 * this timer is setup by the init function and the ISR for Timer_1 then
 * triggers the regular checking of the active timers.
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
#include "system.h"
//#include <stdio.h>
#include "es_lib/timers/timers.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "TIMERS"

static UINT16 timer_counter = 0;

volatile BOOL timer_ticked = FALSE;

/*
 * Data structure for a Timer on the Cinnamon Bun.
 */
typedef struct {
	BOOL active;
	UINT16 expiry_count;
	expiry_function function;
	union sigval expiry_data;
} sys_timer_t;

/*
 * The Cinnamon Bun maintains a table of timers which can be activated
 * by the calling code. The NUMBER_OF_TIMERS, defined in your system.h
 * file defines how many timers the code maintains. 
 *
 * If your project uses a limited number of know timers then you can set 
 * NUMBER_OF_TIMERS to a known value.
 */
#pragma udata
sys_timer_t timers[NUMBER_OF_TIMERS];

/*
 * Timer_1 ISR. To keep ISR short it simply restarts TIMER_1 and sets 
 * the variable "timer_tick" which is should be regularly checked by 
 * the main control loop of your project by calling the CHECK_TIMERS()
 * macro.
 */
void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void)
{
	IFS0bits.T1IF = 0;
	TMR1 = 0x00;

	timer_ticked = TRUE;
}

/*
 * void timer_init(void)
 *
 * Function to initialise the data structures for timers and start 
 * Timer_1 of the PIC 
 *
 */
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

	/*
	 * Initialise Timer 1 for use as the timer tick 
	 */
	T1CONbits.TCS = 0;      // Internal FOSC/2
	T1CONbits.TCKPS1 = 0;   // Divide by 8
	T1CONbits.TCKPS0 = 1;

	PR1 = ((CLOCK_FREQ / 8) / 1000) * SYSTEM_TICK_ms;

	TMR1 = 0x00;
	IEC0bits.T1IE = 1;
    
	T1CONbits.TON = 1;
}

/*
 * void timer_tick(void)
 *
 * Function periodically called to check the state of active timers and check
 * expiry of any of the timers. If a timer has expired after the current tick
 * it's expiry function is called.
 *
 */
void timer_tick(void)
{
	BYTE loop;
	expiry_function function;
	union sigval data;

	timer_ticked = FALSE;
	timer_counter++;

	/*
	 * Check for expired timers
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		if (  (timers[loop].active)
		    &&(timers[loop].expiry_count == timer_counter) ) {

			/*
			 * timer expired so call expiry function.
			 */
			timers[loop].active = FALSE;

			function = timers[loop].function;
			data = timers[loop].expiry_data;

			timers[loop].expiry_count = 0;
			timers[loop].function = (expiry_function)NULL;
			function(loop, data);
		}
	}
}

/*
 * result_t timer_start(UINT16 ticks,
 *                      expiry_function function,
 *                      union sigval data,
 *                      es_timer *timer)
 *
 * Function to start a timer on the system.
 *
 * Input  : UINT16 ticks
 *              The duration of the timer in system timer ticks.
 *              The two convienence macros (SECONDS_TO_TICKS and
 *              MILLI_SECONDS_TO_TICKS) should be used to calculate
 *              this duration to keep code portable in case the
 *              system tick duration is changed from it's current
 *              value.
 *
 * Input  : expiry_function function
 *              The function which is to be called if the timer expires before
 *              it is canceled.
 *
 * Input  : union sigval data
 *              This Input parameter is a data parameter which will be passed
 *              to the given expiry function if the timer expires. The union
 *              is defined in timers.h and can be a value or a pointer.
 *
 * Input/Output  : es_timer *timer
 *              The timer strucure which the function is to operate on. The
 *              structure's timer identifier will be updated depending on which
 *              system timer is allocated to the timer.
 *
 * Return : SUCCESS             Timer created successfully
 *          ERR_TIMER_ACTIVE    Error - Given timer is already active
 *          ERR_NO_RESOURCES    Error - No Free system timers available.
 *
 */
result_t timer_start(UINT16 ticks,
		     expiry_function function,
		     union sigval data,
		     es_timer *timer)
{
	timer_t loop;

	if(timer->status != INACTIVE) {
		return(ERR_TIMER_ACTIVE);
	}

	/*
	 * Find the First empty timer
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		if (!timers[loop].active) {
			/*
			 * Found an inactive timer so assign to this expiry
			 */
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

	return(ERR_NO_RESOURCES);
}

/*
 * result_t timer_cancel(es_timer *timer)
 *
 * Function to cancel a running timer on the system.
 *
 * Input/Output  : es_timer *timer
 *              The timer strucure which the function is to operate on.
 *
 * Return : SUCCESS
 *
 */
result_t timer_cancel(es_timer *timer)
{
	if(timer->status == ACTIVE) {
		if(timers[timer->timer_id].active) {
			timers[timer->timer_id].active = FALSE;
			timers[timer->timer_id].expiry_count = 0;
			timers[timer->timer_id].function = (expiry_function) NULL;
		}
		timer->status = INACTIVE;
	}
	return(SUCCESS);
}

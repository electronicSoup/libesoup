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
#include "es_lib/timers/hw_timers.h"
#include "es_lib/timers/timers.h"

#ifdef ES_LINUX
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#endif // ES_LINUX

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "TIMERS"

#ifdef MCP
static u16 timer_counter = 0;

volatile BOOL timer_ticked = FALSE;

static u8 hw_timer_paused = FALSE;
static u8 hw_timer = BAD_TIMER;

/*
 * Data structure for a Timer on the Cinnamon Bun.
 */
typedef struct {
	BOOL active;
	u16 expiry_count;
	expiry_function function;
	union sigval expiry_data;
} sys_timer_t;
#endif // MCP

/*
 * The Cinnamon Bun maintains a table of timers which can be activated
 * by the calling code. The NUMBER_OF_TIMERS, defined in your system.h
 * file defines how many timers the code maintains. 
 *
 * If your project uses a limited number of know timers then you can set 
 * NUMBER_OF_TIMERS to a known value.
 */
#ifdef MCP
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#pragma udata
#endif //__PIC24FJ256GB106__
sys_timer_t timers[NUMBER_OF_TIMERS];
#endif // MCP

/*
 * Timer_1 ISR. To keep ISR short it simply restarts TIMER_1 and sets 
 * the variable "timer_tick" which is should be regularly checked by 
 * the main control loop of your project by calling the CHECK_TIMERS()
 * macro.
 */
#ifdef MCP
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
static void hw_expiry_function(u8 data)
{
	timer_ticked = TRUE;
}
#endif //__PIC24FJ256GB106__

#if defined(__18F2680) || defined(__18F4585)
void timer_isr(void)
{
	if (INTCONbits.TMR0IF) {
		INTCONbits.TMR0IF = 0;
		TMR0H = TMR0H_VAL;      // Have to write High Byte First.
		TMR0L = TMR0L_VAL;
		timer_ticked = TRUE;
	}
}
#endif // (__18F2680) || (__18F4584)
#endif // MCP

/*
 * void timer_init(void)
 *
 * Function to initialise the data structures for timers and start 
 * Timer_1 of the PIC 
 *
 */
#ifdef MCP
void timer_init(void)
{
	u8 loop;

	/*
	 * Initialise our Data Structures
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		timers[loop].active = FALSE;
		timers[loop].expiry_count = 0;
		timers[loop].function = (expiry_function)NULL;
	}

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	hw_timer = BAD_TIMER;

	hw_timer = hw_timer_start(mSeconds, 5, TRUE, hw_expiry_function, 0);
	hw_timer_paused = FALSE;
#endif //__PIC24FJ256GB106__

#if defined( __18F2680) || defined(__18F4585)
	/*
	 * Timer 0 set up
	 */
	T0CONbits.T08BIT = 0;   // 16 bit operation
	T0CONbits.T0CS = 0;     // Timer 0 Off internal clock
	T0CONbits.PSA = 1;      // Disable prescaler for Timer 0

	TMR0H = TMR0H_VAL;      // Have to write to High Reg first.
	TMR0L = TMR0L_VAL;

	INTCON2bits.TMR0IP = 1; // Set Timer to High Priority ISR
	T0CONbits.TMR0ON = 1;   // Enable Timer 0

	/*
	 * Enable interrupts from Timer 0
	 */
	INTCON2bits.TMR0IP = 1; // Timer 0 High Priority
	INTCONbits.TMR0IF = 0;  // Clear the Timer 0 interupt flag
	INTCONbits.TMR0IE = 1;  // Timer 0 Interrupt Enable
#endif // (__18F2680) || (__18F4585)
}
#endif // MCP

/*
 * void timer_tick(void)
 *
 * Function periodically called to check the state of active timers and check
 * expiry of any of the timers. If a timer has expired after the current tick
 * it's expiry function is called.
 *
 */
#ifdef MCP
void timer_tick(void)
{
	u16 active_timers;
	u8 loop;
	expiry_function function;
	union sigval data;

	active_timers = 0;
	timer_ticked = FALSE;
	timer_counter++;

	/*
	 * Check for expired timers
	 */
	for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
		if (timers[loop].active) {
			active_timers++;

			if (timers[loop].expiry_count == timer_counter) {

				/*
				 * timer expired so call expiry function.
				 */
				timers[loop].active = FALSE;

				function = timers[loop].function;
				data = timers[loop].expiry_data;

				timers[loop].expiry_count = 0;
				timers[loop].function = (expiry_function) NULL;
				function(loop, data);
			}
		}
	}

	if(!active_timers && !hw_timer_paused) {
		/*
		 * No active timers in the system so might as well pause the
		 * HW Timer.
		 */
		hw_timer_paused = TRUE;
		hw_timer_pause(hw_timer);
	}

}
#endif // MCP

/*
 * result_t timer_start(u16 ticks,
 *                      expiry_function function,
 *                      union sigval data,
 *                      es_timer *timer)
 *
 * Function to start a timer on the system.
 *
 * Input  : u16 ticks
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
result_t timer_start(u16 ticks,
		     expiry_function function,
		     union sigval data,
		     es_timer *timer)
{
#ifdef MCP
	timer_t loop;

	if(timer->status != INACTIVE) {
		LOG_E("start_timer() ERR_TIMER_ACTIVE\n\r");
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

			/*
			 * If our hw_timer isn't running restart it:
			 */
			if(hw_timer_paused) {
				if(hw_timer_restart(hw_timer, mSeconds, 5, TRUE, hw_expiry_function, 0) != SUCCESS) {
					LOG_E("Failed to restart HW timer\n\r");
				}
				hw_timer_paused = FALSE;
			}
			return(SUCCESS);
		}
	}

	LOG_E("start_timer() ERR_NO_RESOURCES\n\r");
	return(ERR_NO_RESOURCES);
#elif defined(ES_LINUX)
	struct itimerspec its;
	struct sigevent action;
	int ret;

//	LOG_D("start_timer(%d) 5ms Ticks\n\r", duration);

	if(timer->status == ACTIVE) {
		LOG_D("Cancel Running timer\n\r");
		timer_cancel(timer);
	}

	action.sigev_notify = SIGEV_THREAD;
	action.sigev_notify_function = (void (*)(union sigval))function; 
	action.sigev_notify_attributes = NULL;
	action.sigev_value = data;

	ret = timer_create(CLOCK_REALTIME, &action, &(timer->timer_id));

	if(ret) {
		LOG_E("Error can't create timer\n\r");
		return(ERR_GENERAL_ERROR);
	}
	
//	LOG_D("Setting time to %d Seconds %d nano Seonds\n\r", 
//		   (duration * SYSTEM_TICK_ms) / 1000, 
//		   (duration * SYSTEM_TICK_ms) % 1000 * 1000000);

	its.it_value.tv_sec = (ticks * SYSTEM_TICK_ms) / 1000;
	its.it_value.tv_nsec = (ticks * SYSTEM_TICK_ms) % 1000 * 1000000;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(timer->timer_id, 0, &its, NULL) == -1) {
		LOG_E("Error can't set time\n\r");
		return(ERR_GENERAL_ERROR);
	}
	timer->status = ACTIVE;

	return(SUCCESS);
#endif
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
#ifdef MCP
	if(timer->status == ACTIVE) {
		if(timers[timer->timer_id].active) {
			timers[timer->timer_id].active = FALSE;
			timers[timer->timer_id].expiry_count = 0;
			timers[timer->timer_id].function = (expiry_function) NULL;
		}
		timer->status = INACTIVE;
	}
#elif defined(ES_LINUX)
	struct itimerspec its;

	if(timer->status == ACTIVE) {
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 0;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;

		if (timer_settime(timer->timer_id, 0, &its, NULL) == -1) {
			LOG_E("Error can't create timer\n\r");
			return(ERR_GENERAL_ERROR);
		}
		timer->status = INACTIVE;
	}
#endif
	return(SUCCESS);
}

/*
 * timer_cancel_all
 */
result_t timer_cancel_all(void)
{
	u8 loop;
	es_timer timer;

	LOG_D("timer_cancel_all()\n\r");

	for (loop = 0; loop < NUMBER_OF_TIMERS; loop++) {
		if (timers[loop].active) {
			timers[loop].active = FALSE;
			timer.status = ACTIVE;
			timer.timer_id = loop;

			timer_cancel(&timer);
		}
	}
	return (SUCCESS);
}

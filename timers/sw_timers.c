/*
 *
 * file libesoup/timers/sw_timers.c
 *
 * Timer functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
 * by the SYS_SW_TICK_ms Macro and is currently set to 5 milli Second. 
 *
 * Two macros can be used to calculate the Ticks required for a timer.
 * 
 * SECONDS_TO_TICKS(s)
 * MILLI_SECONDS_TO_TICKS(ms)
 * 
 * Both of these utilities should be used for portability, just in case the 
 * timer tick period is changed.
 *
 * The libesoup/examples directory contains an example main.c named 
 * main_sw_timers.c
 *
 */
#define DEBUG_FILE TRUE
#define TAG "SW_TIMERS"

#include "libesoup_config.h"
#include "libesoup/logger/serial_log.h"
#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/sw_timers.h"

#ifdef ES_LINUX
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#endif // ES_LINUX

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

#ifndef SYS_NUMBER_OF_SW_TIMERS
#error libesoup_config.h file should define SYS_NUMBER_OF_SW_TIMERS (see libesoup/examples/libesoup_config.h)
#endif

#ifdef MCP
static uint16_t  timer_counter = 0;

volatile boolean timer_ticked = FALSE;

static uint8_t   hw_timer_paused = FALSE;
static uint8_t   hw_timer = BAD_TIMER;
#endif // MCP

/*
 * Data structure for a Timer on the Cinnamon Bun.
 */
typedef struct {
	boolean         active;
	uint16_t        expiry_count;
	expiry_function function;
	union sigval    expiry_data;
} sys_timer_t;

/*
 * The Cinnamon Bun maintains a table of timers which can be activated
 * by the calling code. The SYS_NUMBER_OF_SW_TIMERS, defined in your libesoup_config.h
 * file defines how many timers the code maintains. 
 *
 * If your project uses a limited number of know timers then you can set 
 * SYS_NUMBER_OF_SW_TIMERS to a known value.
 */
#ifdef MCP
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || (__dsPIC33EP256MU806__)
#pragma udata
#endif //__PIC24FJ256GB106__
#endif // MCP
sys_timer_t timers[SYS_NUMBER_OF_SW_TIMERS];

/*
 * Timer_1 ISR. To keep ISR short it simply restarts TIMER_1 and sets 
 * the variable "timer_tick" which is should be regularly checked by 
 * the main control loop of your project by calling the CHECK_TIMERS()
 * macro.
 */
#ifdef MCP
static void hw_expiry_function(void *data)
{
	timer_ticked = TRUE;
}
#endif // MCP

/*
 * void sw_timer_init(void)
 *
 * Function to initialise the data structures for timers and start 
 * Timer_1 of the PIC 
 *
 */
void sw_timer_init(void)
{
	uint8_t loop;

	/*
	 * Initialise our Data Structures
	 */
	for(loop=0; loop < SYS_NUMBER_OF_SW_TIMERS; loop++) {
		timers[loop].active = FALSE;
		timers[loop].expiry_count = 0;
		timers[loop].function = (expiry_function)NULL;
	}

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
	hw_timer = BAD_TIMER;

	hw_timer = hw_timer_start(mSeconds, 5, TRUE, hw_expiry_function, NULL);
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

/*
 * void timer_tick(void)
 *
 * Function periodically called to check the state of active timers and check
 * expiry of any of the timers. If a timer has expired after the current tick
 * it's expiry function is called.
 *
 * This function is not required by ES_LINUX systems
 */
#ifdef MCP
void timer_tick(void)
{
	uint16_t active_timers;
	uint8_t loop;
	expiry_function function;
	union sigval data;

	active_timers = 0;
	timer_ticked = FALSE;
	timer_counter++;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Tick\n\r");
#endif

	/*
	 * Check for expired timers
	 */
	for(loop=0; loop < SYS_NUMBER_OF_SW_TIMERS; loop++) {
		if (timers[loop].active) {
			active_timers++;

			if (timers[loop].expiry_count == timer_counter) {

				/*
				 * timer expired so call expiry function.
				 */
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
                                LOG_D("Expiry\n\r");
#endif
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
 * result_t sw_timer_start(uint16_t ticks,
 *                      expiry_function function,
 *                      union sigval data,
 *                      timer_t *timer)
 *
 * Function to start a timer on the system.
 *
 * Input  : uint16_t ticks
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
 * Output  : timer_t *timer
 *              The timer identifier which the function returns to the caller.
 *
 * Return : SUCCESS             Timer created successfully
 *          ERR_TIMER_ACTIVE    Error - Given timer is already active
 *          ERR_NO_RESOURCES    Error - No Free system timers available.
 *
 */
result_t sw_timer_start(uint16_t ticks,
		     expiry_function function,
		     union sigval data,
		     timer_t *timer)
{
#ifdef MCP
	timer_t loop;

	if(*timer != BAD_TIMER) {
                if(*timer < SYS_NUMBER_OF_SW_TIMERS) {
                        if (timers[*timer].active) {
#if (DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_WARNING)
                                LOG_W("sw_timer_start() timer passed in active. Cancelling!\n\r");
#endif
                        }
                }
		sw_timer_cancel(*timer);
	}

	/*
	 * Find the First empty timer
	 */
	for(loop=0; loop < SYS_NUMBER_OF_SW_TIMERS; loop++) {
		if (!timers[loop].active) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
                        LOG_D("Using SW timer %d\n\r", loop);
#endif
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

			*timer = loop;

			/*
			 * If our hw_timer isn't running restart it:
			 */
			if(hw_timer_paused) {
				if(hw_timer_restart(hw_timer, mSeconds, 5, TRUE, hw_expiry_function, NULL) != SUCCESS) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					LOG_E("Failed to restart HW timer\n\r");
#endif
				}
				hw_timer_paused = FALSE;
			}
			return(SUCCESS);
		}
	}

#if (SYS_LOG_LEVEL <= LOG_ERROR)
	LOG_E("start_timer() ERR_NO_RESOURCES\n\r");
#endif
	return(ERR_NO_RESOURCES);

#elif defined(ES_LINUX)
	struct itimerspec its;
	struct sigevent action;
	int ret;

	action.sigev_notify = SIGEV_THREAD;
	action.sigev_notify_function = (void (*)(union sigval))function; 
	action.sigev_notify_attributes = NULL;
	action.sigev_value = data;

	ret = timer_create(CLOCK_REALTIME, &action, timer);

	if(ret) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Error can't create timer\n\r");
#endif
		return(ERR_GENERAL_ERROR);
	}
	
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
//	LOG_D("Setting time to %d Seconds %d nano Seonds\n\r", 
#endif
	its.it_value.tv_sec = (ticks * SYS_SW_TIMER_TICK_ms) / 1000;
	its.it_value.tv_nsec = (ticks * SYS_SW_TIMER_TICK_ms) % 1000 * 1000000;
	its.it_interval.tv_sec = 0;
	its.it_interval.tv_nsec = 0;

	if (timer_settime(timer, 0, &its, NULL) == -1) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Error can't set time\n\r");
#endif
		return(ERR_GENERAL_ERROR);
	}

	return(SUCCESS);
#endif
}

/*
 * result_t sw_timer_cancel(timer_t timer)
 *
 * Function to cancel a running timer on the system.
 *
 * Input  : timer_t timer
 *              The timer identifier which the function is to operate on.
 *
 * Return : SUCCESS
 *
 */
result_t sw_timer_cancel(timer_t timer)
{
#ifdef MCP
	if ((timer == BAD_TIMER) || (timer >= SYS_NUMBER_OF_SW_TIMERS)) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
                LOG_E("sw_timer_cancel() Bad timer identifier passed in!\n\r");
#endif
                return(ERR_BAD_INPUT_PARAMETER);
	} else if (timers[timer].active) {
                timers[timer].active = FALSE;
                timers[timer].expiry_count = 0;
                timers[timer].function = (expiry_function) NULL;
        } else {
#if (DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_INFO)
                LOG_I("sw_timer_cancel() timer not active!\n\r");
#endif
        }
        
#elif defined(ES_LINUX)
	struct itimerspec its;

        its.it_value.tv_sec = 0;
        its.it_value.tv_nsec = 0;
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;

        if (timer_settime(timer, 0, &its, NULL) == -1) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
                LOG_E("Error can't create timer\n\r");
#endif
                return(ERR_GENERAL_ERROR);
        }
#endif
	return(SUCCESS);
}

/*
 * sw_timer_cancel_all
 */
#if defined(MCP)
result_t sw_timer_cancel_all(void)
{
	uint8_t    loop;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("sw_timer_cancel_all()\n\r");
#endif
	for (loop = 0; loop < SYS_NUMBER_OF_SW_TIMERS; loop++) {
		if (timers[loop].active) {
			sw_timer_cancel(loop);
		}
	}
	return (SUCCESS);
}
#endif // if defined(MCP)

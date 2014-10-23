/**
 *
 * \file es_lib/timers/timers.h
 *
 * Timer function prototypes of the electronicSoup Cinnamon Bun
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
#ifndef TIMERS_H
#define TIMERS_H

/*
 * Enumerated type for the current status of a timer in the system. A timer
 * is either Active or it's not.
 */
typedef enum {
    INACTIVE = 0x00,
    ACTIVE,
} timer_status_t;

/*
 * Simple macro to initialise the current statusof a timer to inactive.
 * A timer should always be initialsed to an inactive status before it is used
 * otherwise the timer might appear to be already active
 */
#define TIMER_INIT(timer) timer.status = INACTIVE;

/*
 * timer_t Timer identifier
 *
 * A Timer ID is used to identify a timer. It should not be used directly
 * by code but only used by timer.c
 */
#ifdef MCP
typedef u8 timer_t;
#endif

/*
 * The actual timer structure is simply the timer identifier and it's status.
 */
typedef struct
{
	timer_status_t status;
	timer_t        timer_id;
} es_timer;

/*
 * SECONDS_TO_TICKS
 *
 * Convience Macro to convert seconds to system timer ticks
 */
#define SECONDS_TO_TICKS(s)  ((s) * (1000 / SYSTEM_TICK_ms))

/*
 * MILLI_SECONDS_TO_TICKS
 *
 * Convience Macro to convert milliSeconds to system timer ticks
 */
#define MILLI_SECONDS_TO_TICKS(ms) ((ms < SYSTEM_TICK_ms) ? 1 : (ms / SYSTEM_TICK_ms))

#ifdef MCP
/*
 * .
 */
union sigval {          /* Data passed with notification */
           u16     sival_int;         /* Integer value */
           void   *sival_ptr;         /* Pointer value */
};
#endif

/**
 * \brief typedef expiry_function
 *
 * When a timer is created an expiry function is passed to the creation function.
 * The CAN Node Core SW executes this expiry_function when the timer expires.
 *
 * The expiry_function is a pointer to a function which accepts as parameter a
 * pointer to a BYTE. The expiry function will not return anything.
 */
typedef void (*expiry_function)(timer_t timer_id, union sigval);


#ifdef MCP
    #define CHECK_TIMERS()  if(timer_ticked) timer_tick();

    extern volatile BOOL timer_ticked;

    extern void timer_init(void);
    extern void timer_tick(void);
#endif

extern result_t timer_start(u16, expiry_function, union sigval, es_timer *);
extern result_t timer_cancel(es_timer *timer);

#endif


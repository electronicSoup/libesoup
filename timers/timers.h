/**
 *
 * \file es_lib/timers/timers.h
 *
 * Timer function prototypes of the electronicSoup Cinnamon Bun
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
 */
/*
 * A note on timers. The timer structures used in this electronicSoup
 * CinnamonBun Library are not as simple as they could be, given the simplicity
 * usually aspired to in simple embedded code. The structures and functions
 * used are complicated more then necessary because the timer API is modeled on
 * the Linux timer API. This is not for any other reason then the C code
 * written to use this library code can be compiled and run under Linux. This
 * has the advantage that protocols written for the CinnamonBun can easily be
 * ported to the RaspberryPi platform.
 */
#ifndef TIMERS_H
#define TIMERS_H

#ifdef MCP

#if defined (__18F2680) || defined(__18F4585)
/*
 * Calculate the 16 bit value that will give us an ISR for the system tick
 * duration.
 */
#define TMR0H_VAL ((0xFFFF - ((SYSTEM_TICK_ms * CLOCK_FREQ) / 4000)) >> 8) & 0xFF
#define TMR0L_VAL (0xFFFF - ((SYSTEM_TICK_ms * CLOCK_FREQ) / 4000)) & 0xFF
#endif // (__18F2680) || __18F4585)


    /*
     * CHECK_TIMERS()
     *
     * this CHECK_TIMERS() macro should be placed in the main loop of you code
     * so taht it calls the library timer functionality when ever the tick
     * interrup has occured.
     */
    #define CHECK_TIMERS()  if(timer_ticked) timer_tick();

    extern volatile BOOL timer_ticked;

    /*
     * timer_init()
     *
     * This function should be called to initialise the timer functionality
     * of the electronicSoup CinnamonBun Library. It initialises all data
     * structures. The project's system.h header file should define the number
     * of timers the library should define space for an manage. See the
     * definition of NUMBER_OF_TIMERS in the example system.h file in the
     * es_lib directory.
     */
    extern void timer_init(void);

    /*
     * timer_tick()
     *
     * This function should not be called directly but called with the
     * CHECK_TIMERS macro defined above. It should be called only when the
     * timer interrupt has fired for a timer tick. The period of the timer tick
     * is defined in core.h.
     */
    extern void timer_tick(void);

    /*
     * timer_isr
     */
#if defined(__18F2680) || defined(__18F4585)
    extern void timer_isr(void);
#endif // (__18F2680) || (__18F4585)

#endif // MCP

/**
 * @brief timer data structure.
 *
 * The actual timer structure is simply the timer identifier and it's status.
 */
typedef struct
{
	timer_status_t status;
	timer_t        timer_id;
} es_timer;

/**
 * @brief convience macro to initialise timer to inactive
 *
 * Simple macro to initialise the current statusof a timer to inactive.
 * A timer should always be initialsed to an inactive status before it is used
 * otherwise the timer might appear to be already active
 */
#define TIMER_INIT(timer) timer.status = INACTIVE;

/**
 * @brief convience macro to convert a Seconds value to system ticks
 *
 * For portability code should always use this macro to calculate system ticks
 * for a timer. If the system changes the @see SYSTEM_TICK_ms value for either
 * finer timer granularity or less granularity.
 */
#define SECONDS_TO_TICKS(s)  ((s) * (1000 / SYSTEM_TICK_ms))

/**
 * @brief convience macro to convert a MilliSeconds value to system ticks
 * 
 * as for @see SECONDS_TO_TICKS code should always use this macro in case system
 * timer granularity is changed. In addition future electronicSoup deivces may
 * well use different System Tick values.
 */
#define MILLI_SECONDS_TO_TICKS(ms) ((ms < SYSTEM_TICK_ms) ? 1 : (ms / SYSTEM_TICK_ms))

    
/*
 * timer_start()
 * 
 */
/**
 * @brief function to start a timer
 *
 * @param in duration: duration of the timer in system ticks. @see SECONDS_TO_TICKS
 * 
 * Starting a timer:
 * 
 * Declare an expiry function:
 * 
 * void expiry(timer_t timer_id, union sigval)
 * {
 *     // Do something     
 * }
 * 
 * // Declare both an es_timer and data to pass to the expiry function. 
 * // The data does not have to be used.
 * 
 * es_timer     timer;
 * union sigval data;
 * result_t     rc;
 *
 * // and Initialise an es_timer
 * 
 * TIMER_INIT(timer)
 *
 * // If data is to be passed to the expiry function then initialise it.
 * // the data can either be a 16 bit value or a pointer:
 * 
 * data.sival_int = 0;
 * data.sival_ptr = &es_timer;
 *
 * // Create the timer in this case for 5 Seconds
 *
 * rc = timer_start(SECONDS_TO_TICKS(5), expiry, data, &timer); 
 * 
 * // Possible error codes returned:
 * 
 * ERR_TIMER_ACTIVE : The timer passed in is already active. This might be
 *                    due to an uninitialised data structure TIMER_INIT(timer)
 * 
 * ERR_NO_RESOURCES : Out of Software timers. The number of Software timers
 *                    available in the system is defined by NUMBER_OF_TIMERS
 *                    in your system.h configuration file.
 * 
 */
extern result_t timer_start(u16 ticks, expiry_function fn, union sigval data, es_timer *timer);
extern result_t timer_cancel(es_timer *timer);
extern result_t timer_cancel_all(void);

#endif


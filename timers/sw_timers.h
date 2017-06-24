/**
 *
 * \file libesoup/timers/sw_timers.h
 *
 * Software Timer definitions and function prototypes
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
#ifndef SW_TIMERS_H
#define SW_TIMERS_H

/*
 * timer_t Timer identifier
 *
 * A Timer identifer returned from a timer created by sw_timer_create()
 */
#if defined(MCP)
typedef uint8_t timer_t;
#endif  // if defined(MCP)

/**
 * @brief convience macro to convert a Seconds value to system ticks
 *
 * For portability code should always use this macro to calculate system ticks
 * for a timer. If the system changes the @see SYS_SYSTEM_TICK_ms value for either
 * finer timer granularity or less granularity.
 */
#define SECONDS_TO_TICKS(s)  ((s) * (1000 / SYS_SW_TIMER_TICK_ms))

/**
 * @brief convience macro to convert a MilliSeconds value to system ticks
 * 
 * as for @see SECONDS_TO_TICKS code should always use this macro in case system
 * timer granularity is changed. In addition future electronicSoup deivces may
 * well use different System Tick values.
 */
#define MILLI_SECONDS_TO_TICKS(ms) ((ms < SYS_SW_TIMER_TICK_ms) ? 1 : (ms / SYS_SW_TIMER_TICK_ms))

#if defined(MCP)
/**
 * @brief Data passed to expiry funciton on timer expiry.
 *
 * The sigval union comes straight from the Linux timer API, which is why this
 * definition if encased in a test for MCP definition. The union is passed to
 * the expiry function if the timer expires. It can either carry a 16 bit
 * integer value or a pointer.
 *
 * The reason for Linux API conformance is so that code is portable between
 * Linux and Microchip devices. In the case of MCP @see libesoup/timers/timers.h
 * provides the timer functionality, where as under Linux its timer code is
 * used. In both cases user code will be the same.
 *
 * Data passed to the expiry function can either be interpreted as a 16 bit
 * value or a pointer. It's up to the creater of the timer and the connected 
 * expiry function to decide what is being passed.
 */
union sigval {
           uint16_t     sival_int;         /**< 16 bit Integer value */
           void   *sival_ptr;         /**< Pointer value */
};
#endif  // if defined(MCP)

/**
 * @brief call signiture of the timer expiry function.
 *
 * When a timer is created an expiry function is passed to the creation
 * function. The libesoup timer code executes this expiry_function when the timer
 * expires, passing the expiry function any data provided on timer creation.
 *
 * The expiry_function is a pointer to a function which accepts as parameter
 * the timer_id identifer of the timer which has expired and any associated data
 *
 * The expiry function is declared void and will not return anything to the
 * timer library code.
 *
 * Any timer expiry function should be short and sweet and return control as
 * soon as possible to the libesoup timer functionality.
 */
typedef void (*expiry_function)(timer_t timer_id, union sigval);


#ifdef MCP

#if defined (__18F2680) || defined(__18F4585)
/*
 * Calculate the 16 bit value that will give us an ISR for the system tick
 * duration.
 */
#define TMR0H_VAL ((0xFFFF - ((SYS_SW_TIMER_TICK_ms * SYS_CLOCK_FREQ) / 4000)) >> 8) & 0xFF
#define TMR0L_VAL (0xFFFF - ((SYS_SW_TIMER_TICK_ms * SYS_CLOCK_FREQ) / 4000)) & 0xFF
#endif // (__18F2680) || __18F4585)


/*
 * CHECK_TIMERS()
 *
 * this CHECK_TIMERS() macro should be placed in the main loop of you code
 * so taht it calls the library timer functionality when ever the tick
 * interrup has occured.
 */
#define CHECK_TIMERS()  if(timer_ticked) timer_tick();

extern volatile boolean timer_ticked;

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

/*
 * sw_timer_init()
 *
 * This function should be called to initialise the timer functionality
 * of the electronicSoup CinnamonBun Library. It initialises all data
 * structures. The project's libesoup_config.h header file should define the number
 * of timers the library should define space for an manage. See the
 * definition of SYS_NUMBER_OF_TIMERS in the example libesoup_config.h file in the
 * libesoup directory.
 */
extern void sw_timer_init(void);

/*
 * sw_timer_start()
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
 *                    in your libesoup_config.h configuration file.
 * 
 */
extern result_t sw_timer_start(uint16_t duration, expiry_function fn, union sigval data, timer_t *timer);
extern result_t sw_timer_cancel(timer_t timer);
extern result_t sw_timer_cancel_all(void);

#endif  // SW_TIMERS_H

/**
 * \file libesoup/timers/sw_timers.h
 *
 * \brief Software Timer API definitions and function prototypes
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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
/**
 * \brief
 * A note on timers. The timer structures used in this electronicSoup
 * CinnamonBun Library are not as simple as they could be, given the simplicity
 * usually aspired to in simple embedded code. The structures and functions
 * used are complicated more then necessary because the timer API is modeled on
 * the Linux timer API. This is not for any other reason then the C code
 * written to use this library code can be compiled and run under Linux. This
 * has the advantage that protocols written for the CinnamonBun can easily be
 * ported to the RaspberryPi platform.
 */
#ifndef _SW_TIMERS_H
#define _SW_TIMERS_H

/*
 * This API Code is only included in a build if the configuration file
 * libesoup_config.h includes the definition SYS_SW_TIMERS
 */
#ifdef SYS_SW_TIMERS

#include "libesoup/timers/hw_timers.h"

/**
 * \name Software Timers 
 */
/**@{*/

#define TIMER_INIT(timer) timer = BAD_TIMER;

#if defined(XC16) || defined(__XC8)

/**
 * \ingroup Timers
 * \brief convience macro to convert a Seconds value to system ticks
 *
 * For portability code should always use this macro to calculate system ticks
 * for a timer. If the system changes the @see SYS_SYSTEM_TICK_ms value for either
 * finer timer granularity or less granularity.
 */
//#define SECONDS_TO_TICKS(s)  ((s) * (1000 / SYS_SW_TIMER_TICK_ms))

/**
 * \ingroup Timers
 * \brief convience macro to convert a MilliSeconds value to system ticks
 * 
 * as for @see SECONDS_TO_TICKS code should always use this macro in case system
 * timer granularity is changed. In addition future electronicSoup deivces may
 * well use different System Tick values.
 */
//#define MILLI_SECONDS_TO_TICKS(ms) ((ms < SYS_SW_TIMER_TICK_ms) ? 1 : (ms / SYS_SW_TIMER_TICK_ms))

#endif  // if defined(XC16) || __XC8


#if defined(XC16) || defined(__XC8)

#if defined (__18F2680) || defined(__18F4585)
/*
 * Calculate the 16 bit value that will give us an ISR for the system tick
 * duration.
 */
#define TMR0H_VAL ((0xFFFF - ((SYS_SW_TIMER_TICK_ms * SYS_CLOCK_FREQ) / 4000)) >> 8) & 0xFF
#define TMR0L_VAL (0xFFFF - ((SYS_SW_TIMER_TICK_ms * SYS_CLOCK_FREQ) / 4000)) & 0xFF
#endif // (__18F2680) || __18F4585)

/*
 * global variable which is set by the System tick Hardware Timer. The 
 * \ref CHCEK_TIMERS macro must be called in the main loop to constantly check
 * for a system tick and process expired timers accordingly.
 */
extern volatile boolean timer_ticked;

/**
 * \ingroup Timers
 * \brief Macro to check the status of Software Timers in the system.
 *
 * The Software Timers Module starts a Hardware timer to repeatedly expire 
 * at the interval get by the libesoup_config.h definition of SYS_SW_TIMER_TICK_ms.
 * The Hardware timer will signal a system tick and the CHECK_TIMERS() macro
 * must be called in the main loop of application code so that it calls the 
 * library timer functionality when ever the tick interrup has occured.
 */
#define CHECK_TIMERS()  if(timer_ticked) timer_tick();

/**
 * \ingroup Timers
 * \function timer_tick()
 * \brief Process a system tick. Search for expired timers and call expiry funcitons.
 *
 * This function should not be called directly but called with the
 * CHECK_TIMERS macro defined above. It should be called only when the
 * timer interrupt has fired for a timer tick. The period of the timer tick
 * is defined in the system configuraiton file \ref libesoup_config.h
 */
extern void timer_tick(void);

/**
 * \ingroup Timers
 * \function timer_isr()
 * \brief The PIC18F timer interrupt 
 * 
 * The PIC18F uC's have a very limited interrupt structure containing only 
 * high priority and low priority interrupts. This timer interrupt will be called
 * from one of those main interrupt routines if the timer interrupt has occured.
 */
#if defined(__18F2680) || defined(__18F4585)
    extern void timer_isr(void);
#endif // (__18F2680) || (__18F4585)

#endif // XC16 || __XC8

/**
 * \ingroup Timers
 * \function sw_timer_init()
 * \brief Initialisation function for the Software Timers modules. Called by \ref libesoup_init()
 *
 * If the system configuration file \ref libesoup_config.h includes the definition
 * of \ref SYS_SW_TIMERS this function will be called automatically by the 
 * library initialisation funciton \ref libesoup_init()
 * 
 * The funciton initialises all data structures required by the module.
 * In additon to enabling the module with \ref SYS_SW_TIMERS the configuration 
 * file \ref libesoup_config.h should define the number of Software timers
 * that this module should initialise and manage with the definition 
 * \ref SYS_NUMBER_OF_TIMERS
 */
extern void sw_timer_init(void);

/**
 * \ingroup Timers
 * \funciton sw_timer_start()
 * \brief Start a Software based timer.
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
extern result_t sw_timer_start(ty_time_units units, uint16_t duration, timer_type type, expiry_function fn, union sigval data, timer_t *timer);
extern result_t sw_timer_cancel(timer_t timer);
extern result_t sw_timer_cancel_all(void);

/**}@*/

#endif // SYS_SW_TIMERS

#endif  // _SW_TIMERS_H


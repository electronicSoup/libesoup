/**
 * \file libesoup/timers/sw_timers.h
 *
 * \brief Software Timer API definitions and function prototypes
 *
 * Copyright 2017 electronicSoup Limited
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
#ifndef _SW_TIMERS_H
#define _SW_TIMERS_H

/*
 * This API Code is only included in a build if the configuration file
 * libesoup_config.h includes the definition SYS_SW_TIMERS
 */
#ifdef SYS_SW_TIMERS

#include "libesoup/errno.h"
#include "libesoup/timers/time.h"
#include "libesoup/timers/hw_timers.h"

/**
 * \ingroup Timers
 * \name Software Timers 
 */
/**@{*/

#define TIMER_INIT(timer) timer = BAD_TIMER_ID;

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
 * at the interval configured by the libesoup_config.h definition of SYS_SW_TIMER_TICK_ms.
 * This Hardware timer provides the "System Tick" which the CHECK_TIMERS() macro
 * processes when called. The Macro must be called in the main loop of 
 * application code so that it calls the library timer functionality when ever 
 * the tick interrupt has occurred.
 */
#define CHECK_TIMERS()  if(timer_ticked) timer_tick();

/*
 * \ingroup Timers
 * \function timer_tick()
 * \brief Process a system tick. Search for expired timers and call expiry functions.
 *
 * This function should not be called directly but called with the
 * CHECK_TIMERS macro defined above. It should be called only when the
 * timer interrupt has fired for a timer tick. The period of the timer tick
 * is defined in the system configuraiton file \ref libesoup_config.h
 */
extern void timer_tick(void);

/*
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
 * library initialisation function \ref libesoup_init()
 * 
 * The function initialises all data structures required by the module.
 * In addition to enabling the module with \ref SYS_SW_TIMERS the configuration 
 * file \ref libesoup_config.h should define the number of Software timers
 * that this module should initialise and manage with the definition 
 * \ref SYS_NUMBER_OF_SW_TIMERS
 */
extern void sw_timer_init(void);

/**
 * \ingroup Timers
 * \funciton sw_timer_start()
 * \brief Start a Software based timer.
 * 
 * @param *timer    The timer_id returned to the caller for the started timer \ref timer_id
 * @param *request  Structure containing all details of timer to be created \ref timer_req
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern timer_id sw_timer_start(struct timer_req *request);

/**
 * \ingroup Timers
 * \function sw_timer_cancel()
 * \brief Function to cancel a hardware timer running in the system.
 * 
 * \param timer Identifier of the timer, previously started, to be cancelled \ref timer_id
 */
extern timer_id sw_timer_cancel(timer_id timer);

/**
 * \ingroup Timers
 * \function sw_timer_cancel_all()
 * \brief Function to cancel all running software timers running in the system.
 */
extern timer_id sw_timer_cancel_all(void);

/**}@*/

#endif // SYS_SW_TIMERS

#endif  // _SW_TIMERS_H


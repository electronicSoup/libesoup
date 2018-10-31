/**
 * @file libesoup/timers/sw_timers.h
 *
 * @author John Whitmore
 *
 * @brief Software Timer API definitions and function prototypes
 *
 * Copyright 2017-2018 electronicSoup Limited
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
 * @ingroup Timers
 * @name Software Timers 
 */
/**@{*/

#if defined(XC16) || defined(__XC8)

/**
 * @brief boolean which indicates that the HW Timer has 'ticked'
 *
 * global variable which is set by the System tick Hardware Timer. The 
 * CHCEK_TIMERS macro must be called in the main loop to constantly check
 * for a system tick and process expired timers accordingly.
 */
extern volatile boolean timer_ticked;

#endif // XC16 || __XC8

/**
 * @ingroup Timers
 * @brief Start a Software based timer.
 * 
 * @param *request  Structure containing all details of timer to be created \ref timer_req
 * @return timer_id of the started timer if successfull, negative on error.
 */
extern timer_id sw_timer_start(struct timer_req *request);

/**
 * @ingroup Timers
 * @brief Function to cancel a hardware timer running in the system.
 * 
 * @param timer Pointer to the previously started timer, to be cancelled \ref timer_id
 * 
 * This is an atomic operation. Interrupts will be canceled for the duration of
 * the call. The caller should not test the timer for BAD_TIMER_ID but rather
 * pass in the value. If the timer is BAD_TIMER_ID then no action is taken.
 */
extern timer_id sw_timer_cancel(timer_id *timer);

/**
 * @ingroup Timers
 * @brief Function to cancel all running software timers running in the system.
 */
extern timer_id sw_timer_cancel_all(void);

/**}@*/

#endif // SYS_SW_TIMERS

#endif  // _SW_TIMERS_H


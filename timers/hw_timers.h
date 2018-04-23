/*
 * @file libesoup/timers/hw_timers.h
 *
 * @author John Whitmore
 *
 * @brief Hardware Timer API definitions
 * 
 * To include Hardware timer functionality in a build the configuration switch
 * SYS_HW_TIMERS must be included in the libesoup_config.h file.
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
 */
/**
 * @brief API Functions for using hardware timers.
 * 
 * These API Functions are enabled in a build by defining SYS_HW_TIMERS in the
 * libesoup_config.h configuration file.
 *
 *******************************************************************************
 *
 */

#ifndef _HW_TIMERS_H
#define _HW_TIMERS_H

#include "libesoup_config.h"

/*
 * Code should only be compiled if SYS_HW_TIMERS is enabled in libesoup_config.h
 */
#ifdef SYS_HW_TIMERS

#include "libesoup/timers/time.h"

/**
 * @name Hardware Timers 
 */
/**@{*/

#if (SYS_LOG_LEVEL != NO_LOGGING)
/**
 * @ingroup Timers
 * @brief This Debugging function is only included in a build if debugging
 * is enabled. It simply returns the number of currently active Hardware 
 * timers running in the system.
 */
extern uint8_t hw_timer_active_count(void);
#endif

/**
 * @ingroup Timers
 * @brief Function to start a uC hardware timer.
 * 
 * @param *request  Structure containing all details of timer to be created @ref timer_req
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 * 
 * Note: Only uSeconds, mSeconds and Seconds are valid
 * units for a Hardware timer. If a timer of duration measured in minutes, or
 * greater, is required you'll need to use a Software based timer.
 * 
 */
extern timer_id hw_timer_start(struct timer_req *request);

/**
 * @brief Function to pause a started hardware timer
 * 
 * @param timer The timer identifier of the hardware timer to pause.
 * @return Status of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern timer_id hw_timer_pause(timer_id timer);

/**
 * @ingroup Timers
 * @brief Function to restart a previously paused hardware timer.
 * 
 * @param *timer Identifier of the hardware timer, previously started with a 
 *                 call to @ref hw_timer_start(), to be restart with new
 *                 parameters for duration etc. @ref timer_id
 * @param *request  Structure containing all details of timer to be created @ref timer_req
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern timer_id hw_timer_restart(timer_id timer, struct timer_req *request);

/**
 * @ingroup Timers
 * @brief Function to cancel a hardware timer running in the system.
 * 
 * @param timer Identifier of the timer, previously started, to be cancelled @ref timer_id
 */
extern timer_id     hw_timer_cancel(timer_id timer);

/**
 * @ingroup Timers
 * @brief Function to cancel all running hardware timers running in the system.
 */
extern void     hw_timer_cancel_all();

#endif // #ifdef SYS_HW_TIMERS

#endif // _HW_TIMERS_H

/**}@*/ 

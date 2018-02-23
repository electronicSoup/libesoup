/*
 * \file libesoup/timers/hw_timers.h
 *
 * \brief Hardware Timer API definitions
 * 
 * To include Hardware timer functionality in a build the configuration switch
 * SYS_HW_TIMERS must be included in the libesoup_config.h file.
 * 
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
 */
/**
 * \brief API Functions for using hardware timers.
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
 * \name Hardware Timers 
 */
/**@{*/
/*
 * \ingroup Timers
 * \function hw_timer_init()
 * \brief Function to initialisation the data structures used to manage the 
 *        micro-controller's hardware timers. Auto called by \ref libesoup_init()
 * 
 * Warning: NO runtime checks are performed to ensure that this initialisation 
 * function has been called before a timer is started! In other words you can
 * call a function to start Hardware timer whilst the management data structures
 * are in an undefined/uninitialised state. Make sure that your project calls
 * \ref libesoup_init() prior to any API Calls
 */
extern void     hw_timer_init(void);

#if (SYS_LOG_LEVEL != NO_LOGGING)
/**
 * \ingroup Timers
 * \function hw_timer_active_count()
 * \brief This Debugging function is only included in a build if debugging
 * is enabled. It simply returns the number of currently active Hardware 
 * timers running in the system.
 */
extern uint8_t hw_timer_active_count(void);
#endif

/**
 * \ingroup Timers
 * \function hw_timer_start()
 * @brief Function to start a uC hardware timer.
 * 
 * @param *timer    The timer_id returned to the caller for the started timer \ref timer_id
 * @param *request  Structure containing all details of timer to be created \ref timer_req
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 * 
 * Note: Only uSeconds, mSeconds and Seconds are valid
 * units for a Hardware timer. If a timer of duration measured in minutes, or
 * greater, is required you'll need to use a Software based timer.
 * 
 */
extern result_t hw_timer_start(timer_id *timer, struct timer_req *request);

/**
 * \brief Function to pause a started hardware timer
 * 
 * \param timer The timer identifier of the hardware timer to pause.
 * \return Status of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern result_t hw_timer_pause(timer_id timer);

/**
 * \ingroup Timers
 * \function hw_timer_restart
 * \brief Function to restart a previously paused hardware timer.
 * 
 * @param *timer Identifier of the hardware timer, previously started with a 
 *                 call to \ref hw_timer_start(), to be restart with new
 *                 parameters for duration etc. \req timer_id
 * @param *request  Structure containing all details of timer to be created \ref timer_req
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern result_t hw_timer_restart(timer_id *timer, struct timer_req *request);

/**
 * \ingroup Timers
 * \function hw_timer_cancel()
 * \brief Function to cancel a hardware timer running in the system.
 * 
 * \param timer Identifier of the timer, previously started, to be cancelled \ref timer_id
 */
extern void     hw_timer_cancel(timer_id timer);

/**
 * \ingroup Timers
 * \function hw_timer_cancel_all()
 * \brief Function to cancel all running hardware timers running in the system.
 */
extern void     hw_timer_cancel_all();

#endif // #ifdef SYS_HW_TIMERS

#endif // _HW_TIMERS_H

/**}@*/ 

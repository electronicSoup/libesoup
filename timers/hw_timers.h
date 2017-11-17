/*
 * \file libesoup/timers/hw_timers.h
 *
 * \brief Hardware Timer API definitions
 * 
 * To include Hardware timer functionality in a build the configuration switch
 * SYS_HW_TIMERS must be included in the libesoup_config.h file.
 * 
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
 */
/** \addtogroup Timers Hardware Timers
 *  @{
 * 
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
#include "libesoup/timers/time.h"

/*
 * Code should only be compiled if SYS_HW_TIMERS is enabled in libesoup_config.h
 */
#ifdef SYS_HW_TIMERS

/**
 * \name Hardware Timers 
 */
/**@{*/
/**
 * \ingroup Timers
 * \brief Dummy timer identifier for a non existent timer.
 */
#define BAD_TIMER   0xff

/**
 * \ingroup Timers
 * \enum    hw_timer_type
 * \brief   Enumerated type for the different types of HW Timers
 */
typedef enum {
    single_shot,  /**< Single shot time which expires once only */
    repeat,       /**< Timer which repeats and will continuiously expire, unitl canceled */
} hw_timer_type;

/**
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
 * @param units Specifies units of time of the input timer duration. \ref ty_time_units
 * @param duration  Duration of requested timer. Number of time units specified by units parameter
 * @param type      The timer can be a single shot timer or repeat until canceled \ref hw_timer_type
 * @param expiry_function The function to be called on the expiry of timer.
 * @param data Caller defined data to be passed to the expiry function.
 * @return A timer identifier which can be used to cancel the started timer.
 * 
 * Note: The enumeration for units of time contains a number of different units
 * from uSeconds up to Hours, but only uSeconds, mSeconds and Seconds are valid
 * units for a Hardware timer. If a timer of duration measured in minutes, or
 * greater, is required you'll need to use a Software based timer.
 * 
 * \ref BAD_TIMER is returned if the call failed to start the requested timer.
 */
extern uint8_t  hw_timer_start(ty_time_units units, uint16_t duration, hw_timer_type type, void (*expiry_function)(void *), void *data);

/**
 * \brief Function to pause a started hardware timer
 * 
 * \param timer The timer identifier of the hardware timer to pause.
 * \return Status of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern result_t hw_timer_pause(uint8_t timer);

/**
 * \ingroup Timers
 * \function hw_timer_restart
 * \brief Function to restart a previously paused hardware timer.
 * 
 * @param hw_timer Identifier of the hardware timer, previously started with a 
 *                 call to \ref hw_timer_start(), to be restart with new
 *                 parameters for duration etc.
 * @param units Units of the duration being passed into the function \ref ty_time_units
 * @param duration The duration of the hardware timer to be started, units as specified by the units parameter
 * @param type  Type of timer to be started see \ref hw_timer_type
 * @param expiry_function Function to be called on timer expiry.
 * @param data User defined data to be passed to the expiry function.
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern result_t hw_timer_restart(uint8_t hw_timer, ty_time_units units, uint16_t duration, hw_timer_type type, void (*expiry_function)(void *), void *data);

/**
 * \ingroup Timers
 * \function hw_timer_cancel()
 * \brief Function to cancel a hardware timer running in the system.
 * 
 * \param timer Identifier of the timer, previously started, to be cancelled
 */
extern void     hw_timer_cancel(uint8_t timer);

/**
 * \ingroup Timers
 * \function hw_timer_cancel_all()
 * \brief Function to cancel all running hardware timers running in the system.
 */
extern void     hw_timer_cancel_all();

#endif // #ifdef SYS_HW_TIMERS

#endif // _HW_TIMERS_H

/**}@*/ 

/**
 * @}
 */

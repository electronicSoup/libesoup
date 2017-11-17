/*
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
/** \addtogroup timers Hardware Timers
 *  @{
 * 
 * Public header file containing API Functions for using hardware timers.
 * This API is...
 *
 * \file libesoup/timers/hw_timers.h
 *
 * \brief Hardware Timer API definitions
 * 
 * To include Hardware timer functionality in a build the configuration switch
 * SYS_HW_TIMERS must be included in the libesoup_config.h file.
 * 
 *******************************************************************************
 *
 */

#ifndef _HW_TIMERS_H
#define _HW_TIMERS_H

#include "libesoup_config.h"
#include "libesoup/timers/time.h"

#ifdef SYS_HW_TIMERS

/**
 * \brief Dummy timer identifier for a non existent timer.
 */
#define BAD_TIMER   0xff

/**
 * \brief Enumerated type for the types of HW Timers:
 *              single_shot or repeat
 */
typedef enum {
    single_shot,
    repeat,
} hw_timer_type;

/**
 * \brief Function to initialisation the data structures used to manage the 
 *        micro-controller's hardware timers.
 * 
 * This function should be called on startup, if hardware timers are to be used
 * on the system. 
 * 
 * Note: NO runtime checks are performed to ensure that this initialisation 
 * function has been called before a timer is started! In other words you can
 * call a function to start Hardware timer whilst the management data structures
 * are in an undefined/uninitialised state.
 */
extern void     hw_timer_init(void);

#if (SYS_LOG_LEVEL != NO_LOGGING)
extern uint8_t hw_timer_active_count(void);
#endif

/**
 * @brief Function to start a micro-controller hardware timer.
 * 
 * @param units Specifies units of time of the input timer duration.
 * @param duration  Duration of requested timer.
 * @param type      The timer can be a single shot timer or repeat forever.
 * @param expiry_function The function to be called on the expiry of timer.
 * @param data Caller defined data to be passed to the expiry function.
 * @return A timer identifier which can be used to cancel the started timer.
 * 
 * Note: The enumeration for units of time contains a number of different units
 * from uSeconds up to Hours, but only uSeconds, mSeconds and Seconds are valid
 * units for a Hardware timer. If a timer of duration measured in minutes, or
 * greater, is required you'll need to use a Software based timer.
 */
extern uint8_t  hw_timer_start(ty_time_units units, uint16_t duration, hw_timer_type type, void (*expiry_function)(void *), void *data);

/**
 * \brief Function to pause a started hardware timer
 * 
 * \param The timer identifier of the hardware timer to pause.
 * \return Status of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern result_t hw_timer_pause(uint8_t timer);

/**
 * \brief Function to restart a paused hardware timer.
 * 
 * This function restarts a paused timer. The new 
 * 
 * @param hw_timer Identifier of the hardware timer to restart.
 * @param units Units of the duration being passed into the function.
 * @param time The duration of the hardware timer to be started.
 * @param repeat Boolean indicating whether the timer should run continuously.
 * @param expiry_function Function to be called on timer expiry.
 * @param data User defined data to be passed to the expiry function.
 * @return Satus of the operation:
 *             - SUCCESS
 *             - ERR_BAD_INPUT_PARAMETER
 */
extern result_t hw_timer_restart(uint8_t hw_timer, ty_time_units units, uint16_t time, uint8_t repeat, void (*expiry_function)(void *), void *data);

/**
 * \brief Function to cancel a hardware timers running in the system.
 * 
 * \param timer Identifier of the timer to be cancelled
 */
extern void     hw_timer_cancel(uint8_t timer);

/**
 * \brief Function to cancel all running hardware timers running in the system.
 */
extern void     hw_timer_cancel_all();

#endif // #ifdef SYS_HW_TIMERS

#endif // _HW_TIMERS_H


/**
 * @}
 */

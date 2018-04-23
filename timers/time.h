/**
 * @file libesoup/timers/time.h
 *
 * @author John Whitmore
 *
 * @brief time definitions required by both hardware and software timers
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
#ifndef _TIME_H
#define _TIME_H

/**
 * @defgroup Timers Timers
 * @{
 * This is the timers group. Text not going where I want.
 */

/**
 * @ingroup Timers
 * @enum ty_time_units
 * @brief Enumeration for units of time. Used to specify durations for timers.
 * 
 */
typedef enum {
    uSeconds,        /**< Micro Seconds */
    mSeconds,        /**< Milli Seconds */
    Seconds,         /**< Seconds */
    Minutes,         /**< Minutes */
    Hours            /**< Hours */
} ty_time_units;

/**
 * @ingroup Timers
 * @enum    timer_type
 * @brief   Enumerated type for the different types of Timers
 */
typedef enum {
    single_shot,  /**< Single shot time which expires once only */
    repeat,       /**< Timer which repeats and will continuiously expire, unitl canceled */
} timer_type;

/**
 * @ingroup Timers
 * @brief timer identifier
 */
#if defined(XC16) || defined(__XC8)
typedef int16_t timer_id;
#endif // Microchip Compiler

/**
 * @ingroup Timers
 * @brief Dummy timer identifier for a non existent timer.
 */
#define BAD_TIMER_ID   0xff
//#define TIMER_INIT(timer) timer = BAD_TIMER_ID;

#if defined(XC16) || defined(__XC8)
/**
 * @ingroup Timers
 * @brief Data type passed to expiry function on timer expiry.
 *
 * The sigval union comes straight from the Linux timer API, which is why this
 * definition if encased in a test for XC16 || __XC8 definition. The union is passed to
 * the expiry function if the timer expires. It can either carry a 16 bit
 * integer value or a pointer.
 */
union sigval {
           uint16_t   sival_int;         /**< 16 bit Integer value */
           void      *sival_ptr;         /**< Pointer value */
};
#endif  // if defined(XC16) || __XC8

/**
 * @ingroup Timers
 * @brief call signature of the timer expiry function.
 *
 * When a timer is created an expiry function is passed to the creation
 * function. The libesoup timer code executes this expiry_function when the timer
 * expires, passing the expiry function any data provided on timer creation.
 *
 * The expiry_function is a pointer to a function which accepts as parameter
 * the @ref timer_id identifer of the timer which has expired and any associated data
 *
 * The expiry function is declared void and will not return anything to the
 * timer library code.
 *
 * Any timer expiry function should be short and sweet and return control as
 * soon as possible to the libesoup timer functionality.
 */
typedef void (*expiry_function)(timer_id timer, union sigval data);

/**
 * @ingroup Timers
 * @brief  data structure used when requesting a timer.
 * 
 * This structure is used when requesting both a Hardware timer and a Software
 * timer.
 */
struct timer_req {
    ty_time_units   units;    /**< Time units for timer @ref ty_time_units */
    uint16_t        duration; /**< Duration of the timer in units */
    timer_type      type;     /**< Type of timer to be created @ref timer_type */
    expiry_function exp_fn;   /**< expiry function to be called @ref expiry_function */
    union sigval    data;     /**< data to be passed to the expiry fnction on expiry union @ref sigval */
};

/**
 * @}
 */

#endif  // _TIME_H

/**
 * @file libesoup/core.h
 *
 * @author John Whitmore
 *
 * Core definitions required by electronicSoup Code Library
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
#ifndef _TIME_H
#define _TIMER_H
/**
 * \defgroup Timers Timers
 * @{
 * 
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
 * @}
 */

#endif  // _TIME_H

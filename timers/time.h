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
 * \brief Enumeration for units of time. Used to specify durations for timers.
 * 
 * These are fairly straight forward and represent human representations of
 * time durations. When used to start timers the API functions will convert the 
 * duration in to a micro-controller specific tick count or crystal pulse count.
 */
typedef enum {
    uSeconds,
    mSeconds,
    Seconds,
    Minutes,
    Hours
} ty_time_units;
/**
 * @}
 */

#endif  // _TIME_H

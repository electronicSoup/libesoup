/**
 *
 * \file es_lib/timers/hw_timers.h
 *
 * Hardware Timer functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
 *******************************************************************************
 *
 */
#ifndef _HW_TIMERS
#define _HW_TIMERS

#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__) || defined(__dsPIC33EP256MU806__)
#define NUMBER_HW_TIMERS  5
#elif defined(__18F2680) || defined(__18F4585)
#define NUMBER_HW_TIMERS  1
#endif

#define TIMER_0 0
#define TIMER_1 1
#define TIMER_2 2
#define TIMER_3 3
#define TIMER_4 4
#define TIMER_5 5

#define BAD_TIMER 0xff

extern void     hw_timer_init(void);
extern u8       hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void *), void *data);
//extern result_t hw_timer_restart(u8 hw_timer, ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void *), void *data);
//extern result_t hw_timer_pause(u8 timer);

extern void     hw_timer_cancel(u8 timer);
extern void     hw_timer_cancel_all();

#endif // _HW_TIMERS
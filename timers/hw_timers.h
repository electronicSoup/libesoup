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

#define BAD_TIMER 0xff

extern void     hw_timer_init(void);
extern uint8_t  hw_timer_start(ty_time_units units, uint16_t time, uint8_t repeat, void (*expiry_function)(void *), void *data);
extern result_t hw_timer_restart(uint8_t hw_timer, ty_time_units units, uint16_t time, uint8_t repeat, void (*expiry_function)(void *), void *data);
extern result_t hw_timer_pause(uint8_t timer);

extern void     hw_timer_cancel(uint8_t timer);
extern void     hw_timer_cancel_all();

#endif // _HW_TIMERS

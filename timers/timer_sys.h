/**
 *
 * \file es_can/timers/timers_sys.h
 *
 * Timer definitions of electronicSoup CAN code Library
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef TIMERS_H
#define TIMERS_H

#ifdef MCP
    #define CHECK_TIMERS()  if(timer_tick) tick();

    extern volatile BOOL timer_tick;

    extern void init_timer(void);
    extern void tick(void);
#endif

extern result_t start_timer(u16, expiry_function, union sigval, es_timer *);

extern result_t cancel_timer(es_timer *timer);

#endif


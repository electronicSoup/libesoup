/**
 *
 * \file es_lib/timers/rtc.c
 *
 * This file contains code for dealing with Real Date Time values
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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

struct datetime {
    u16 year;
    u8  month;
    u8  day;
    u8  hours;
    u8  minutes;
    u8  seconds;
};

extern void rtc_init();
extern result_t rtc_update_current_datetime(u8 *data, u16 len);

extern result_t rtc_set_alarm(ty_time_units units, u16 time, u8 nice, void (*expiry_function)(void));

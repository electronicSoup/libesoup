/**
 *
 * \file libesoup/timers/rtc.c
 *
 * This file contains code for dealing with Real Date Time values
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
 */
#ifndef RTC_H
#define RTC_H

struct datetime {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
};

extern void rtc_init();
extern result_t rtc_update_current_datetime(uint8_t *data, uint16_t len);

extern void    *rtc_set_alarm_offset(ty_time_units units, uint16_t time, uint8_t nice, void (*expiry_fn)(void *), void *expiry_data);

extern result_t rtc_get_current_datetime(struct datetime *dt);
extern result_t rtc_get_dummy_datetime(struct datetime *dt);

#endif // RTC_H

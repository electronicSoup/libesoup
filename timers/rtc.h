/**
 *
 * @file libesoup/timers/rtc.h
 *
 * @author John Whitmore
 *
 * This file contains code for dealing with Real Date Time values
 *
 * Copyright 2017-2018 electronicSoup Limited
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
#ifndef _RTC_H
#define _RTC_H

#include "libesoup_config.h"

#ifdef SYS_HW_RTC

#include "libesoup/timers/time.h"

/**
 * @struct  datetime
 * @brief   structure to hold a date time value
 */
struct datetime {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    uint8_t  hours;
    uint8_t  minutes;
    uint8_t  seconds;
};

/**
 * @struct  bcd_datetime
 * @brief   structure to hold a date time value in BCD Format
 */
struct bcd_datetime {
    uint16_t year_one:4;
    uint16_t year_ten:4;
    uint16_t :8;
    uint16_t day_one:4;
    uint16_t day_ten:2;
    uint16_t :2;
    uint16_t month_one:4;
    uint16_t month_ten:1;
    uint16_t :3;
    uint16_t hour_one:4;
    uint16_t hour_ten:2;
    uint16_t :2;
    uint16_t weekday:3;
    uint16_t :5;
    uint16_t second_one:4;
    uint16_t second_ten:4;
    uint16_t minute_one:4;
    uint16_t minute_ten:4;
};

/**
 * @def   rtc_init()
 * @brief function to initialise rtc functionality
 */
extern result_t rtc_init(void);
extern result_t rtc_set_datetime(struct bcd_datetime*);
extern result_t rtc_get_datetime(struct bcd_datetime*);
#ifdef SYS_SERIAL_LOGGING
extern void rtc_print_datetime(struct bcd_datetime*);
#endif // SYS_SERIAL_LOGGING

extern result_t rtc_set_alarm(struct bcd_datetime*, expiry_function, union sigval);

#endif // SYS_HW_RTC
#endif // _RTC_H

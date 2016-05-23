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
/*
 * To use this code the system.h file must define SW_RTC_TICK_SECS
 * 
 * SW_RTC_TICK_SECS - This is the rtc period of a tick in seconds
 */
#include <stdlib.h>

#define DEBUG_FILE
#define TAG "SW_RTC"

#define SECONDS_PER_MINUTE  60
#define MINUTES_PER_HOUR    60
#define HOURS_PER_DAY       24

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/hw_timers.h"
#include "es_lib/timers/rtc.h"

struct alarm_data {
	struct alarm_data *next;
	struct datetime    datetime;
	void             (*expiry_fn)(void *);
	void              *expiry_data;
};

static struct alarm_data *alarm_list = NULL;
static struct datetime    current_datetime;
static u8                 current_datetime_valid = FALSE;
static u16                current_isr_secs = 0;

/*
 * Function prototypes
 */
static void increment_current_time(u16 current_isr_secs);
static void check_alarm();
static void add_alarm_to_list(struct alarm_data *alarm);
static s8 datetime_cmp(struct datetime *a, struct datetime *b);

void rtc_init(void)
{
	alarm_list = NULL;
}

void timer_expiry(u8 data)
{
	increment_current_time(current_isr_secs);

	current_isr_secs = SW_RTC_TICK_SECS;

	hw_timer_start(Seconds, current_isr_secs, FALSE, timer_expiry, 0);

	if(alarm_list) {
		check_alarm();
	}
}

static void increment_current_time(u16 secs)
{
	u16 tmp_seconds;
	u16 tmp_minutes;
	u16 tmp_hours;
	u16 tmp_day;

	tmp_seconds = current_datetime.seconds + secs;
	tmp_minutes = current_datetime.minutes + tmp_seconds / SECONDS_PER_MINUTE;
	tmp_hours   = current_datetime.hours + tmp_minutes / MINUTES_PER_HOUR;
	tmp_day     = current_datetime.day + tmp_hours / HOURS_PER_DAY;

//	LOG_D("Current seconds is %d\n\r", current_datetime.seconds);
//	LOG_D("Add on %d Seconds\n\r", secs);
//	LOG_D("tmp_seconds is %d\n\r", tmp_seconds);
//	LOG_D("/ is %d remainder is %d \n\r", tmp_seconds / 60, tmp_seconds % 60);

	current_datetime.seconds = tmp_seconds % SECONDS_PER_MINUTE;
	current_datetime.minutes = tmp_minutes % MINUTES_PER_HOUR;
	current_datetime.hours   = tmp_hours % HOURS_PER_DAY;
	current_datetime.day     = tmp_day;
}

result_t rtc_update_current_datetime(u8 *data, u16 len)
{
//	u32 timer;

	LOG_D("rtc_update_current_datetime()\n\r");

	if(len != 17) {
		LOG_E("Bad input datetime\n\r");
		return(ERR_BAD_INPUT_PARAMETER);
	}

	current_datetime.year    = ((data[0] - '0') * 1000) + ((data[1] -'0') * 100) + ((data[2] -'0') * 10) + (data[3] - '0');
	current_datetime.month   = ((data[4] - '0') * 10) + (data[5] - '0');
	current_datetime.day     = ((data[6] - '0') * 10) + (data[7] - '0');
	current_datetime.hours   = ((data[9] - '0') * 10) + (data[10] - '0');
	current_datetime.minutes = ((data[12] - '0') * 10) + (data[13] - '0');
	current_datetime.seconds = ((data[15] - '0') * 10) + (data[16] - '0');

	current_datetime_valid = TRUE;

	LOG_D("Current datetime set to %d%d%d-%d:%d:%d\n\r",
		current_datetime.year,
		current_datetime.month,
		current_datetime.day,
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds);

	if (SW_RTC_TICK_SECS > current_datetime.seconds) {
		current_isr_secs = SW_RTC_TICK_SECS - current_datetime.seconds;
	} else if(SW_RTC_TICK_SECS == current_datetime.seconds) {
		current_isr_secs = SW_RTC_TICK_SECS;
	} else {
		current_isr_secs = current_datetime.seconds % SW_RTC_TICK_SECS;
	}

	hw_timer_start(Seconds, current_isr_secs, FALSE, timer_expiry, 0);

//	LOG_D("Current isr secs %d last digit %d\n\r", current_isr_secs, (data[16] - '0'));

	return(SUCCESS);
}

void *rtc_set_alarm_offset(ty_time_units units, u16 time, u8 nice, void (*expiry_fn)(void *), void *expiry_data)
{
	u16                current_minutes;
	u16                total_alarm_minutes;
	struct datetime    tmp_datetime;
	struct alarm_data *alarm;

	tmp_datetime.year    = current_datetime.year;
	tmp_datetime.month   = current_datetime.month;
	tmp_datetime.day     = current_datetime.day;
	tmp_datetime.hours   = 0;
	tmp_datetime.minutes = 0;
	tmp_datetime.seconds = 0;

	/*
	 * Calculate the Alarm Datetime
	 *
	 */
	if (units == Seconds) {
		tmp_datetime.seconds = current_datetime.seconds + time;
		tmp_datetime.minutes = current_datetime.minutes + tmp_datetime.seconds / 60;
		tmp_datetime.seconds = tmp_datetime.seconds % 60;
		tmp_datetime.hours   = current_datetime.hours + tmp_datetime.minutes / 60;
		tmp_datetime.minutes = tmp_datetime.minutes % 60;
		tmp_datetime.day     = current_datetime.day + tmp_datetime.hours / 24;
		tmp_datetime.hours   = tmp_datetime.hours % 24;
	} else if (units == Minutes) {
		LOG_D("rtc_set_alarm_offset(Minutes, %d)\n\r", time);
		tmp_datetime.seconds = 0;

		if(nice) {
			if(time < MINUTES_PER_HOUR) {
				if((MINUTES_PER_HOUR % time) == 0) {
					tmp_datetime.minutes = ((current_datetime.minutes / time) + 1) * time;
				} else {
					current_minutes = (MINUTES_PER_HOUR * current_datetime.hours) + current_datetime.minutes;
					total_alarm_minutes = ((current_minutes / time) + 1) * time;
					tmp_datetime.minutes = total_alarm_minutes % MINUTES_PER_HOUR;
					tmp_datetime.hours = total_alarm_minutes / MINUTES_PER_HOUR;
				}
			}
		} else {
			tmp_datetime.minutes = current_datetime.minutes + time;
		}

		tmp_datetime.hours = tmp_datetime.hours + current_datetime.hours + (tmp_datetime.minutes / MINUTES_PER_HOUR);
		tmp_datetime.minutes = tmp_datetime.minutes % MINUTES_PER_HOUR;
		tmp_datetime.day = current_datetime.day + tmp_datetime.hours / HOURS_PER_DAY;
		tmp_datetime.hours = tmp_datetime.hours % HOURS_PER_DAY;
	} else if (units == Hours) {
		tmp_datetime.seconds = current_datetime.seconds;
		tmp_datetime.minutes = current_datetime.minutes;
		tmp_datetime.hours   = current_datetime.hours + time;
		tmp_datetime.day     = current_datetime.day + tmp_datetime.hours / HOURS_PER_DAY;
		tmp_datetime.hours   = tmp_datetime.hours % HOURS_PER_DAY;
	} else {
		LOG_E("Unrecognised Alarm offset units\n\r");
		return(NULL);
	}

	/*
	 * Create a new alarm data structure
	 */
	alarm = malloc(sizeof(struct alarm_data));

	alarm->next             = NULL;
	alarm->datetime.year    = tmp_datetime.year;
	alarm->datetime.month   = tmp_datetime.month;
	alarm->datetime.day     = tmp_datetime.day;
	alarm->datetime.hours   = tmp_datetime.hours;
	alarm->datetime.minutes = tmp_datetime.minutes;
	alarm->datetime.seconds = tmp_datetime.seconds;

	alarm->expiry_fn   = expiry_fn;
	alarm->expiry_data = expiry_data;

	/*
	 * Add the alarm to the list in order
	 */
	add_alarm_to_list(alarm);

	return((void *)alarm);
}

static void add_alarm_to_list(struct alarm_data *alarm)
{
	struct alarm_data *next = NULL;
	struct alarm_data *prev = NULL;
	u16                count = 0;

	LOG_D("add_alarm_to_list(%2d:%2d)\n\r", alarm->datetime.hours, alarm->datetime.minutes);

	if (alarm_list == NULL) {
		LOG_D("Alarm list null so adding to head\n\r");
		alarm_list = alarm;
	} else {
		next = alarm_list;

		if(datetime_cmp(&alarm->datetime, &next->datetime) <= 0) {
			LOG_D("Alarm should be first so inserting to head\n\r");
			/*
			 * Insert at head of list
			 */
			alarm->next = alarm_list;
			alarm_list = alarm;
		} else {
			prev = alarm_list;
			next = prev->next;

			while(next) {
				if (datetime_cmp(&alarm->datetime, &next->datetime) <= 0) {
					/*
					 * Insert
					 */
					prev->next = alarm;
					alarm->next = next;
				} else {
					prev = next;
					next = prev->next;
				}
			}

			if(!next) {
				prev->next = alarm;
				alarm->next = NULL;
			}
		}
	}

	next = alarm_list;
	while(next) {
		next = next->next;
		count++;
	}

	LOG_D("Alarm count %d\n\r", count);
}

/*
 * if Return value < 0 then it indicates datetime a is before datetime b.
 * if Return value > 0 then it indicates datetime b is before datetime a.
 * if Return value = 0 then it indicates datetime a is the same as datetime b
 */
static s8 datetime_cmp(struct datetime *a, struct datetime *b)
{
	/*
	 * Compare years
	 */
	if(a->year != b->year) {
		LOG_D("Comparison on year %d %d\n\r", a->year, b->year);
		return(a->year - b->year);
	}

	/*
	 * Compare months
	 */
	if(a->month != b->month) {
		LOG_D("Comparison on Month %d %d\n\r", a->month, b->month);
		return(a->month - b->month);
	}

	/*
	 * Compare day
	 */
	if(a->day != b->day) {
		LOG_D("Comparison on day %d %d\n\r", a->day, b->day);
		return(a->day - b->day);
	}

	/*
	 * Compare hours
	 */
	if(a->hours != b->hours) {
		LOG_D("Comparison on Hours %d %d\n\r", a->hours, b->hours);
		return(a->hours - b->hours);
	}

	/*
	 * Compare minutes
	 */
	if(a->minutes != b->minutes) {
		LOG_D("Comparison on minutes %d %d\n\r", a->minutes, b->minutes);
		return(a->minutes - b->minutes);
	}

	/*
	 * Compare seconds
	 */
	if(a->seconds != b->seconds) {
		LOG_D("Comparison on seconds %d %d\n\r", a->seconds, b->seconds);
		return(a->seconds - b->seconds);
	}

	/*
	 * Both must be equal
	 */
	return(0);
}

static void check_alarm()
{
	u8                 finished = FALSE;
	u16                count = 0;
	struct alarm_data *tmp_alarm;

	LOG_D("check_alarm() Compare Current time %d:%d:%d\n\r",
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds);

	/*
	 * While loop as a number of alarms could have the same time
	 */
	while (alarm_list && !finished) {
		asm ("CLRWDT");
		LOG_D("Check alarm - %d:%d:%d\n\r",
			alarm_list->datetime.hours,
			alarm_list->datetime.minutes,
			alarm_list->datetime.seconds);

		if(datetime_cmp(&current_datetime, &alarm_list->datetime) >= 0) {
			/*
			 * Call the expiry function
			 */
			alarm_list->expiry_fn(alarm_list->expiry_data);

			LOG_D("Back from expiry function\n\r");
			/*
			 * Remove the alarm from the Queue
			 */
			tmp_alarm = alarm_list;
			
			alarm_list = tmp_alarm->next;

			LOG_D("Free the alarm at head of list\n\r");
			free(tmp_alarm);
		} else {
			LOG_D("Finished the checking\n\r");
			finished = TRUE;
		}
	}

	tmp_alarm = alarm_list;
	count = 0;
	while(tmp_alarm) {
		LOG_D("alarm - %d:%d:%d\n\r",
			tmp_alarm->datetime.hours,
			tmp_alarm->datetime.minutes,
			tmp_alarm->datetime.seconds);

		tmp_alarm = tmp_alarm->next;
		count++;
	}
	LOG_D("Alarm count %d\n\r", count);
}

result_t rtc_get_current_datetime(struct datetime *dt)
{
	if(current_datetime_valid) {
		dt->hours = current_datetime.hours;
		dt->minutes = current_datetime.minutes;
		dt->seconds = current_datetime.seconds;

		return (SUCCESS);
	} else {
		return(ERR_NOT_READY);
	}
}

result_t rtc_get_dummy_datetime(struct datetime *dt)
{
	dt->year    = 2000;
	dt->month   = 1;
	dt->day     = 1;
	dt->hours   = 00;
	dt->minutes = 00;
	dt->seconds = 00;

	return (SUCCESS);
}

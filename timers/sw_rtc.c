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
static u8                 alarm_set = FALSE;

/*
 * Function prototypes
 */
static void increment_current_time(u16 current_isr_secs);
static void check_alarm();
static void add_alarm_to_list(struct alarm_data *alarm);
static s8 alarm_cmp(struct alarm_data *a, struct alarm_data *b);

void rtc_init(void)
{
	alarm_list = NULL;
}

void timer_expiry(u8 data)
{
	increment_current_time(current_isr_secs);

	current_isr_secs = SW_RTC_TICK_SECS;

	hw_timer_start(Seconds, current_isr_secs, FALSE, timer_expiry, 0);

	if(alarm_set) {
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
	tmp_minutes = current_datetime.minutes + tmp_seconds / 60;
	tmp_hours   = current_datetime.hours + tmp_minutes / 60;
	tmp_day     = current_datetime.day + tmp_hours / 24;

//	LOG_D("Current seconds is %d\n\r", current_datetime.seconds);
//	LOG_D("Add on %d Seconds\n\r", secs);
//	LOG_D("tmp_seconds is %d\n\r", tmp_seconds);
//	LOG_D("/ is %d remainder is %d \n\r", tmp_seconds / 60, tmp_seconds % 60);

	current_datetime.seconds = tmp_seconds % 60;
	current_datetime.minutes = tmp_minutes % 60;
	current_datetime.hours   = tmp_hours % 24;
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

result_t rtc_set_alarm_offset(ty_time_units units, u16 time, u8 nice, void (*expiry_fn)(void *), void *expiry_data)
{
	struct datetime tmp_datetime;
	struct alarm_data *alarm;

	/*
	 * Calculate the Alarm Datetime
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
		tmp_datetime.seconds = current_datetime.seconds;
		tmp_datetime.minutes = current_datetime.minutes + time;
		tmp_datetime.hours   = current_datetime.hours + tmp_datetime.minutes / 60;
		tmp_datetime.minutes = tmp_datetime.minutes % 60;
		tmp_datetime.day     = current_datetime.day + tmp_datetime.hours / 24;
		tmp_datetime.hours   = tmp_datetime.hours % 24;
	} else if (units == Hours) {
		tmp_datetime.seconds = current_datetime.seconds;
		tmp_datetime.minutes = current_datetime.minutes;
		tmp_datetime.hours   = current_datetime.hours + time;
		tmp_datetime.day     = current_datetime.day + tmp_datetime.hours / 24;
		tmp_datetime.hours   = tmp_datetime.hours % 24;
	} else {
		LOG_E("Unrecognised Alarm offset units\n\r");
		return(ERR_BAD_INPUT_PARAMETER);
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

	return(SUCCESS);
}

static void add_alarm_to_list(struct alarm_data *alarm)
{
	struct alarm_data *next = NULL;
	struct alarm_data *prev = NULL;

	LOG_D("add_alarm_to_list()\n\r");

	if (alarm_list == NULL) {
		alarm_list = alarm;
		return;
	} else {
		next = alarm_list;

		if(alarm_cmp(alarm, next) <= 0) {
			/*
			 * Insert at head of list
			 */
			alarm->next = alarm_list;
			alarm_list = alarm;
			return;
		} else {
			prev = alarm_list;
			next = prev->next;

			while(next) {
				if (alarm_cmp(alarm, next) <= 0) {
					/*
					 * Insert at head of list
					 */
					prev->next = alarm;
					alarm->next = next;
					return;
				} else {
					prev = next;
					next = prev->next;
				}
			}
		}
	}
}

static s8 alarm_cmp(struct alarm_data *a, struct alarm_data *b)
{
	/*
	 * Compare years
	 */
	if(a->datetime.year < b->datetime.year) {
		return(-1);
	} else if (a->datetime.year > b->datetime.year) {
		return(1);
	}

	/*
	 * Compare months
	 */
	if(a->datetime.month < b->datetime.month) {
		return(-1);
	} else if (a->datetime.month > b->datetime.month) {
		return(1);
	}

	/*
	 * Compare day
	 */
	if(a->datetime.day < b->datetime.day) {
		return(-1);
	} else if (a->datetime.day > b->datetime.day) {
		return(1);
	}

	/*
	 * Compare hours
	 */
	if(a->datetime.hours < b->datetime.hours) {
		return(-1);
	} else if (a->datetime.hours > b->datetime.hours) {
		return(1);
	}

	/*
	 * Compare minutes
	 */
	if(a->datetime.minutes < b->datetime.minutes) {
		return(-1);
	} else if (a->datetime.minutes > b->datetime.minutes) {
		return(1);
	}

	/*
	 * Compare seconds
	 */
	if(a->datetime.seconds < b->datetime.seconds) {
		return(-1);
	} else if (a->datetime.seconds > b->datetime.seconds) {
		return(1);
	}

	/*
	 * Both must be equal
	 */
	return(0);
}

static void check_alarm()
{
	u8 finished = FALSE;
	struct alarm_data *tmp_alarm;

	LOG_D("check_alarm() Compare Current time %d:%d:%d\n\r",
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds);

	/*
	 * While loop as a number of alarms could have the same time
	 */
	while (alarm_list && !finished) {
		LOG_D("Check alarm - %d:%d:%d\n\r",
			alarm_list->datetime.hours,
			alarm_list->datetime.minutes,
			alarm_list->datetime.seconds);

		if(alarm_cmp(alarm_list, &current_datetime) <= 0) {
			/*
			 * Call the expiry function
			 */
			alarm_list->expiry_fn(alarm_list->expiry_data);

			/*
			 * Remove the alarm from the Queue
			 */
			tmp_alarm = alarm_list;
			
			alarm_list = tmp_alarm->next;
			
			free(tmp_alarm);
		} else {
			finished = TRUE;
		}
	}

	if(!alarm_list) {
		alarm_set = FALSE;
	}
}

result_t rtc_get_current_datetime(struct datetime *dt)
{
	dt->hours = current_datetime.hours;
	dt->minutes = current_datetime.minutes;
	dt->seconds = current_datetime.seconds;

	return(SUCCESS);
}
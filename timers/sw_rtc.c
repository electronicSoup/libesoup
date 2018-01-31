/**
 *
 * \file libesoup/timers/rtc.c
 *
 * This file contains code for dealing with Real Date Time values
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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
#include "libesoup_config.h"

/*
 * This is an experimental piece of code never finished and will probably be
 * removed as moved to an actual HW RTC.
 */
#ifdef SYS_SW_RTC

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "SW_RTC";
#include "libesoup/logger/serial_log.h"
#endif

#define SECONDS_PER_MINUTE  60
#define MINUTES_PER_HOUR    60
#define HOURS_PER_DAY       24

#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/rtc.h"
#include "libesoup/jobs/jobs.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_SW_TIMER_TICK_ms
#error libesoup_config.h file should define SYS_SW_TIMER_TICK_ms (see libesoup/examples/libesoup_config.h)
#endif

struct alarm_data {
	struct alarm_data *next;
	struct datetime    datetime;
	void             (*expiry_fn)(void *);
	void              *expiry_data;
};

static struct alarm_data *alarm_list = NULL;
static struct datetime    current_datetime;
static uint8_t                 current_datetime_valid = FALSE;
static uint16_t                current_isr_secs = 0;

/*
 * Function prototypes
 */
static void increment_current_time(uint16_t current_isr_secs);
static void check_alarm();
static void add_alarm_to_list(struct alarm_data *alarm);
static int8_t datetime_cmp(struct datetime *a, struct datetime *b);

void rtc_init(void)
{
	alarm_list = NULL;
}

void timer_expiry(void *data)
{
	increment_current_time(current_isr_secs);

	current_isr_secs = SYS_SW_RTC_TICK_SECS;

	hw_timer_start(Seconds, current_isr_secs, FALSE, timer_expiry, NULL);

	if(alarm_list) {
		check_alarm();
	}
}

static void increment_current_time(uint16_t secs)
{
	uint16_t tmp_seconds;
	uint16_t tmp_minutes;
	uint16_t tmp_hours;
	uint16_t tmp_day;

	tmp_seconds = current_datetime.seconds + secs;
	tmp_minutes = current_datetime.minutes + tmp_seconds / SECONDS_PER_MINUTE;
	tmp_hours   = current_datetime.hours + tmp_minutes / MINUTES_PER_HOUR;
	tmp_day     = current_datetime.day + tmp_hours / HOURS_PER_DAY;

	current_datetime.seconds = tmp_seconds % SECONDS_PER_MINUTE;
	current_datetime.minutes = tmp_minutes % MINUTES_PER_HOUR;
	current_datetime.hours   = tmp_hours % HOURS_PER_DAY;
	current_datetime.day     = tmp_day;
}

result_t rtc_update_current_datetime(uint8_t *data, uint16_t len)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("rtc_update_current_datetime()\n\r");
#endif

	if(len != 17) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Bad input datetime\n\r");
#endif
		return(ERR_BAD_INPUT_PARAMETER);
	}

	current_datetime.year    = ((data[0] - '0') * 1000) + ((data[1] -'0') * 100) + ((data[2] -'0') * 10) + (data[3] - '0');
	current_datetime.month   = ((data[4] - '0') * 10) + (data[5] - '0');
	current_datetime.day     = ((data[6] - '0') * 10) + (data[7] - '0');
	current_datetime.hours   = ((data[9] - '0') * 10) + (data[10] - '0');
	current_datetime.minutes = ((data[12] - '0') * 10) + (data[13] - '0');
	current_datetime.seconds = ((data[15] - '0') * 10) + (data[16] - '0');

	current_datetime_valid = TRUE;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Current datetime set to %d%d%d-%d:%d:%d\n\r",
#endif
		current_datetime.year,
		current_datetime.month,
		current_datetime.day,
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds);

	if (SYS_SW_RTC_TICK_SECS > current_datetime.seconds) {
		current_isr_secs = SYS_SW_RTC_TICK_SECS - current_datetime.seconds;
	} else if(SYS_SW_RTC_TICK_SECS == current_datetime.seconds) {
		current_isr_secs = SYS_SW_RTC_TICK_SECS;
	} else {
		current_isr_secs = current_datetime.seconds % SYS_SW_RTC_TICK_SECS;
	}

	hw_timer_start(Seconds, current_isr_secs, FALSE, timer_expiry, NULL);

	return(SUCCESS);
}

void *rtc_set_alarm_offset(ty_time_units units, uint16_t time, uint8_t nice, void (*expiry_fn)(void *), void *expiry_data)
{
	uint16_t                current_minutes;
	uint16_t                total_alarm_minutes;
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
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("rtc_set_alarm_offset(Minutes, %d)\n\r", time);
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Unrecognised Alarm offset units\n\r");
#endif
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("add_alarm_to_list(%2d:%2d)\n\r", alarm->datetime.hours, alarm->datetime.minutes);
#endif
	if (alarm_list == NULL) {
		alarm_list = alarm;
	} else {
		next = alarm_list;

		if(datetime_cmp(&alarm->datetime, &next->datetime) <= 0) {
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
}

/*
 * if Return value < 0 then it indicates datetime a is before datetime b.
 * if Return value > 0 then it indicates datetime b is before datetime a.
 * if Return value = 0 then it indicates datetime a is the same as datetime b
 */
static int8_t datetime_cmp(struct datetime *a, struct datetime *b)
{
	/*
	 * Compare years
	 */
	if(a->year != b->year) {
		return(a->year - b->year);
	}

	/*
	 * Compare months
	 */
	if(a->month != b->month) {
		return(a->month - b->month);
	}

	/*
	 * Compare day
	 */
	if(a->day != b->day) {
		return(a->day - b->day);
	}

	/*
	 * Compare hours
	 */
	if(a->hours != b->hours) {
		return(a->hours - b->hours);
	}

	/*
	 * Compare minutes
	 */
	if(a->minutes != b->minutes) {
		return(a->minutes - b->minutes);
	}

	/*
	 * Compare seconds
	 */
	if(a->seconds != b->seconds) {
		return(a->seconds - b->seconds);
	}

	/*
	 * Both must be equal
	 */
	return(0);
}

static void check_alarm()
{
	uint8_t                 finished = FALSE;
	struct alarm_data *tmp_alarm;

	/*
	 * While loop as a number of alarms could have the same time
	 */
	while (alarm_list && !finished) {
		if(datetime_cmp(&current_datetime, &alarm_list->datetime) >= 0) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Alarm Expired - %d:%d:%d\n\r",
				alarm_list->datetime.hours,
				alarm_list->datetime.minutes,
				alarm_list->datetime.seconds);
#endif
			/*
			 * Remove the alarm from the Queue
			 */
			tmp_alarm = alarm_list;

			alarm_list = tmp_alarm->next;

			/*
			 * Call the expiry function
			 */
			// tmp_alarm->expiry_fn(tmp_alarm->expiry_data);
			jobs_add(tmp_alarm->expiry_fn, (void *)tmp_alarm->expiry_data);

                        /*
                         * And free the expired alarm
                         */
			free(tmp_alarm);

		} else {
			finished = TRUE;
		}
	}
}

result_t rtc_get_current_datetime(struct datetime *dt)
{
	if(current_datetime_valid) {
                dt->year    = current_datetime.year;
                dt->month   = current_datetime.month;
                dt->day     = current_datetime.day;
		dt->hours   = current_datetime.hours;
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

#endif // SYS_SW_RTC

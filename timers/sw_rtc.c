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
#define DEBUG_FILE
#define TAG "SW_RTC"

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/hw_timers.h"
#include "es_lib/timers/sw_rtc.h"

static struct datetime current_datetime;
static u8  current_datetime_valid = FALSE;

static u16 current_isr_secs = 0;
//static u16 sleep_request_secs = 0;

static u8               alarm_set = FALSE;
static struct datetime  alarm_datetime;
static void           (*alarm_expiry_fn)(void) = NULL;
/*
 * Function prototypes
 */
//static void rtc_10_sec_isr();
static void increment_current_time(u16 current_isr_secs);
static void check_alarm();

#if 0
#ifdef MCP
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
void _ISR __attribute__((__no_auto_psv__)) _T5Interrupt(void)
{
	IFS1bits.T5IF = 0;

	increment_current_time(current_isr_secs);
	current_isr_secs = 0;

	rtc_10_sec_isr();

	LOG_D("Current datetime set to %d%d%d-%d:%d:%d\n\r", current_datetime.year,
		current_datetime.month,
		current_datetime.day,
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds);
}
#endif //__PIC24FJ256GB106__
#endif
#endif

#if 0
void rtc_init()
{
	u32 timer;

	LOG_D("rtc_init()\n\r");
	current_datetime_valid = FALSE;

 	/*
	 * Initialise Timer 4
	 */
	T4CONbits.T32 = 1;      // 16 Bit Timer
	T4CONbits.TCS = 0;      // Internal FOSC/2
	T4CONbits.TCKPS1 = 1;   // Divide by 256
	T4CONbits.TCKPS0 = 1;

	increment_current_time(current_isr_secs);
	rtc_10_sec_isr();

	IEC1bits.T5IE = 1;
	T4CONbits.TON = 1;
}
#endif

#if 0
static void rtc_10_sec_isr()
{
	u32 timer;

	timer = ((u32)((CLOCK_FREQ / 256) * 10) - 1) ;
	PR4 = (u16)(timer & 0xffff);
	PR5 = (u16)((timer >> 16) & 0xffff);
	TMR4 = 0x00;
	TMR5 = 0x00;

	current_isr_secs = 10;
}
#endif

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
#if 0
	/*
	 * Turn off the RTC till we calcualte secs to till next isr
	 */
	T4CONbits.TON = 0;
#endif
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
#if 0
	timer = (u32)(((CLOCK_FREQ / 256) * current_isr_secs) - 1);
	PR4 = (u16)(timer & 0xffff);
	PR5 = (u16)((timer >> 16) & 0xffff);
	TMR4 = 0x00;
	TMR5 = 0x00;

	/*
	 * Turn timer back on
	 */
	T4CONbits.TON = 1;
#endif

	return(SUCCESS);
}

result_t rtc_set_alarm(ty_time_units units, u16 time, u8 nice, void (*expiry_function)(void))
{
	u16 tmp_minutes;
	u16 tmp_hours;
	u16 tmp_day;

	if(alarm_set) {
		LOG_E("Alarm is already set. Ingoring second request!");
		return(ERR_NO_RESOURCES);
	}

	alarm_datetime.hours   = current_datetime.hours;
	alarm_datetime.minutes = current_datetime.minutes;
	alarm_datetime.seconds = current_datetime.seconds;

	if(units == Minutes) {
		tmp_minutes = current_datetime.minutes + time;
		tmp_hours   = current_datetime.hours + tmp_minutes / 60;
		tmp_day     = current_datetime.day + tmp_hours / 24;

		alarm_datetime.minutes = tmp_minutes % 60;
		alarm_datetime.hours = tmp_hours % 24;

		LOG_D("rtc_set_alarm(+%d minutes) = %d:%d\n\r", time, alarm_datetime.hours, alarm_datetime.minutes);
	} else {
		LOG_E("Uncoded branch! Only processing Minute Interval Alarms\n\r");
	}
	alarm_set = TRUE;
	alarm_expiry_fn = expiry_function;

	return(SUCCESS);
}

static void check_alarm()
{
	LOG_D("check_alarm() Compare Current time %d:%d:%d with Alarm %d:%d:%d\n\r",
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds,
		alarm_datetime.hours,
		alarm_datetime.minutes,
		alarm_datetime.seconds);

	if(  (alarm_datetime.hours == current_datetime.hours)
	   &&(alarm_datetime.minutes == current_datetime.minutes)) {
//	   &&(alarm_datetime.seconds == current_datetime.seconds)) {
		alarm_set = FALSE;
		alarm_expiry_fn();
	}
}

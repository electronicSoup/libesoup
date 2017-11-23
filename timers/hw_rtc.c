/**
 *
 * \file libesoup/timers/hw_rtc.c
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
#define DEBUG_FILE TRUE

#include "libesoup_config.h"

#ifdef SYS_HW_RTC

#ifdef SYS_SERIAL_LOGGING
static const char *TAG = "RTC";

#include "libesoup/logger/serial_log.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

#endif // SYS_SERIAL_LOGGING

#include "libesoup/timers/rtc.h"

static uint8_t  datetime_valid = FALSE;

static expiry_function exp_fn = NULL;
static union sigval exp_data;

/*
 * Function prototypes
 */

void _ISR __attribute__((__no_auto_psv__)) _RTCCInterrupt(void)
{
	result_t rc;
	struct bcd_datetime datetime;

	RTCC_ISR_FLAG = 0;
	
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	rc =  rtc_get_datetime(&datetime);
	LOG_D("RTCC ISR ");
	rtc_print_datetime(&datetime);
#endif
#endif // SYS_SERIAL_LOGGING
	
	if (exp_fn) exp_fn(0x00, exp_data);
}

typedef enum {
	minutes_seconds_ptr = 0b00,
	weekday_hours_ptr = 0b01,
	month_day_ptr = 0b10,
	year_ptr = 0b11,
} ty_rtc_pointer;

/*
 * Make sure the secondary clock is powered on
 */
#if defined(__dsPIC33EP256MU806__)
result_t rtc_init(void)
{
	result_t            rc = SUCCESS;
	struct bcd_datetime datetime;
	
	datetime_valid = FALSE;

	/*
	 * Enable the Secondary 32.768KHz Oscillator.
	 * This uses the XC16 Compiler's builtin assembly function  
	 */	
	__builtin_write_OSCCONL(OSCCONL | 0x02);

	RTCC_PIN = DISABLED;            // Output on RTCC Pin disabled for the moment
	ALARM_OFF                       // Alarm disabled for the moment

	RTCC_ISR_FLAG = 0;
	RTCC_ISR_PRIOTITY = 0x07;
	RTCC_ISR_ENABLE = ENABLED;
		
	datetime.year_ten = 1;
	datetime.year_one = 8;
	datetime.month_ten = 0;
	datetime.month_one = 1;
	datetime.day_ten = 0;
	datetime.day_one = 1;
	datetime.hour_ten = 0;
	datetime.hour_one = 0;
	datetime.minute_ten = 0;
	datetime.minute_one = 0;
	datetime.second_ten = 0;
	datetime.second_one = 0;

	rc = rtc_set_datetime(&datetime);
	if(rc != SUCCESS) {
		return(rc);
	}
	RTCC_ON

	return(SUCCESS);
}
#endif // __dsPIC33EP256MU806__

#if defined(__dsPIC33EP256MU806__)
result_t rtc_set_datetime(struct bcd_datetime *datetime)
{
	uint16_t *data;
	
	data = (uint16_t *)datetime;

	RTCC_OFF                        // Disable the RTCC for the moment
	DATETIME_WRITE

	/*
	 * Set the datetime
	 */	
	VALUE_POINTER = year_ptr;
	RTCVAL = *data++;
	RTCVAL = *data++;
	RTCVAL = *data++;
	RTCVAL = *data++;
	
	DATETIME_LOCK
	RTCC_ON

	return(SUCCESS);
}
#endif // __dsPIC33EP256MU806__

#if defined(__dsPIC33EP256MU806__)
result_t rtc_get_datetime(struct bcd_datetime *datetime)
{
	uint16_t *data;
	
	data = (uint16_t *)datetime;

	VALUE_POINTER = year_ptr;
	
	*data++ = RTCVAL;
	*data++ = RTCVAL;
	*data++ = RTCVAL;
	*data++ = RTCVAL;
	
	return(SUCCESS);
}
#endif // __dsPIC33EP256MU806__

#if defined(__dsPIC33EP256MU806__)
result_t rtc_set_alarm(struct bcd_datetime *datetime, expiry_function fn, union sigval data)
{
	uint16_t *date;
	
	date = (uint16_t *)datetime;

	ALARM_OFF
		
	exp_fn = fn;
	exp_data = data;

	/*
	 * Set the datetime
	 */	
	ALARM_POINTER = year_ptr;
	ALRMVAL = *date++;
	ALRMVAL = *date++;
	ALRMVAL = *date++;
	ALRMVAL = *date++;
	
	ALCFGRPTbits.AMASK = RTC_ALARM_EVERY_MINUTE; //RTC_ALARM_EVERY_MINUTE;
	
	/*
	 * No repeat
	 */
	ALARM_CHIME = 0;
	ALARM_REPEAT = 0;

	RTCC_ISR_FLAG = 0;
	ALARM_ON

	return(SUCCESS);
}
#endif // __dsPIC33EP256MU806__

#ifdef SYS_SERIAL_LOGGING
void rtc_print_datetime(struct bcd_datetime *datetime)
{
	LOG_I("%d%d/%d%d/%d%d %d%d:%d%d:%d%d\n\r", 
		datetime->day_ten,
		datetime->day_one,
		datetime->month_ten,
		datetime->month_one,
		datetime->year_ten,
		datetime->year_one,
		datetime->hour_ten,
		datetime->hour_one,
		datetime->minute_ten,
		datetime->minute_one,
		datetime->second_ten,
		datetime->second_one);
	
}
#endif // SYS_SERIAL_LOGGING

#endif // SYS_HW_RTC

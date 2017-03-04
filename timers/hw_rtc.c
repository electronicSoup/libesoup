/**
 *
 * \file es_lib/timers/hw_rtc.c
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
#define DEBUG_FILE
#define TAG "RTC"

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/rtc.h"

static uint8_t  current_datetime_valid = FALSE;

/*
 * Function prototypes
 */

void _ISR __attribute__((__no_auto_psv__)) _RTCCInterrupt(void)
{
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "RTCC ISR\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
}

result_t rtc_update_current_datetime(uint8_t *data, uint16_t len)
{
	uint16_t year          = 0x00;
	uint16_t month_day     = 0x00;
	uint16_t weekday_hours = 0x00;
	uint16_t min_sec       = 0x00;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "rtc_update_current_datetime()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)

	if(len != 17) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Bad input datetime\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
		return(ERR_BAD_INPUT_PARAMETER);
	}
#if 0
	current_datetime.year    = ((data[0] - '0') * 1000) + ((data[1] -'0') * 100) + ((data[2] -'0') * 10) + (data[3] - '0');
	current_datetime.month   = ((data[4] - '0') * 10) + (data[5] - '0');
	current_datetime.day     = ((data[6] - '0') * 10) + (data[7] - '0');
	current_datetime.hours   = ((data[9] - '0') * 10) + (data[10] - '0');
	current_datetime.minutes = ((data[12] - '0') * 10) + (data[13] - '0');
	current_datetime.seconds = ((data[15] - '0') * 10) + (data[16] - '0');
#endif
	year = ((data[2] - '0') & 0x0f);
	year = (year << 4) | ((data[3] - '0') & 0x0f);

	month_day = (data[4] - '0') & 0x0f;
	month_day = (month_day < 4) | ((data[5] - '0') & 0x0f);
	month_day = (month_day < 4) | ((data[6] - '0') & 0x0f);
	month_day = (month_day < 4) | ((data[7] - '0') & 0x0f);

	weekday_hours = (data[9] - '0') & 0x0f;
	weekday_hours = (weekday_hours < 4) | ((data[10] - '0') & 0x0f);

	min_sec = (data[12] - '0') & 0x0f;
	min_sec = (min_sec < 4) | ((data[13] - '0') & 0x0f);
	min_sec = (min_sec < 4) | ((data[15] - '0') & 0x0f);
	min_sec = (min_sec < 4) | ((data[16] - '0') & 0x0f);

	RCFGCALbits.RTCWREN = 1;

//	asm ("__builtin_write_RTCWEN");
	RCFGCALbits.RTCPTR = 0b11;  // Year

	RTCVAL = year;
	RTCVAL = month_day;
	RTCVAL = weekday_hours;
	RTCVAL = min_sec;

	RCFGCALbits.RTCEN   = 1;
	RCFGCALbits.RTCWREN = 0;

	/*
	 * Just to test the alarm feature: +10 Mins
	 */
	min_sec = ((data[12] - '0') & 0x0f) + 1;
	min_sec = (min_sec < 4) | ((data[13] - '0') & 0x0f);
	min_sec = (min_sec < 4) | ((data[15] - '0') & 0x0f);
	min_sec = (min_sec < 4) | ((data[16] - '0') & 0x0f);

	ALCFGRPTbits.ALRMPTR = 0b10;
	ALRMVAL = year;
	ALRMVAL = month_day;
	ALRMVAL = weekday_hours;
	ALRMVAL = min_sec;

	ALCFGRPTbits.ALRMEN = 1;

	IFS3bits.RTCIF = 0;
	IEC3bits.RTCIE = 1;

	current_datetime_valid = TRUE;
#if 0
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Current datetime set to %d%d%d-%d:%d:%d\n\r",
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif //  if defined(SYS_LOG_LEVEL)
		current_datetime.year,
		current_datetime.month,
		current_datetime.day,
		current_datetime.hours,
		current_datetime.minutes,
		current_datetime.seconds);
#endif
	return(SUCCESS);
}

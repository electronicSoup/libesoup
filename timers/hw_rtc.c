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

#endif

#include "libesoup/timers/rtc.h"

#define DATETIME_WRITE  RCFGCALbits.RTCWREN = ENABLED;
#define DATETIME_LOCK   RCFGCALbits.RTCWREN = DISABLED;

/*
 * According to Microchip DS70584 RTCWREN has to be set to change RTCEN
 */
#define RTCC_ON 	RCFGCALbits.RTCWREN = ENABLED; \
                        RCFGCALbits.RTCEN = ENABLED;   \
			RCFGCALbits.RTCWREN = DISABLED;
#define RTCC_OFF 	RCFGCALbits.RTCWREN = ENABLED; \
                        RCFGCALbits.RTCEN = DISABLED;   \
			RCFGCALbits.RTCWREN = DISABLED;
#define VALUE_POINTER   RCFGCALbits.RTCPTR

#define ALARM_ON	ALCFGRPTbits.ALRMEN = ENABLED;
#define ALARM_OFF	ALCFGRPTbits.ALRMEN = DISABLED;
#define ALARM_POINTER   ALCFGRPTbits.ALRMPTR

static uint8_t  datetime_valid = FALSE;

/*
 * Function prototypes
 */

void _ISR __attribute__((__no_auto_psv__)) _RTCCInterrupt(void)
{
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("RTCC ISR\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
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
result_t rtc_init(void)
{
	result_t            rc = SUCCESS;
	struct bcd_datetime datetime;
	
	datetime_valid = FALSE;
	
	OSCCONbits.LPOSCEN = ENABLED;   // Enabled the secondary Crystal
	RCFGCALbits.RTCOE = DISABLED;   // Output on RTCC Pin disabled for the moment
	ALARM_OFF                       // Alarm disabled for the moment
	
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

result_t rtc_set_datetime(struct bcd_datetime *datetime)
{
	uint16_t *data;
	
	data = (uint16_t *)datetime;

#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("rtc_update_current_datetime()\n\r");
#endif
#endif // SYS_SERIAL_LOGGING

	RTCC_OFF                        // Disable the RTCC for the moment
	NVMKEY = 0x55;
	NVMKEY = 0xAA;
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


result_t rtc_set_alarm(struct bcd_datetime *datetime, expiry_function fn, union sigval data)
{
	uint16_t *date;
	
	date = (uint16_t *)datetime;

	ALARM_OFF

	/*
	 * Set the datetime
	 */	
	ALARM_POINTER = year_ptr;
	ALRMVAL = *date++;
	ALRMVAL = *date++;
	ALRMVAL = *date++;
	ALRMVAL = *date++;
	
	ALARM_ON

	return(SUCCESS);
}

#endif // SYS_HW_RTC

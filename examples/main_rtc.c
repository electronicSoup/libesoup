/**
 * @file libesoup/examples/main_rtc.c
 *
 * @author John Whitmore
 *
 * \brief Example main.c file to demonstrate Real Time Clock API.
 *
 * Core definitions required by electronicSoup Code Library
 *
 * Copyright 2017 electronicSoup Limited
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
#ifdef SYS_EXAMPLE_RTC

#define SET_ALARM

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE

static const char *TAG ="RTC_MAIN";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include "libesoup/timers/rtc.h"

void fn(timer_id timer, union sigval);

int main(void)
{
	uint16_t            loop_a;
	uint16_t            loop_b;
	result_t            rc;
#ifdef SYS_HW_RTC
	struct bcd_datetime datetime;
	union sigval        data;
#endif // SYS_HW_RTC
	/*
	 * Initialise the libesoup library
	 */
	rc = libesoup_init();
	if(rc != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Failed to initialise libesoup!\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
	}

	/*
	 * To test the RTC Module read the current datetime and log it,
	 * is serial logging is enabled
	 */
#ifdef SYS_HW_RTC
	rc = rtc_get_datetime(&datetime);
	if(rc != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Failed to read RTCC Datetime\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
	}
#endif // SYS_HW_RTC

#ifdef SYS_SERIAL_LOGGING
	rtc_print_datetime(&datetime);
#endif // SYS_SERIAL_LOGGING


	/*
	 * Set an Alarm for a minutes time
	 */
#ifdef SYS_HW_RTC
#ifdef SET_ALARM
        datetime.second_ten = 3;
	rc = rtc_set_alarm(&datetime, fn, data);
#endif // SET_ALARM
#endif
	for(loop_a = 0; loop_a < 0xffff; loop_a++) {
		for(loop_b = 0; loop_b < 0x8000; loop_b++) {
			Nop();
		}
	}

#ifdef SYS_HW_RTC
	rc = rtc_get_datetime(&datetime);
	if(rc != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Failed to read RTCC Datetime\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
	}
#endif // SYS_HW_RTC

#ifdef SYS_SERIAL_LOGGING
	LOG_D("Time %d%d:%d%d\n\r", datetime.minute_ten, datetime.minute_one, datetime.second_ten, datetime.second_one);
#endif // SYS_SERIAL_LOGGING
        while(1) {
        }
        return 0;
}

void fn(timer_id timer, union sigval data)
{
#ifdef SYS_SERIAL_LOGGING
	LOG_D("Expiry\n\r");
#endif // SYS_SERIAL_LOGGING
}

#endif // SYS_EXAMPLE_RTC

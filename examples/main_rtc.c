/**
 * @file libesoup/examples/main_rtc.c
 *
 * @author John Whitmore
 * 
 * \brief Example main.c file to demonstrate Real Time Clock API. 
 *
 * Core definitions required by electronicSoup Code Library
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
static const char *TAG ="RTC_MAIN";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include "libesoup/timers/rtc.h"

int main(void)
{
	result_t            rc;
	struct bcd_datetime datetime;

	/*
	 * Initialise the libesoup library
	 */
	rc = libesoup_init();
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Failed to initialise libesoup!\n\r");
#endif 
#endif // SYS_SERIAL_LOGGING

	rc = rtc_get_datetime(&datetime);
	if(rc != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Failed to read RTCC Datetime\n\r");
#endif 
#endif // SYS_SERIAL_LOGGING
	}
        
        while(1) {
        }
        return 0;
}

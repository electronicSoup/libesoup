/**
 *
 * libesoup/core.c
 *
 * File containing the function to initialise the libesoup library
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
 *******************************************************************************
 *
 */

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#ifdef XC16
static const char  __attribute__((unused)) *TAG = "CORE";
#elif __XC8
static const char  *TAG = "CORE";
#endif // XC16 elif XC8
#include "libesoup/logger/serial_log.h"
#endif  // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/boards/board.h"

#ifdef SYS_HW_TIMERS
#include "libesoup/timers/hw_timers.h"
#endif

#ifdef SYS_SW_TIMERS
#include "libesoup/timers/sw_timers.h"
#endif

#ifdef SYS_UART
#include "libesoup/comms/uart/uart.h"
#endif

#ifdef SYS_JOBS
#include "libesoup/jobs/jobs.h"
#endif

#ifdef SYS_HW_RTC
#include "libesoup/timers/rtc.h"
#endif

#ifdef SYS_SPI_BUS
#include "libesoup/comms/spi/spi.h"
#endif

#ifdef SYS_RAND
#include "libesoup/utils/rand.h"
#endif

#ifdef SYS_CHANGE_NOTIFICATION
#include "libesoup/processors/dsPIC33/change_notification/change_notification.h"
#endif // SYS_CHANGE_NOTIFICATION

/*
 * The Instruction Clock Frequency being used by the system.
 * 
 * SYS_CLOCK_FREQ is the frequency requested by libesoup_config.h but that
 * may not be possible, if invalid.
 */
uint32_t sys_clock_freq;

result_t libesoup_init(void)
{
	uint32_t loop;
#ifdef XC16
	result_t rc  __attribute__((unused)) = 0;
#else
	result_t rc = 0;
#endif

#if __XC8
#ifdef SYS_SERIAL_LOGGING
	TAG = TAG;
#endif // SYS_SERIAL_LOGGING
	rc = 0;
#endif
	
	cpu_init();
	
	/*
	 * Allow the clock to settle
	 */
	for(loop = 0; loop < 0x80000000; loop++) {
		Nop();
		Nop();
	}

#ifdef SYS_UART
	uart_init();
#endif

#ifdef SYS_SERIAL_LOGGING
        rc = serial_logging_init();
	RC_CHECK
#endif

#ifdef SYS_HW_TIMERS
	hw_timer_init();
#endif
	
#ifdef SYS_SW_TIMERS
	sw_timer_init();
#endif

#ifdef SYS_HW_RTC
	rc = rtc_init();
	RC_CHECK
#endif
		
#ifdef SYS_JOBS
	jobs_init();
#endif

#ifdef SYS_SPI_BUS
        spi_init();
#endif

#ifdef SYS_RAND
	random_init();
#endif

#ifdef SYS_CHANGE_NOTIFICATION
	rc = change_notifier_init();
	RC_CHECK
#endif // SYS_CHANGE_NOTIFICATION

	return(board_init());
}

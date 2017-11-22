/*
 *
 * libesoup/core.c
 *
 * File containing the function to initialise the libesoup library
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
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

#ifdef SYS_HW_TIMERS
#include "libesoup/timers/hw_timers.h"
#endif

#ifdef SYS_SW_TIMERS
#include "libesoup/timers/sw_timers.h"
#endif

#ifdef SYS_UART
#include "libesoup/comms/uart/uart.h"
#endif

#ifdef SYS_SERIAL_LOGGING
#include "libesoup/logger/serial_log.h"
#endif

#ifdef SYS_JOBS
#include "libesoup/jobs/jobs.h"
#endif

#ifdef SYS_HW_RTC
#include "libesoup/timers/rtc.h"
#endif

#ifdef SYS_SPI_BUS
#include "libesoup/utils/spi.h"
#endif


result_t libesoup_init(void)
{
	result_t rc = SUCCESS;
	
	cpu_init();

#ifdef SYS_UART
	uart_init();
#endif

#ifdef SYS_SERIAL_LOGGING
        rc = serial_logging_init();
        if (rc != SUCCESS) {
                /*
                 * What to do?
                 */
                return(rc);
        }
#endif

#ifdef SYS_HW_TIMERS
	hw_timer_init();
#endif
	
#ifdef SYS_SW_TIMERS
	sw_timer_init();
#endif

#ifdef SYS_HW_RTC
	rc = rtc_init();
        if (rc != SUCCESS) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed in initialise RTC Module\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
                return(rc);
        }
#endif
		
#ifdef SYS_JOBS
	jobs_init();
#endif
	
#ifdef SYS_SPI_BUS
        spi_init();
#endif
	return(rc);
}

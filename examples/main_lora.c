/**
 * \file libesoup/examples/main_lora.c
 *
 * Example main.c file for using AI Thinker Ra-01 LoRa Module .
 *
 * Copyright 2017-2019 electronicSoup Limited
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
#ifdef SYS_EXAMPLE_LORA

#include "libesoup/comms/uart/uart.h"

#if defined(SYS_SERIAL_LOGGING)
#define DEBUG_FILE
static const char *TAG = "Main";
#include "libesoup/logger/serial_log.h"
#endif
#include "libesoup/timers/delay.h"
#include "libesoup/timers/sw_timers.h"

#ifdef SYS_SW_TIMERS
static void expiry(timer_id timer, union sigval);
#endif

/*
 *
 */
int main()
{
        result_t         rc;
#ifdef SYS_SW_TIMERS
	timer_id         timer;
	struct timer_req request;
#endif

        rc = libesoup_init();

	if (rc < 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("ERROR Oops\n\r");
#endif
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Testing\n\r");
#endif // DEBUG
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
	LOG_E("Test of ERROR Message\n\r");
#endif
	/*
	 * If the build includes SW Timers start a ping pong timer one
	 * can load the serial logging buffer and the other can check
	 * that it's emptied
	 */
#ifdef SYS_SW_TIMERS
	request.period.units    = Seconds;
	request.period.duration = 10;
	request.type            = repeat_expiry;
	request.exp_fn          = expiry;
	request.data.sival_int  = 0x00;

	timer = sw_timer_start(&request);
	if(rc < 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start SW Timer\n\r");
#endif
	}
#endif

        while(1) {
		libesoup_tasks();
        }

        return(0);
}

/*
 * Expiry Function if SYS_SW_TIMERS defined
 */
#ifdef SYS_SW_TIMERS
static void expiry(timer_id timer, union sigval data)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Expiry\n\r");
#endif
}
#endif // SYS_SW_TIMERS
#endif // SYS_EXAMPLE_LORA

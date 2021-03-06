/**
 * @file libesoup/timers/delay.c
 *
 * @author John Whitmore
 *
 * @brief Functionality for delaying the uC
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
#define DEBUG_FILE

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#if defined(XC16)
__attribute__ ((unused)) static const char *TAG = "DELAY";
#elif defined(__XC8)
static const char *TAG = "DELAY";
#endif // XC16 XC8
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

/*
 * Code should only be compiled if SYS_HW_TIMERS is enabled in libesoup_config.h
 */
#ifdef SYS_HW_TIMERS

#include "libesoup/errno.h"
#include "libesoup/timers/hw_timers.h"

static volatile uint8_t delay_over;

/*
 * Hardware timer expiry function
 */
void hw_expiry_function(timer_id timer, union sigval data)
{
	delay_over = TRUE;
}

/*
 * delay function implementation
 */
result_t delay(struct period *period)
{
	result_t           rc;
	struct timer_req   timer_request;

	timer_request.period.units    = period->units;
	timer_request.period.duration = period->duration;
	timer_request.type            = single_shot_expiry;
	timer_request.exp_fn          = hw_expiry_function;

        delay_over = FALSE;
        rc = hw_timer_start(&timer_request);
	RC_CHECK

        while(!delay_over) {
#if defined(XC16)
                __asm__ ("CLRWDT");
#elif defined(__XC8)
		asm("CLRWDT");
#else
#error "Need a nop of watchdog macro for compiler"
#endif
        }
	return(0);
}

result_t delay_mS(uint16_t duration)
{
	struct period period = {mSeconds, duration};

	delay(&period);
	return(SUCCESS);
}

#if !defined(__dsPIC33EP256GP502__)
result_t delay_uS(uint16_t duration)
{
	struct period period = {uSeconds, duration};

	delay(&period);
	return(SUCCESS);
}
#endif // __dsPIC33EP256GP502__
#endif // #ifdef SYS_HW_TIMERS

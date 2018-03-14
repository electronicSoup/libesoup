/**
 *
 * \file libesoup/utils/sleep.c
 *
 * Functionality for delaying the uC
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
#define DEBUG_FILE

#include <p33EP256MU806.h>

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "DELAY";
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
result_t delay(ty_time_units units, uint16_t duration)
{
	timer_id           hw_timer;
	struct timer_req   timer_request;

	/*
	 * If the delay is uSeconds compensate for the Instruction overhead of
	 * starting a HW Timer. If the timer is less then the overhead there is
	 * no point in creating a HW Timer so spin in a Nop loop.
	 */
#if defined(__dsPIC33EP256MU806__)
	if(units == uSeconds) {	
		if(duration > HW_TIMER_OVERHEAD) {
			duration -= HW_TIMER_OVERHEAD;
		} else {
			/*
			 * The delay passed in is too small to accurately calculate
			 * The calculations depend on sys_clock_freq and multiply and
			 * divide calculations will cost more then the timer.
			 * Roll it by hand!
			 */
			return(ERR_RANGE_ERROR);
		}
	}
#endif // defined(__dsPIC33EP256MU806__)
	TIMER_INIT(hw_timer);
	timer_request.units          = units;
	timer_request.duration       = duration;
	timer_request.type           = single_shot;
	timer_request.exp_fn         = hw_expiry_function;
	timer_request.data.sival_int = 0;

        delay_over = FALSE;
        hw_timer = hw_timer_start(&hw_timer, &timer_request);

        while(!delay_over) {
#if defined(XC16)
                __asm__ ("CLRWDT");
#elif defined(__XC8)
		asm("CLRWDT");
#else
#error "Need a nop of watchdog macro for compiler"
#endif
        }
	return(SUCCESS);
}

#endif // #ifdef SYS_HW_TIMERS

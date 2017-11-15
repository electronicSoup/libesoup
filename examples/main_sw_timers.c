/*
 * File:   main.c
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

/*
 * Set up the configuration words of the processor:
 */

#include "libesoup_config.h"

//#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/sw_timers.h"

static void expiry(timer_t timer_id, union sigval);

int main(void)
{
        result_t     rc;
        union sigval data;
	timer_t      timer;

	cpu_init();

        /*
         * Microchip platforms depend on a Hardware timer for the regular
         * tick so Hardware timers have to be initialised, and Serial logging
	 * if required.
         */

        rc = sw_timer_start(SECONDS_TO_TICKS(30), expiry, data, &timer);
        
        if(rc != SUCCESS) {
		// Error Condition
        }
        
        while(1) {
#if defined(XC16) || defined(__XC8)
                CHECK_TIMERS();
#endif
        }
        return 0;
}

#if defined(XC16)
static void expiry(timer_t timer_id  __attribute__((unused)), union sigval data __attribute__((unused)))
#elif defined(__XC8)
static void expiry(timer_t timer_id, union sigval data)
#endif
{
	// Timer has expired
}

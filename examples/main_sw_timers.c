/**
 * \file   main.c
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
 */

/*
 * Set up the configuration words of the processor:
 */

#include "libesoup_config.h"

#include "libesoup/timers/sw_timers.h"

static void expiry(timer_id timer_id, union sigval);

int main(void)
{
        result_t         rc;
	struct timer_req request;
	timer_id         timer;

	rc = libesoup_init();

	TIMER_INIT(timer);

	request.units          = Seconds;
	request.duration       = 30;
	request.type           = single_shot;
	request.exp_fn         = expiry;
	request.data.sival_int = 0;
	
        rc = sw_timer_start(&timer, &request);
        
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
static void expiry(timer_id timer  __attribute__((unused)), union sigval data __attribute__((unused)))
#elif defined(__XC8)
static void expiry(timer_id timer, union sigval data)
#endif
{
	// Timer has expired
}

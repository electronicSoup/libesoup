/**
 * @file libesoup/examples/main_sw_timers.c
 *
 * @author John Whitmore
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

/*
 * Set up the configuration words of the processor:
 */

#include "libesoup_config.h"

#include "libesoup/timers/delay.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/timers/sw_timers.h"

static void expiry(timer_id timer_id, union sigval);

int main(void)
{
        result_t         rc;
	struct timer_req request;
	timer_id         timer;
	struct period    period;

	rc = libesoup_init();

#if defined(__dsPIC33EP256MU806__)
	rc = gpio_set(RD3, GPIO_MODE_DIGITAL_OUTPUT, 0);
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        TRISEbits.TRISE0 = GPIO_INPUT_PIN;
        TRISEbits.TRISE1 = GPIO_OUTPUT_PIN;
        TRISEbits.TRISE2 = GPIO_OUTPUT_PIN;
        TRISEbits.TRISE3 = GPIO_OUTPUT_PIN;
#endif
	period.units     = Seconds;
	period.duration  = 5;
	delay(&period);
	
	request.period.units    = Seconds;
	request.period.duration = 30;
	request.type            = single_shot_expiry;
	request.exp_fn          = expiry;
	request.data.sival_int  = 0;
	
#if defined(__dsPIC33EP256MU806__)	
	LATDbits.LATD3 = 1;
#endif
        timer = sw_timer_start(&request);
        
        if(timer < 0) {
		// Error Condition
        }
        
        while(1) {
		libesoup_tasks();
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
#if defined(__dsPIC33EP256MU806__)	
	LATDbits.LATD3 = 0;
#endif
}

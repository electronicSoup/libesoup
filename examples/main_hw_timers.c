/**
 * @file libesoup/examples/main_hw_timers.c
 *
 * @author John Whitmore
 *
 * Example main.c file to demonstrate Hardware timers.
 *
 * Core definitions required by electronicSoup Code Library
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
#ifdef SYS_EXAMPLE_HW_TIMERS

#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"
#include "libesoup/timers/hw_timers.h"
#include "libesoup/timers/delay.h"

//#define DELAY_TEST
#define HW_TIMER_TEST
//#define HW_TIMER_REPEAT

/*
 * Forward declaration of our Hardware timer expiry function, which will be
 * called when the timer expires.
 */
void exp_func(timer_id timer, union sigval data);

int main(void)
{
        result_t         rc;
        timer_id         timer;
        struct timer_req request;
        struct period    period;

    	/*
         * Initialise the libesoup library
         */
        rc = libesoup_init();
    	if(rc < 0) {
                /*
		 * Error condition
		 */
        }

        /*
         *
         */
#if defined(__dsPIC33EP256MU806__)
	rc = gpio_set(RD3, GPIO_MODE_DIGITAL_OUTPUT, 0);
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	rc = gpio_set(RD4, GPIO_MODE_DIGITAL_OUTPUT, 0);
	rc = gpio_set(RD5, GPIO_MODE_DIGITAL_OUTPUT, 0);
#elif defined(__18F4585)
	rc = gpio_set(PRC6, GPIO_MODE_DIGITAL_OUTPUT, 0);
	rc = gpio_set(PRC7, GPIO_MODE_DIGITAL_OUTPUT, 0);
#endif // (__18F4585)

#ifdef HW_TIMER_TEST
	period.units     = Seconds;
	period.duration  = 5;
	delay(&period);

	/*
	 * Documentation:
	 *
	 * dsPIC33 @ 60M 500uS timer gives 504uS  Delta  4uS
	 * dsPIC33 @ 30M 500uS timer gives 504uS  Delta  4uS
	 * dsPIC33 @  8M 500uS timer gives 508uS  Delta  8uS
	 */
	request.period.units          = mSeconds;
	request.period.duration       = 20;

	/*
	 * pic18F4585 or more specifically XC8 compiler doesn't do
	 * recursion so not repeating timers, single_shot only
	 */
#if defined(__18F4585)
	request.type           = single_shot_expiry;
#elif defined(HW_TIMER_REPEAT)
	request.type           = repeat_expiry;
#else
	request.type           = single_shot_expiry;
#endif
	request.exp_fn         = exp_func;
	request.data.sival_int = 0;

#if defined(__dsPIC33EP256MU806__)
	LATDbits.LATD3 = 1;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	LATDbits.LATD4 = 1;
	LATDbits.LATD5 = 1;
#elif defined(__18F4585)
	LATCbits.LATC6 = 1;
	LATCbits.LATC7 = 1;
#endif
        timer = hw_timer_start(&request);
        if(timer < 0) {
	        /*
		 * Handle the error condition.
		 */
		Nop();
        }
#endif

        while(1) {
#ifdef DELAY_TEST
		/*
	         * Documentation:
	         *
		 * The delay routine will return ERR_RANGE_ERROR if the duration
		 * passed in is shorter then it can safely obtain.
	         */
		LATDbits.LATD3  = 0;
		period.units    = uSeconds;
		period.duration = 10;
		rc = delay(&period);
		if(rc != SUCCESS) {
			LATDbits.LATD0 = 1;
		}
		LATDbits.LATD3 = 1;
		rc = delay(&period);
		if(rc != SUCCESS) {
			LATDbits.LATD0 = 1;
		}
#endif
		Nop();
        }
        return 0;
}

void exp_func(timer_id timer, union sigval data)
{
#if defined(__dsPIC33EP256MU806__)
#ifdef HW_TIMER_REPEAT
	LATDbits.LATD3 = ~PORTDbits.RD3;
#else
	LATDbits.LATD3 = 0;
#endif
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
	LATDbits.LATD4 = 0;
	LATDbits.LATD5 = 0;
#elif defined(__18F4585)
	LATCbits.LATC6 = 0;
	LATCbits.LATC7 = 0;
#endif
}
#endif // SYS_EXAMPLE_HW_TIMERS

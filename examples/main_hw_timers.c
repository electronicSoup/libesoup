/**
 * @file libesoup/examples/main_hw_timers.c
 *
 * @author John Whitmore
 * 
 * Example main.c file to demonstrate Hardware timers. 
 *
 * Core definitions required by electronicSoup Code Library
 *
 * Copyright 2017 2018 electronicSoup Limited
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

#include "libesoup/timers/hw_timers.h"

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
	union sigval     data;

	/*
	 * Initialise the libesoup library
	 */
	rc = libesoup_init();

        /*
         * set pin RE0 as an Input pin
         */
#if defined(__dsPIC33EP256MU806__)
        ANSELEbits.ANSE0 = 0;
        TRISEbits.TRISE0 = 1;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        TRISEbits.TRISE0 = 1;
#endif

        /*
         * Set Pin RE1 as an Output Pin
         */
#if defined(__dsPIC33EP256MU806__)
        ANSELEbits.ANSE1 = 0;
        TRISEbits.TRISE1 = 0;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        TRISEbits.TRISE1 = 0;
#endif

        /*
         * Set Pin RE2 as an Output Pin
         */
#if defined(__dsPIC33EP256MU806__)
        ANSELEbits.ANSE2 = 0;
        TRISEbits.TRISE2 = 0;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        TRISEbits.TRISE2 = 0;
#endif

        /*
         * Set Pin RE2 as an Output Pin
         */
#if defined(__dsPIC33EP256MU806__)
        ANSELEbits.ANSE3 = 0;
        TRISEbits.TRISE3 = 0;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        TRISEbits.TRISE3 = 0;
#endif

#if defined(__18F4585)
	/*
	 * All AD pins configured as Digital
	 */
	ADCON1 = 0x0f;

	LATA = 0x00;
	TRISA = 0x00;

	TRISDbits.TRISD5 = 0;
	LATDbits.LATD5 = 1;

	TRISDbits.TRISD6 = 1;

	INTCONbits.GIE = 1;    // Enable Interrupts
	INTCONbits.PEIE = 1;   // Enable Periphal Interrupts
#endif // (__18F4585)
        
	request.units    = Seconds;
	request.duration = 5;
	request.type     = single_shot;
	request.exp_fn   = exp_func;
	request.data     = data;

        rc = hw_timer_start(&timer, &request);
        if(rc != SUCCESS) {
	        /*
		 * Handle the error condition.
		 */
        }
	
#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        LATEbits.LATE3 = 1;
#endif
        	        
        while(1) {
        }
        return 0;
}

void exp_func(timer_id timer, union sigval data)
{
#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
        LATEbits.LATE3 = ~LATEbits.LATE3;
#endif
}

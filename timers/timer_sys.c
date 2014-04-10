/**
 *
 * \file es_can/timers/timers_sys.c
 *
 * Timer functionalty of electronicSoup CAN code Library
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
#include "es_lib/core.h"
#include "system.h"
#include <stdio.h>

#define DEBUG_FILE
#include "es_lib/logger/serial.h"

#if LOG_LEVEL < NO_LOGGING
    #define TAG "TIMER"
#endif

#define NUMBER_OF_TIMERS 20

static UINT16 timer_counter = 0;

volatile BOOL timer_tick = FALSE;

typedef struct {
    BOOL active;
    UINT16 expiry_count;
    expiry_function function;
    union sigval expiry_data;
} sys_timer_t;

#pragma udata
sys_timer_t timers[NUMBER_OF_TIMERS];

#if defined( __C30__ )
void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void)
{
    IFS0bits.T1IF = 0;
    TMR1 = 0x00;

    timer_tick = TRUE;
}
#endif

void init_timer(void)
{
    BYTE loop;

    /*
     * Initialise our Data Structures
     */
    for(loop=0; loop < NUMBER_OF_TIMERS; loop++)
    {
        timers[loop].active = FALSE;
        timers[loop].expiry_count = 0;
        timers[loop].function = (expiry_function)NULL;
    }

#if defined(__18CXX)
    /*
     * Timer 0 set up
     */
    T0CONbits.T08BIT = 0;         // 16 bit opperation
    T0CONbits.T0CS = 0;           // Timer 0 Off internal clock
    T0CONbits.PSA = 0;            // Enable prescaler for Timer 0
    T0CONbits.T0PS0 = 0;          //
    T0CONbits.T0PS1 = 0;          //   Divide the clock by 256 prescaler
    T0CONbits.T0PS2 = 1;          //

    TMR0H = 0xfd;
    TMR0L = 0x90;                 // Should give 5mS

    INTCON2bits.TMR0IP = 1;       // Set Timer to High Priority ISR
    T0CONbits.TMR0ON = 1;         // Enable Timer 0

    /*
     * Enable interrupts from Timer 0
     */
    INTCONbits.TMR0IE = 1;        // Timer 0 Interrupt Enable
    INTCONbits.TMR0IF = 0;        // Clear the Timer 0 interupt flag

#elif defined( __C30__ )
    // Initialise Timer 1 for use as the 5mS timer
    T1CONbits.TCS = 0;      // Internal FOSC/2
    T1CONbits.TCKPS1 = 0;   // Divide by 8
    T1CONbits.TCKPS0 = 1;
//    T1CONbits.TCKPS = 0x02;  // Divide by 64
    
    // TODO
    PR1 = ((CLOCK_FREQ / 8) / 1000) * SYSTEM_TICK_ms;

    TMR1 = 0x00;
//    PR1 = 0xffff;
    IEC0bits.T1IE = 1;
    
    T1CONbits.TON = 1;
#endif
}

void tick(void)
{
    BYTE loop;
    expiry_function function;
    union sigval data;

    timer_tick = FALSE;
    timer_counter++;

    /*
     * Check for expired timers
     */
    for(loop=0; loop < NUMBER_OF_TIMERS; loop++)
    {
        if (  (timers[loop].active)
            &&(timers[loop].expiry_count == timer_counter) )
        {
            timers[loop].active = FALSE;

            function = timers[loop].function;
            data = timers[loop].expiry_data;

            timers[loop].expiry_count = 0;
            timers[loop].function = (expiry_function)NULL;
            function(data);
        }
    }
}

result_t start_timer(UINT16 ticks,
        expiry_function function,
        union sigval data,
        es_timer *timer)
{
    timer_t loop;

    if(timer->status != INACTIVE) {
        DEBUG_E("Timer already Active\n\r");
        return(ERR_TIMER_ACTIVE);
    }

    /*
     * Find the First empty timer
     */
    for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
        if (!timers[loop].active) {
            timers[loop].active = TRUE;

            if( (0xFFFF - timer_counter) > ticks) {
                timers[loop].expiry_count = timer_counter + ticks;
            } else {
                timers[loop].expiry_count = ticks - (0xFFFF - timer_counter);
            }
            timers[loop].function = function;
            timers[loop].expiry_data = data;

            timer->status = ACTIVE;
            timer->timer_id = loop;
            return(SUCCESS);
        }
    }
    DEBUG_E("No Timers Free");

    return(ERR_NO_RESOURCES);
}

result_t cancel_timer(es_timer *tmr)
{
    if(tmr->status == ACTIVE) {
        if(timers[tmr->timer_id].active) {
            timers[tmr->timer_id].active = FALSE;
            timers[tmr->timer_id].expiry_count = 0;
            timers[tmr->timer_id].function = (expiry_function) NULL;
        }
        tmr->status = INACTIVE;
    }
    return(SUCCESS);
}

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
#include "es_can/core.h"
#include "system.h"
#include <stdio.h>

#define DEBUG_FILE
#include "es_can/logger/serial.h"

#if LOG_LEVEL < NO_LOGGING
    #define TAG "TIMER"
#endif

#define NUMBER_OF_TIMERS 20

static UINT16 timer_counter = 0;

volatile BOOL timer_tick = FALSE;

extern void err(char *);

static result_t pri_start_timer(UINT16 ticks,
                expiry_function function,
                union sigval data,
                es_timer *timer,
                BOOL system);

typedef struct{
    struct {
        BYTE active : 1;
        BYTE system : 1;
    } bit_field;
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
        timers[loop].bit_field.active = 0;
        timers[loop].bit_field.system = 0;
        timers[loop].expiry_count = 0;
        timers[loop].function = (expiry_function)NULL;
//        timers[loop].expiry_data = NULL;
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
    
    // TODO
    PR1 = (SYSTEM_TICK_ms /1000) / ((CLOCK_FREQ/2)/8);

    TMR1 = 0x00;
    PR1 = 0x2724;
    IEC0bits.T1IE = 1;
    
    T1CONbits.TON = 1;
#endif
}

void tick(void)
{
    BYTE loop;
    expiry_function function;
    union sigval data;

    timer_counter++;

    /*
     * Check for expired timers
     */
    for(loop=0; loop < NUMBER_OF_TIMERS; loop++)
    {
        if (  (timers[loop].bit_field.active)
            &&(timers[loop].expiry_count == timer_counter) )
        {
            timers[loop].bit_field.active = 0;
            timers[loop].bit_field.system = 0;

            function = timers[loop].function;
            data = timers[loop].expiry_data;

            timers[loop].expiry_count = 0;
            timers[loop].function = (expiry_function)NULL;
//            timers[loop].expiry_data = 0x00;
            function(data);
        }
    }
}

static result_t pri_start_timer(UINT16 ticks,
                                expiry_function function,
                                union sigval data,
                                es_timer *timer,
                                BOOL system)
{
    timer_removethis_t loop;

    if(timer->status != INACTIVE) {
        return(ERR_TIMER_ACTIVE);
    }

    /*
     * Find the First empty timer
     */
    for(loop=0; loop < NUMBER_OF_TIMERS; loop++) {
        if (!timers[loop].bit_field.active) {
            timers[loop].bit_field.active = 1;
            timers[loop].bit_field.system = system;

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
    /* TODO
     * This error code on a Node tries to send a Net Log Message
     * on the Dongle ????
     */
    err("Error Node Out of Timers");

    return(ERR_NO_RESOURCES);
}

result_t start_timer(UINT16 ticks,
                     expiry_function function,
                     union sigval data,
                     es_timer *timer)
{
    return(pri_start_timer(ticks, function, data, timer, TRUE));
}

result_t app_start_timer(UINT16 ticks,
                         expiry_function function,
                         union sigval data,
                         es_timer *timer)
{
    return(pri_start_timer(ticks, function, data, timer, FALSE));
}

result_t cancel_timer(es_timer *tmr)
{
    if(tmr->status == ACTIVE) {
        if(timers[tmr->timer_id].bit_field.active) {
            timers[tmr->timer_id].bit_field.active = 0;
            timers[tmr->timer_id].bit_field.system = 0;
            timers[tmr->timer_id].expiry_count = 0;
            timers[tmr->timer_id].function = (expiry_function) NULL;
//            timers[tmr->timer_id].expiry_data = 0x00;
        }
        tmr->status = INACTIVE;
    }
    return(SUCCESS);
}

void cancel_app_timers(void)
{
    BYTE loop;

    for(loop=0; loop < NUMBER_OF_TIMERS; loop++)
    {
        if (!timers[loop].bit_field.system)
        {
            timers[loop].bit_field.active = 0;
            timers[loop].bit_field.system = 0;
            timers[loop].expiry_count = 0;
            timers[loop].function = (expiry_function)NULL;
//            timers[loop].expiry_data = 0x00;
        }
    }
}

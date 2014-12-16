/**
 *
 * \file es_lib/timers/timers.h
 *
 * Timer function prototypes of the electronicSoup Cinnamon Bun
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
/*
 * A note on timers. The timer structures used in this electronicSoup
 * CinnamonBun Library are not as simple as they could be, given the simplicity
 * usually aspired to in simple embedded code. The structures and functions
 * used are complicated more then necessary because the timer API is modeled on
 * the Linux timer API. This is not for any other reason then the C code
 * written to use this library code can be compiled and run under Linux. This
 * has the advantage that protocols written for the CinnamonBun can easily be
 * ported to the RaspberryPi platform.
 */
#ifndef TIMERS_H
#define TIMERS_H

#ifdef __18F2680
/*
 * Calculate the 16 bit value that will give us an ISR for the system tick
 * duration.
 */
#define TMR0H_VAL ((UINT8) ( (0xFFFF - (UINT16)( SYSTEM_TICK_ms * ( (CLOCK_FREQ / 2) / 256) * 1000 ) ) >> 8)  & 0xFF)
#define TMR0L_VAL ((UINT8) ( 0xFFFF - (UINT16)( SYSTEM_TICK_ms * ( (CLOCK_FREQ / 2) / 256) * 1000 ) )  & 0xFF)

//	TMR0H = 0xfd;
//	TMR0L = 0x90; // Should give 5mS      623
#endif //__18F2680

/*
 * Enumerated type for the current status of a timer in the system. A timer
 * is either Active or it's not.
 */
typedef enum {
    INACTIVE = 0x00,
    ACTIVE,
} timer_status_t;

/*
 * Simple macro to initialise the current statusof a timer to inactive.
 * A timer should always be initialsed to an inactive status before it is used
 * otherwise the timer might appear to be already active
 */
#define TIMER_INIT(timer) timer.status = INACTIVE;

/*
 * timer_t Timer identifier
 *
 * A Timer ID is used to identify a timer. It should not be used directly
 * by code but only used by timer.c. It is part of the es_timer structure so
 * we'll just define it here.
 */
#ifdef MCP
typedef u8 timer_t;
#endif

/*
 * The actual timer structure is simply the timer identifier and it's status.
 */
typedef struct
{
	timer_status_t status;
	timer_t        timer_id;
} es_timer;

/*
 * SECONDS_TO_TICKS
 *
 * Convience Macro to convert seconds to system timer ticks
 */
#define SECONDS_TO_TICKS(s)  ((s) * (1000 / SYSTEM_TICK_ms))

/*
 * MILLI_SECONDS_TO_TICKS
 *
 * Convience Macro to convert milliSeconds to system timer ticks
 */
#define MILLI_SECONDS_TO_TICKS(ms) ((ms < SYSTEM_TICK_ms) ? 1 : (ms / SYSTEM_TICK_ms))

#ifdef MCP
/*
 * union sigval
 *
 * The sigval union comes straight from the Linux timer API which is why this
 * definition if encased in a test for MCP definition. The union is passed to
 * the expiry function if the timer expires. It can either carry a 16 bit
 * integer value or a pointer.
 */
union sigval {          /* Data passed with notification */
           u16     sival_int;         /* Integer value */
           void   *sival_ptr;         /* Pointer value */
};
#endif

/**
 * \brief typedef expiry_function
 *
 * When a timer is created an expiry function is passed to the creation function.
 * The CinnamonBun timer code executes this expiry_function when the timer
 * expires.
 *
 * The expiry_function is a pointer to a function which accepts as parameter a
 * pointer to a BYTE containing the timer_id identifer of the timer which has
 * expired and the union sigval, defined above, which can be used to pass data
 * to the expiry funcion. The expiry function is declared void and will not
 * return anything to the timer library code.
 */
typedef void (*expiry_function)(timer_t timer_id, union sigval);


#ifdef MCP

#ifdef __18F2680
        #define PIC18_TIMER_ISR() { \
                if (INTCONbits.TMR0IF) { \
                        INTCONbits.TMR0IF = 0; \
                        TMR0H = TMR0H_VAL;          \
                        TMR0L = TMR0L_VAL;          \
                        timer_tick = 1;        \
                } \
        }
#endif //__18F2680

    /*
     * CHECK_TIMERS()
     *
     * this CHECK_TIMERS() macro should be placed in the main loop of you code
     * so taht it calls the library timer functionality when ever the tick
     * interrup has occured.
     */
    #define CHECK_TIMERS()  if(timer_ticked) timer_tick();

        extern volatile BOOL timer_ticked;

    /*
     * timer_init()
     *
     * This function should be called to initialise the timer functionality
     * of the electronicSoup CinnamonBun Library. It initialises all data
     * structures. The project's system.h header file should define the number
     * of timers the library should define space for an manage. See the
     * definition of NUMBER_OF_TIMERS in the example system.h file in the
     * es_lib directory.
     */
    extern void timer_init(void);

    /*
     * timer_tick()
     *
     * This function should not be called directly but called with the
     * CHECK_TIMERS macro defined above. It should be called only when the
     * timer interrupt has fired for a timer tick. The period of the timer tick
     * is defined in core.h.
     */
    extern void timer_tick(void);
#endif

/*
 * timer_start()
 * 
 */
extern result_t timer_start(u16 duration, expiry_function fn, union sigval data, es_timer *timer);
extern result_t timer_cancel(es_timer *timer);

#endif


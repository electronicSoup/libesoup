/**
 * @file libesoup/processors/pic18f4585.c
 *
 * @author John Whitmore
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
#if defined(__18F4585)

#include "libesoup_config.h"
#include "libesoup/timers/clock.h"

extern void check_timer(uint8_t timer);

#pragma config WDT = OFF
#pragma config MCLRE = OFF
#pragma config DEBUG = OFF
#pragma config LVP = OFF
//#pragma config OSC = IRCIO67    // Internal Osc with IO on Ports
#pragma config OSC = HS         //Crystal
#pragma config IESO = OFF
#pragma config XINST = OFF
#pragma config PWRT = OFF
#pragma config BOREN = OFF
#pragma config STVREN = ON
#pragma config PBADEN = OFF
#pragma config CP0 = OFF
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CPD = OFF
#pragma config CPB = OFF
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config EBTR0 = OFF
#pragma config EBTR1 = OFF
#pragma config EBTR2 = OFF

extern void pic18f_timer_isr(uint8_t timer);
extern void pic18f_uart_isr(void);

void cpu_init(void)
{
        /*
         * clock_init does nothing for this chip but call it to clear a 
         * compiler warning.
         */
        clock_init();
	
        RCONbits.IPEN = 0; // No Interrupt priority level
	INTCONbits.GIE = 1;    // Enable Interrupts
	INTCONbits.PEIE = 1;   // Enable Peripheral Interrupts
}

/*
 * Two Interrupt Service Routines
 */
#if 0
void interrupt high_priority high_isr(void)
{
//	serial_isr();
//	timer_isr();
}
#endif

#if 0
void interrupt low_priority low_isr(void)
{
	count++;
	if(count > ROLLOVER) {
		HEARTBEAT_LED = ~HEARTBEAT_LED;
		count = 0;
	}
#ifdef SERIAL
	serial_isr();
#endif
#ifdef TIME
	timer_isr();
#endif
}
#endif // 0

void interrupt tc_int(void)
{
        if (TMR0IE && TMR0IF) {
                TMR0IF = 0;
                TMR0IE = 0;
                T0CONbits.TMR0ON = 0;
#ifdef SYS_SW_TIMERS
                check_timer(TIMER_0);
#endif
        }

        if (TMR1IE && TMR1IF) {
                TMR1IF=0;
                TMR1IE = 0;
                T1CONbits.TMR1ON = 0;
#ifdef SYS_SW_TIMERS
                check_timer(TIMER_1);
#endif
        }

#if (SYS_LOG_LEVEL != NO_LOGGING)
	if(PIR1bits.TXIF) {
                pic18f_uart_isr();
        }
#endif // (SYS_LOG_LEVEL != NO_LOGGING)
        // process other interrupt sources here, if required
}

void interrupt low_priority tc_clr(void) {
// process any other low priority sources here
}

#endif // __18F4585

/**
 * @file libesoup/processors/pic18f4585.c
 *
 * @author John Whitmore
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
#if defined(__18F4585)

#include "libesoup_config.h"


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

#include "libesoup/timers/time.h"

extern void check_timer(timer_id timer);

extern void pic18f_timer_isr(uint8_t timer);
extern void pic18f_uart_isr(void);

void cpu_init(void)
{
        RCONbits.IPEN   = 0;  // No Interrupt priority level
	INTCONbits.GIE  = 1;  // Enable Interrupts
	INTCONbits.PEIE = 1;  // Enable Peripheral Interrupts

	sys_clock_freq = CRYSTAL_FREQ;
}

void interrupt tc_int(void)
{
#ifndef SYS_CAN_BUS
	LATBbits.LATB2 = ~LATBbits.LATB2;
#endif // SYS_CAN_BUS
	
        if (TMR0IE && TMR0IF) {
                TMR0IF = 0;
                TMR0IE = 0;
                T0CONbits.TMR0ON = 0;
                check_timer(TIMER_0);
        }

        if (TMR1IE && TMR1IF) {
                TMR1IF=0;
                TMR1IE = 0;
                T1CONbits.TMR1ON = 0;
                check_timer(TIMER_1);
        }

#ifdef SYS_UART
	if(PIR1bits.TXIF) {
                pic18f_uart_isr();
        }
#endif
        // process other interrupt sources here, if required
}

void interrupt low_priority tc_clr(void) {
// process any other low priority sources here
}

#endif // __18F4585

/**
 * @file libesoup/processors/dsPIC33EP256MU806.c
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
#if defined(__dsPIC33EP256MU806__)

#include "libesoup_config.h"

/*
 * Set up the configuration words of the processor:
 *
 * file:///opt/microchip/xc16/v1.26/docs/config_docs/33EP256MU806.html
 */
#pragma config GWRP = OFF
#pragma config GSS = OFF
#pragma config GSSK = OFF
//#pragma config FNOSC = FRC
#pragma config FNOSC = PRI   // Primary oscillator
//#pragma config FNOSC = SOSC  // Secondary oscillator
#pragma config IESO = OFF
#pragma config POSCMD = HS
#pragma config IOL1WAY = OFF
#pragma config FCKSM = CSECMD
#pragma config FWDTEN = OFF
#pragma config WINDIS   = OFF              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config BOREN = OFF
#pragma config JTAGEN = OFF

#include "libesoup/timers/clock.h"

/*
 * Interrupts
 */
void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
{
//	LOG_E("Address error");
	while (1) {
	}
}

void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
{
//	LOG_E("Stack error");
	while (1)  {
	}
}

void cpu_init(void)
{
        clock_init();
        
        INTCON2bits.GIE = ENABLED;
}

#endif // defined(__dsPIC33EP256MU806__)

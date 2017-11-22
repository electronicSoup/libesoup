/**
 * @file libesoup/processors/pic24FJ256GB106.c
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
#if defined(__PIC24FJ256GB106__)

#include <xc.h>

#pragma config FWDTEN   = OFF
#pragma config JTAGEN   = OFF
#pragma config WDTPS    = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA    = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS   = OFF              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config ICS      = PGx2             // Comm Channel Select (Emulator functions are shared with PGEC2/PGED2)
#pragma config FNOSC    = PRI              // Oscillator Select (Primary oscillator (XT, HS, EC) with PLL module (XTPLL,HSPLL, ECPLL))
#pragma config POSCMOD  = HS               // Primary Oscillator Select (HS oscillator mode selected)
#pragma config DISUVREG = OFF              // Internal USB 3.3V Regulator Disable bit (Regulator is disabled)

void cpu_init(void)
{
}

#endif // defined(__PIC24FJ256GB106__)

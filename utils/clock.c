/**
 *
 * \file libesoup/utils/clock.c
 *
 * Functions for changing the clock speed of the  processor.
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "system.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_CLOCK_FREQ
#error system.h should define SYS_CLOCK_FREQ (see libesoup/examples/system.h)
#endif

/*
 * The switch __dsPIC33EP256MU806__ is automatically set by the Microchip 
 * build system, if that is the target device of the build.
 */
void clock_init(void)
{
#if defined(__dsPIC33EP256MU806__)
        uint8_t clock;

        /*
         * There's a special case if the required clock frequency is 1/2 the
         * Crystal Frequency then we can simple use Primary Clock.
         */
        if(SYS_CLOCK_FREQ == (CRYSTAL_FREQ/2)) {
                // Initiate Clock Switch to Primary Oscillator
                clock = dsPIC33_PRIMARY_OSCILLATOR;
                __builtin_write_OSCCONH(clock);
        } else {
                // Initiate Clock Switch to Primary Oscillator with PLL (NOSC=0b011)
                clock = dsPIC33_PRIMARY_OSCILLATOR_PLL;
                __builtin_write_OSCCONH(clock);

                /*
                 * N1 = CLKDIVbits.PLLPRE + 2
                 * N2 = 2 * (CLKDIVbits.PLLPOST + 1)
                 * M  = PLLFBDbits.PLLDIV + 2
                 *
                 * CLOCK = (CRYSTAL * M) / (N1 * N2)
                 */
                CLKDIVbits.PLLPRE = 0x00;

                CLKDIVbits.PLLPOST = 0x00;
                PLLFBDbits.PLLDIV = 28;
        }

        __builtin_write_OSCCONL(OSCCON | 0x01);

        // Wait for Clock switch to occur
        while (OSCCONbits.COSC!= clock);

        if(SYS_CLOCK_FREQ != (CRYSTAL_FREQ/2)) {
                // Wait for PLL to lock
                while (OSCCONbits.LOCK!= 1);
        }
#endif // defined(__dsPIC33EP256MU806__)
}

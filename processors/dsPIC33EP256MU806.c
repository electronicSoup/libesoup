/**
 * @file libesoup/processors/dsPIC33EP256MU806.c
 *
 * @author John Whitmore
 *
 * Copyright 2017 electronicSoup Limited
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

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "dsPIC33";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_CLOCK_FREQ
#error libesoup_config.h should define SYS_CLOCK_FREQ (see libesoup/examples/libesoup_config.h)
#endif

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

static void clock_init(void);

/*
 * Interrupts
 */
void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
{
#ifdef SYS_SERIAL_LOGGING
	LOG_E("Address error");
#endif
	while (1) {
	}
}

void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
{
#ifdef SYS_SERIAL_LOGGING
	LOG_E("Stack error");
#endif
	while (1)  {
	}
}

void cpu_init(void)
{
        clock_init();
        
        INTCON2bits.GIE = ENABLED;
}

/*
 * The switch __dsPIC33EP256MU806__ is automatically set by the Microchip 
 * build system, if that is the target device of the build.
 */
static void clock_init(void)
{
	uint32_t fosc;             // See datasheet
	uint32_t fsys;             // See datasheet
        uint8_t  clock;
	uint8_t  n1;
	uint8_t  n2;
	uint8_t  loop;
	uint16_t m;
	boolean  found = FALSE;

        /*
         * There's a special case if the required clock frequency is 1/2 the
         * Crystal Frequency then we can simple use Primary Clock.
	 * NO PLL
         */
	sys_clock_freq = SYS_CLOCK_FREQ;

        if(sys_clock_freq != (CRYSTAL_FREQ/2)) {
		fosc = sys_clock_freq * 2;
	
		if((fosc < 15000000) || (fosc > 120000000)) {
			sys_clock_freq = CRYSTAL_FREQ/2;
			fosc = sys_clock_freq * 2;
		}

		/*
	         * Assuming that we're only interested in Whole MHz frequencies choose
	         * N1 so that Fplli is 1MHz 
	         */
		n1 = CRYSTAL_FREQ / 1000000;

		/*
	         * Now want a value for N2 which satisfies Fsys / N2 = requested Freq
	         */
		found = FALSE;
		for(loop = 0; loop < 3; loop++) {
			n2 = 2 << loop;
			fsys = fosc * n2;
		
			if((fsys >= 120000000) && (fsys <= 340000000)) {
				found = TRUE;
				break;
			}
		}

		if(!found) {
			sys_clock_freq = CRYSTAL_FREQ/2;
		} else {
			/*
	                 * Finally we want a value for m which is fsys/fplli we've set N1
	                 * to force fplli to be 1MHz so m is simple fsys/1MHz
			 */
			m = fsys / 1000000;
		}
	}

        /*
         * There's a special case if the required clock frequency is 1/2 the
         * Crystal Frequency then we can simple use Primary Clock.
	 * NO PLL
         */
        if(sys_clock_freq == (CRYSTAL_FREQ/2)) {
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
                CLKDIVbits.PLLPRE  = n1 - 2;

		if(n2 == 2) {
			CLKDIVbits.PLLPOST = 0b00;
		} else if (n2 == 4) {
			CLKDIVbits.PLLPOST = 0b01;			
		} else if (n2 == 8) {
			CLKDIVbits.PLLPOST = 0b11;			
		}
		
                PLLFBDbits.PLLDIV  = m - 2;
        }

        __builtin_write_OSCCONL(OSCCON | 0x01);

        // Wait for Clock switch to occur
        while (OSCCONbits.COSC!= clock);

        if(sys_clock_freq != (CRYSTAL_FREQ/2)) {
                // Wait for PLL to lock
                while (OSCCONbits.LOCK!= 1);
        }
}

result_t pin_to_port_bit(enum pin_t pin, uint8_t **port, uint8_t *bit)
{
        switch(pin) {
        case (RF3):
		*port = (uint8_t *)&PORTF;
		*bit  = 3;
                break;
                
        case (RD0):
		*port = (uint8_t *)&PORTD;
		*bit  = 0;
                break;
                
        default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                LOG_E("Not coded!\n\r");
#endif
                return(ERR_NOT_CODED);
              break;
        }
        return(SUCCESS);	
}

#endif // defined(__dsPIC33EP256MU806__)

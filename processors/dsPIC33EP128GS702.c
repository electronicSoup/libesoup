/**
 * @file libesoup/processors/dsPIC33EP128GS702.c
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
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
#if defined(__dsPIC33EP128GS702__)
#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "dsPIC33";
#endif // SYS_SERIAL_LOGGING
#include "libesoup/logger/serial_log.h"

#include "libesoup/errno.h"

/*
 * Check required libesoup_config.h defines are found
 */
//#ifndef SYS_CLOCK_FREQ
//#error libesoup_config.h should define SYS_CLOCK_FREQ (see libesoup/examples/libesoup_config.h)
//#endif

/*
 * Set up the configuration words of the processor:
 *
 * file:///opt/microchip/xc16/v1.26/docs/config_docs/33EP256MU806.html
 */
#pragma config BWRP    = OFF
#pragma config BSS     = DISABLED
#pragma config BSEN    = OFF
#pragma config GWRP    = OFF
#pragma config GSS     = DISABLED
#pragma config CWRP    = OFF
#pragma config CSS     = DISABLED
#pragma config AIVTDIS = OFF
#pragma config FNOSC = FRC
//#pragma config FNOSC   = PRI   // Primary oscillator
//#pragma config FNOSC = SOSC  // Secondary oscillator
#pragma config IESO    = OFF
#pragma config POSCMD  = HS
#pragma config IOL1WAY = OFF
#pragma config FCKSM   = CSECMD
#pragma config WDTEN   = OFF    // Disable the Watch Dog Timer
#pragma config WDTPRE  = PR128  // 1:128
#pragma config WDTPOST = PS128  // 1:128
#pragma config WINDIS  = ON     // Watchdog Timer Window disabled
#pragma config JTAGEN  = OFF

//#pragma config OSCIOFNC = ON            // OSC2 Pin Function bit (OSC2 is general purpose digital I/O pin)

//#pragma config PLLKEN = ON              // PLL Lock Wait Enable bit (Clock switch to PLL source will wait until the PLL lock signal is valid.)
//#pragma config FPWRT = PWR128           // Power-on Reset Timer Value Select bits (128ms)
//#pragma config ALTI2C1 = OFF            // Alternate I2C pins for I2C1 (SDA1/SCK1 pins are selected as the I/O pins for I2C1)
//
//static void clock_init(void);

/*
 * Interrupts
 */
void _ISR __attribute__((__no_auto_psv__)) _AddressError(void)
{
	LOG_E("Address error");
	while (1) {
	}
}

void _ISR __attribute__((__no_auto_psv__)) _StackError(void)
{
	LOG_E("Stack error");
	while (1)  {
	}
}

void cpu_init(void)
{
//        clock_init();

        INTCON2bits.GIE = ENABLED;
}

/*
 * The switch __dsPIC33EP256MU806__ is automatically set by the Microchip
 * build system, if that is the target device of the build.
 */
#if 0
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

        if(sys_clock_freq != (BRD_CRYSTAL_FREQ/2)) {
		fosc = sys_clock_freq * 2;

		if((fosc < 15000000) || (fosc > 120000000)) {
			sys_clock_freq = BRD_CRYSTAL_FREQ/2;
			fosc = sys_clock_freq * 2;
		}

		/*
	         * Assuming that we're only interested in Whole MHz frequencies choose
	         * N1 so that Fplli is 1MHz
	         */
		n1 = BRD_CRYSTAL_FREQ / 1000000;

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
			sys_clock_freq = BRD_CRYSTAL_FREQ/2;
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
        if(sys_clock_freq == (BRD_CRYSTAL_FREQ/2)) {
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

        if(sys_clock_freq != (BRD_CRYSTAL_FREQ/2)) {
                // Wait for PLL to lock
                while (OSCCONbits.LOCK!= 1);
        }
#ifdef SYS_USB_HOST
        /*
         * Set up the Aux clock for USB
         */
        // Isaac_Sewell  http://www.microchip.com/forums/m1051260.aspx
        // ACLKCON3 = 0x24C1;
        // ACLKDIV3 = 0x7;
        ACLKCON3bits.ENAPLL   = 0;     // APLL is disabled, the USB clock source is the input clock to the APLL
        ACLKCON3bits.SELACLK  = 1;     // Primary PLL provides the source clock for auxiliary clock divider
        ACLKCON3bits.AOSCMD   = 0;     // Auxiliary Oscillator disabled
        ACLKCON3bits.ASRCSEL  = 1;     // Primary Oscillator is the clock source for APLL
        ACLKCON3bits.FRCSEL   = 0;     // Auxiliary Oscillator or Primary Oscillator is the clock source for APLL (determined by ASRCSEL bit)
        ACLKCON3bits.APLLPOST = 0b110; // Divided by 2
        ACLKCON3bits.APLLPRE  = 0b011; // Divided by 4  // Different from Isacc 8MHz -> 16 MHz

        ACLKDIV3bits.APLLDIV  = 0b111; // Divide by 24

        ACLKCON3bits.ENAPLL = 1;
        while (ACLKCON3bits.APLLCK != 1);
#endif // SYS_USB_HOST
}
#endif

enum adc_pin get_adc_from_gpio(enum gpio_pin gpio_pin)
{
	enum adc_pin adc_pin;

	switch(gpio_pin) {
	case RA0:
		adc_pin = AN0;
		break;
	case RA1:
		adc_pin = AN1;
		break;
	case RA2:
		adc_pin = AN2;
		break;
	case RB0:
		adc_pin = AN3;
		break;
	case RB9:
		adc_pin = AN4;
		break;
	case RB1:
		adc_pin = AN6;
		break;
	case RB2:
		adc_pin = AN7;
		break;
	case RB3:
		adc_pin = AN18;
		break;
	case RB5:
		adc_pin = AN19;
		break;
	case RB6:
		adc_pin = AN20;
		break;
	case RB7:
		adc_pin = AN21;
		break;
	default:
		adc_pin = INVALID_ADC_PIN;
		break;
	}

	return(adc_pin);
}

enum gpio_pin get_gpio_from_adc(enum adc_pin adc_pin)
{
	enum gpio_pin gpio_pin;

	switch(adc_pin) {
	case AN0:
		gpio_pin = RA0;
		break;
	case AN1:
		gpio_pin = RA1;
		break;
	case AN2:
		gpio_pin = RA2;
		break;
	case AN3:
		gpio_pin = RB0;
		break;
	case AN4:
		gpio_pin = RB9;
		break;
	case AN6:
		gpio_pin = RB1;
		break;
	case AN7:
		gpio_pin = RB2;
		break;
	case AN18:
		gpio_pin = RB3;
		break;
	case AN19:
		gpio_pin = RB5;
		break;
	case AN20:
		gpio_pin = RB6;
		break;
	case AN21:
		gpio_pin = RB7;
		break;
	default:
		gpio_pin = INVALID_ADC_PIN;
		break;
	}

	return(gpio_pin);
}

enum pwm_pin get_pwm_from_gpio(enum gpio_pin gpio_pin)
{
	enum pwm_pin pwm_pin;

	switch (gpio_pin) {
	case RA3:
		pwm_pin = PWM1L;
		break;
	case RA4:
		pwm_pin = PWM1H;
		break;
	case RB14:
		pwm_pin = PWM2L;
		break;
	case RB13:
		pwm_pin = PWM2H;
		break;
	case RB12:
		pwm_pin = PWM3L;
		break;
	case RB11:
		pwm_pin = PWM3H;
		break;
	default:
		pwm_pin = INVALID_PWM_PIN;
		break;
	}

	return(pwm_pin);
}

enum gpio_pin get_gpio_from_pwm(enum pwm_pin pwm_pin)
{
	enum gpio_pin gpio_pin;

	switch(pwm_pin) {
	case PWM1L:
		gpio_pin = RA3;
		break;
	case PWM1H:
		gpio_pin = RA4;
		break;
	case PWM2L:
		gpio_pin = RB14;
		break;
	case PWM2H:
		gpio_pin = RB13;
		break;
	case PWM3L:
		gpio_pin = RB12;
		break;
	case PWM3H:
		gpio_pin = RB11;
		break;
	default:
		gpio_pin = INVALID_PWM_PIN;
		break;
	}

	return(gpio_pin);
}

#endif // defined(__dsPIC33E128GS702__)

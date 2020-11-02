/**
 * @file libesoup/processors/dsPIC/dsPIC33EP256GP502.c
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
#if defined(__dsPIC33EP256GP502__)
#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "dsPIC33";
#endif // SYS_SERIAL_LOGGING
#include "libesoup/logger/serial_log.h"

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"

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
#pragma config ICS     = NONE
#pragma config JTAGEN  = OFF
#pragma config FNOSC   = FRC
#pragma config ALTI2C1 = OFF
#pragma config ALTI2C2 = OFF
//#pragma config FNOSC   = PRI   // Primary oscillator
//#pragma config FNOSC = SOSC  // Secondary oscillator
#pragma config IESO    = OFF
#pragma config POSCMD  = HS
#pragma config IOL1WAY = OFF
#pragma config FCKSM   = CSECMD
#pragma config WDTPRE  = PR128  // 1:128
#pragma config WDTPOST = PS128  // 1:128
#pragma config WINDIS  = ON     // Watchdog Timer Window disabled
#pragma config FWDTEN  = OFF

static void clock_init(void);

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
        clock_init();

        INTCON2bits.GIE = ENABLED;
}

static void clock_init(void)
{
#if 1
        uint8_t  clock;
	uint8_t  n1;
	uint8_t  n2;
	uint16_t m;
#endif
        /*
         * There's a special case if the required clock frequency is 1/2 the
         * Crystal Frequency then we can simple use Primary Clock.
	 * NO PLL
         */

	/*
	 * N1 = 4 divide by 4 so 7.37 MHz > 1,842,500 Hz
	 * 1,842,500 Hz * m (43) = 79,227,500 Hz
	 * 79,227,500 Hz / N2 (2) = 39,613,750 Hz
	 */
	n1 = 4;
        m  = 63;
	n2 = 2;

	sys_clock_freq = ((BRD_CRYSTAL_FREQ / n1) * 63) / n2;  // 58038750;

	clock = dsPIC33_INTERNAL_RC_PLL;
	__builtin_write_OSCCONH(clock);

	CLKDIVbits.PLLPRE  = n1 - 2;
	CLKDIVbits.PLLPOST = 0b00;   // n2
	PLLFBDbits.PLLDIV  = m - 2;

        __builtin_write_OSCCONL(OSCCON | 0x01);

        // Wait for Clock switch to occur
        while (OSCCONbits.COSC!= clock);

	while (OSCCONbits.LOCK!= 1);
}

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
	case RB0:
		adc_pin = AN2;
		break;
	case RB1:
		adc_pin = AN3;
		break;
	case RB2:
		adc_pin = AN4;
		break;
	case RB3:
		adc_pin = AN5;
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
		gpio_pin = RB0;
		break;
	case AN3:
		gpio_pin = RB1;
		break;
	case AN4:
		gpio_pin = RB2;
		break;
	case AN5:
		gpio_pin = RB3;
		break;
	default:
		gpio_pin = INVALID_ADC_PIN;
		break;
	}

	return(gpio_pin);
}

/*
 * dsPIC33EP256GP502 had no PWM channels
 */
enum pwm_pin get_pwm_from_gpio(enum gpio_pin gpio_pin)
{
	enum pwm_pin pwm_pin;

	switch (gpio_pin) {
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
	default:
		gpio_pin = INVALID_PWM_PIN;
		break;
	}

	return(gpio_pin);
}

int16_t set_peripheral_input(enum gpio_pin pin)
{
	int16_t  ppin;

	switch(pin) {
	case RA4:
		ppin = 20;
		break;
	case RB0:
		ppin = 32;
		break;
	case RB1:
		ppin = 33;
		break;
	case RB2:
		ppin = 34;
		break;
	case RB3:
		ppin = 35;
		break;
	case RB4:
		ppin = 36;
		break;
	case RB5:
		ppin = 37;
		break;
	case RB6:
		ppin = 38;
		break;
	case RB7:
		ppin = 39;
		break;
	case RB8:
		ppin = 40;
		break;
	case RB9:
		ppin = 41;
		break;
	case RB10:
		ppin = 42;
		break;
	case RB11:
		ppin = 43;
		break;
	case RB12:
		ppin = 44;
		break;
	case RB13:
		ppin = 45;
		break;
	case RB14:
		ppin = 46;
		break;
	case RB15:
		ppin = 47;
		break;
	default:
		ppin = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(ppin);
}

int16_t set_peripheral_output(enum gpio_pin pin, uint16_t function)
{
	result_t rc = 0;

	switch(pin) {
	case RA4:  // PR20
		RPOR0bits.RP20R = function;
		break;
	case RB3:  // 35
		RPOR0bits.RP35R = function;
		break;
	case RB4:  // 36
		RPOR1bits.RP36R = function;
		break;
	case RB5:  // 37
		RPOR1bits.RP37R = function;
		break;
	case RB6:  // 38
		RPOR2bits.RP38R = function;
		break;
	case RB7:  // 39
		RPOR2bits.RP39R = function;
		break;
	case RB8:  // 40
		RPOR3bits.RP40R = function;
		break;
	case RB9:  // 41
		RPOR3bits.RP41R = function;
		break;
	case RB10: // 42
		RPOR4bits.RP42R = function;
		break;
	case RB11: // 43
		RPOR4bits.RP43R = function;
		break;
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}

	return(rc);
}
#if 0
result_t gpio_set(enum gpio_pin pin, uint16_t mode, uint8_t value)
{
	uint8_t  direction;
	uint8_t  analog;
	uint8_t  opendrain;

	analog = 0b0;
	if((mode & GPIO_MODE_ANALOG_INPUT) || (mode & GPIO_MODE_ANALOG_OUTPUT)) analog = 0b1;

	direction = 0b0;
	if((mode & GPIO_MODE_DIGITAL_INPUT) || (mode & GPIO_MODE_ANALOG_INPUT)) direction = 0b1;

	opendrain = 0b0;
	if(mode & GPIO_MODE_OPENDRAIN) opendrain = 0b1;

	switch(pin) {
	case RA0:
		ANSELAbits.ANSA0 = analog;
		TRISAbits.TRISA0 = direction;
		LATAbits.LATA0   = value;
		break;
	case RA1:
		ANSELAbits.ANSA1 = analog;
		TRISAbits.TRISA1 = direction;
		LATAbits.LATA1   = value;
		break;
	case RA2:
		TRISAbits.TRISA2 = direction;
		LATAbits.LATA2   = value;
		break;
	case RA3:
		TRISAbits.TRISA3 = direction;
		LATAbits.LATA3   = value;
		break;
	case RA4:
		TRISAbits.TRISA4 = direction;
		LATAbits.LATA4   = value;
		break;
	case RB0:
		ANSELBbits.ANSB0 = analog;
		TRISBbits.TRISB0 = direction;
		LATBbits.LATB0   = value;
		break;
	case RB1:
		ANSELBbits.ANSB1 = analog;
		TRISBbits.TRISB1 = direction;
		LATBbits.LATB1   = value;
		break;
	case RB2:
		ANSELBbits.ANSB2 = analog;
		TRISBbits.TRISB2 = direction;
		LATBbits.LATB2   = value;
		break;
	case RB3:
		ANSELBbits.ANSB3 = analog;
		TRISBbits.TRISB3 = direction;
		LATBbits.LATB3   = value;
		break;
	case RB4:
		TRISBbits.TRISB4 = direction;
		LATBbits.LATB4   = value;
		break;
	case RB5:
		TRISBbits.TRISB5 = direction;
		LATBbits.LATB5   = value;
		break;
	case RB6:
		ODCBbits.ODCB6   = opendrain;
		TRISBbits.TRISB6 = direction;
		LATBbits.LATB6   = value;
		break;
	case RB7:
		ODCBbits.ODCB7   = opendrain;
		TRISBbits.TRISB7 = direction;
		LATBbits.LATB7   = value;
		break;
	case RB8:
		TRISBbits.TRISB8 = direction;
		LATBbits.LATB8   = value;
		break;
	case RB9:
		TRISBbits.TRISB9 = direction;
		LATBbits.LATB9   = value;
		break;
	case RB10:
		TRISBbits.TRISB10 = direction;
		LATBbits.LATB10   = value;
		break;
	case RB11:
		TRISBbits.TRISB11 = direction;
		LATBbits.LATB11   = value;
		break;
	case RB12:
		TRISBbits.TRISB12 = direction;
		LATBbits.LATB12   = value;
		break;
	case RB13:
		TRISBbits.TRISB13 = direction;
		LATBbits.LATB13   = value;
		break;
	case RB14:
		TRISBbits.TRISB14 = direction;
		LATBbits.LATB14   = value;
		break;
	case RB15:
		TRISBbits.TRISB15 = direction;
		LATBbits.LATB15   = value;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	return(0);
}
#endif

#if 0
result_t gpio_get(enum gpio_pin pin)
{
	result_t rc = SUCCESS;

	switch(pin) {
	case RA0:
		return(PORTAbits.RA0);
		break;
	case RA1:
		return(PORTAbits.RA1);
		break;
	case RA2:
		return(PORTAbits.RA2);
		break;
	case RA3:
		return(PORTAbits.RA3);
		break;
	case RA4:
		return(PORTAbits.RA4);
		break;
	case RB0:
		return(PORTBbits.RB0);
		break;
	case RB1:
		return(PORTBbits.RB1);
		break;
	case RB2:
		return(PORTBbits.RB2);
		break;
	case RB3:
		return(PORTBbits.RB3);
		break;
	case RB4:
		return(PORTBbits.RB4);
		break;
	case RB5:
		return(PORTBbits.RB5);
		break;
	case RB6:
		return(PORTBbits.RB6);
		break;
	case RB7:
		return(PORTBbits.RB7);
		break;
	case RB8:
		return(PORTBbits.RB8);
		break;
	case RB9:
		return(PORTBbits.RB9);
		break;
	case RB10:
		return(PORTBbits.RB10);
		break;
	case RB11:
		return(PORTBbits.RB11);
		break;
	case RB12:
		return(PORTBbits.RB12);
		break;
	case RB13:
		return(PORTBbits.RB13);
		break;
	case RB14:
		return(PORTBbits.RB14);
		break;
	case RB15:
		return(PORTBbits.RB15);
		break;
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(rc);
}
#endif

#endif // defined(__dsPIC33E256GP502__)

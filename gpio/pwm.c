/**
 * @file libesoup/gpio/pwm.c
 *
 * @author John Whitmore
 * 
 * @brief API implementation for PWM functions
 * 
 * Copyright 2018 electronicSoup Limited
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

#include "libesoup_config.h"
#ifdef SYS_PWM

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"

result_t pwm_init(void)
{
	/*
	 * Set the Input clock prescaler. For the moment set the slowest possible
	 */
	PTCONbits.PTEN     = DISABLED;    // Disable the PWM Module for write
	PTCON2bits.PCLKDIV = 0b000;       // Divide by 64
	PTCONbits.PTEN     = ENABLED;     // re-enable the PWM Module

	return(0);
}

result_t pwm_config(enum gpio_pin pin, uint16_t frequency, uint8_t duty_percent)
{
	uint32_t i;
	uint32_t n;
	uint8_t  clkdiv;
	result_t rc;
	
	enum pwm_pin pwm_pin;

	pwm_pin = get_pwm_from_gpio(pin);
	
	if (pwm_pin == INVALID_PWM_PIN) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	/*
	 * Set the GPIO Pin as a digital output
	 */
	rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK

	PTCONbits.PTEN = DISABLED;    // Disable the PWM Module for write

	/*
	 * The period of the PWM pulses is defined by PTPER SFR (in Master Time
	 * Base Mode) or PHASEx and SPHASEx (in Independent Time Based Mode)
	 * 
	 * PTPER, PHASEx, SPHASEx = ( Fosc / ( PWM_Freq * PWM_Prescaler) )
	 * 
	 * PTPER = SYS_CLOCK_FREQ / frequency * 1 << PTCON2bits.PCLKDIV
	 */
#if 0
	i = (SYS_CLOCK_FREQ / frequency);
	clkdiv = PTCON2bits.PCLKDIV;
	do {
		n = (i / (1 << clkdiv));
		
		if (n > 0xffff) {
			clkdiv++;
		}
	} while (n > 0xffff && clkdiv < 0b111);

	if (clkdiv == 0b111 || n > 0xffff)
		return(-ERR_RANGE_ERROR);
#endif
	PTCON2bits.PCLKDIV = 0b000; //clkdiv;	
	
	switch (pwm_pin) {
	case PWM1H:
		PWMCON1bits.ITB = 1;
		PWMCON1bits.MDCS = 0;
		PHASE1 = (uint16_t)(n & 0xffff);
		DTR1 = 0x00;
		PDC1 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM1L:
		PWMCON1bits.ITB = 1;
		PWMCON1bits.MDCS = 0;
		SPHASE1 = 0xff; //(uint16_t)(n & 0xffff);
		ALTDTR1 = 0x00;
		SDC1 = 0x7f; //(uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM2H:
		PWMCON2bits.ITB = 1;
		PWMCON2bits.MDCS = 0;
		PHASE2 = (uint16_t)(n & 0xffff);
		DTR2 = 0x00;
		PDC2 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM2L:
		PWMCON2bits.ITB = 1;
		PWMCON2bits.MDCS = 0;
		SPHASE2 = (uint16_t)(n & 0xffff);
		ALTDTR2 = 0x00;
		SDC2 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM3H:
		PWMCON3bits.ITB = 1;
		PWMCON3bits.MDCS = 0;
		PHASE3 = (uint16_t)(n & 0xffff);
		DTR3 = 0x00;
		PDC3 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM3L:
		PWMCON3bits.ITB = 1;
		PWMCON3bits.MDCS = 0;
		SPHASE3 = (uint16_t)(n & 0xffff);
		ALTDTR3 = 0x00;
		SDC3 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM4H:
		PWMCON4bits.ITB = 1;
		PWMCON4bits.MDCS = 0;
		PHASE4 = (uint16_t)(n & 0xffff);
		DTR4 = 0x00;
		PDC4 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	case PWM4L:
		PWMCON4bits.ITB = 1;
		PWMCON4bits.MDCS = 0;
		SPHASE4 = (uint16_t)(n & 0xffff);
		ALTDTR4 = 0x00;
		SDC4 = (uint16_t)(((n / 100) * duty_percent) & 0xffff);
		break;

	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	
	PTCONbits.PTEN     = ENABLED;    // Disable the PWM Module for write

	return(0);
}

result_t pwm_on(enum gpio_pin pin)
{
	enum pwm_pin pwm_pin;

	pwm_pin = get_pwm_from_gpio(pin);
	
	if (pwm_pin == INVALID_PWM_PIN) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	switch (pwm_pin) {
	case PWM1L:
		IOCON1bits.PENL  = 1;
		break;
	case PWM1H:
		IOCON1bits.PENH  = 1;
		break;
	case PWM2L:
		IOCON2bits.PENL  = 1;
		break;
	case PWM2H:
		IOCON2bits.PENH  = 1;
		break;
	case PWM3L:
		IOCON3bits.PENL  = 1;
		break;
	case PWM3H:
		IOCON3bits.PENH  = 1;
		break;
	case PWM4L:
		IOCON4bits.PENL  = 1;
		break;
	case PWM4H:
		IOCON4bits.PENH  = 1;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(0);
}

result_t pwm_off(enum gpio_pin pin)
{
	enum pwm_pin pwm_pin;

	pwm_pin = get_pwm_from_gpio(pin);
	
	if (pwm_pin == INVALID_PWM_PIN) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	switch (pwm_pin) {
	case PWM1L:
		IOCON1bits.PENL  = 0;
		break;
	case PWM1H:
		IOCON1bits.PENH  = 0;
		break;
	case PWM2L:
		IOCON2bits.PENL  = 0;
		break;
	case PWM2H:
		IOCON2bits.PENH  = 0;
		break;
	case PWM3L:
		IOCON3bits.PENL  = 0;
		break;
	case PWM3H:
		IOCON3bits.PENH  = 0;
		break;
	case PWM4L:
		IOCON4bits.PENL  = 0;
		break;
	case PWM4H:
		IOCON4bits.PENH  = 0;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(0);
}

#endif // SYS_PWM

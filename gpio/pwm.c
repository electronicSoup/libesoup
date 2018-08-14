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

#include "libesoup/errno.h"

result_t pwm_config(enum gpio_pin pin, uint16_t frequency, uint8_t duty)
{
	enum pwm_pin pwm_pin;

	pwm_pin = get_pwm_from_gpio(pin);
	
	if (pwm_pin == INVALID_PWM_PIN) {
		return(ERR_BAD_INPUT_PARAMETER);
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
		return(ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(0);
}

result_t pwm_on(enum gpio_pin pin)
{
	return(0);
}

result_t pwm_off(enum gpio_pin pin)
{
	return(0);
}

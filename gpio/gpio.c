/**
 *
 * \file libesoup/gpio/gpio.c
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

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
__attribute__((unused)) static const char *TAG = "SPI";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"

uint16_t gpio_set(enum pin_t pin, uint16_t mode, uint8_t value)
{
	uint8_t  direction;
	uint8_t  analog;
	
	direction = 0b0;
	analog = 0b0;
	
	if((mode & GPIO_MODE_ANALOG_INPUT) || (mode & GPIO_MODE_ANALOG_OUTPUT)) analog = 0b1;
	if(mode & GPIO_MODE_DIGITAL_INPUT) direction = 0b1;
	
	switch(pin) {
	case RD0:
		TRISDbits.TRISD0 = direction;
		LATDbits.LATD0   = value;
		break;
	case RD1:
		TRISDbits.TRISD1 = direction;
		LATDbits.LATD1   = value;
		break;
	case RD2:
		TRISDbits.TRISD2 = direction;
		LATDbits.LATD2   = value;
		break;
	case RD3:
		TRISDbits.TRISD3 = direction;
		LATDbits.LATD3   = value;
		break;
	case RD4:
		TRISDbits.TRISD4 = direction;
		LATDbits.LATD4   = value;
		break;
	case RD5:
		TRISDbits.TRISD5 = direction;
		LATDbits.LATD5   = value;
		break;
	case RD6:
		TRISDbits.TRISD6 = direction;
		LATDbits.LATD6   = value;
		break;
	case RD7:
		TRISDbits.TRISD7 = direction;
		LATDbits.LATD7   = value;
		break;
	case RG8:
		ANSELGbits.ANSG8 = analog;
		TRISGbits.TRISG8 = direction;
		LATGbits.LATG8   = value;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	
	return(0);
}

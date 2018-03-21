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
		ANSELDbits.ANSD6 = analog;
		TRISDbits.TRISD6 = direction;
		LATDbits.LATD6   = value;
		break;
	case RD7:
		ANSELDbits.ANSD7 = analog;
		TRISDbits.TRISD7 = direction;
		LATDbits.LATD7   = value;
		break;
	case RD8:
		TRISDbits.TRISD8 = direction;
		LATDbits.LATD8   = value;
		break;
	case RD9:
		TRISDbits.TRISD9 = direction;
		LATDbits.LATD9   = value;
		break;
	case RD10:
		TRISDbits.TRISD10 = direction;
		LATDbits.LATD10   = value;
		break;
	case RD11:
		TRISDbits.TRISD11 = direction;
		LATDbits.LATD11   = value;
		break;
	case RE0:
		ANSELEbits.ANSE0 = analog;
		TRISEbits.TRISE0 = direction;
		LATEbits.LATE0   = value;
		break;
	case RE1:
		ANSELEbits.ANSE1 = analog;
		TRISEbits.TRISE1 = direction;
		LATEbits.LATE1   = value;
		break;
	case RE2:
		ANSELEbits.ANSE2 = analog;
		TRISEbits.TRISE2 = direction;
		LATEbits.LATE2   = value;
		break;
	case RE3:
		ANSELEbits.ANSE3 = analog;
		TRISEbits.TRISE3 = direction;
		LATEbits.LATE3   = value;
		break;
	case RE4:
		ANSELEbits.ANSE4 = analog;
		TRISEbits.TRISE4 = direction;
		LATEbits.LATE4   = value;
		break;
	case RE5:
		ANSELEbits.ANSE5 = analog;
		TRISEbits.TRISE5 = direction;
		LATEbits.LATE5   = value;
		break;
	case RE6:
		ANSELEbits.ANSE6 = analog;
		TRISEbits.TRISE6 = direction;
		LATEbits.LATE6   = value;
		break;
	case RE7:
		ANSELEbits.ANSE7 = analog;
		TRISEbits.TRISE7 = direction;
		LATEbits.LATE7   = value;
		break;
	case RF0:
		TRISFbits.TRISF0 = direction;
		LATFbits.LATF0   = value;
		break;
	case RF1:
		TRISFbits.TRISF1 = direction;
		LATFbits.LATF1   = value;
		break;
	case RF3:
		TRISFbits.TRISF3 = direction;
		LATFbits.LATF3   = value;
		break;
	case RF4:
		TRISFbits.TRISF4 = direction;
		LATFbits.LATF4   = value;
		break;
	case RF5:
		TRISFbits.TRISF5 = direction;
		LATFbits.LATF5   = value;
		break;
	case RG6:
		ANSELGbits.ANSG6 = analog;
		TRISGbits.TRISG6 = direction;
		LATGbits.LATG6   = value;
		break;
	case RG7:
		ANSELGbits.ANSG7 = analog;
		TRISGbits.TRISG7 = direction;
		LATGbits.LATG7   = value;
		break;
	case RG8:
		ANSELGbits.ANSG8 = analog;
		TRISGbits.TRISG8 = direction;
		LATGbits.LATG8   = value;
		break;
	case RG9:
		ANSELGbits.ANSG9 = analog;
		TRISGbits.TRISG9 = direction;
		LATGbits.LATG9   = value;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	
	return(0);
}

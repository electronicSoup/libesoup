/**
 * @file libesoup/gpio/drivers/gpio_dsPIC33EP256GP502.c
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
#include "libesoup/gpio/gpio.h"

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

result_t gpio_toggle_output(enum gpio_pin pin)
{
	switch(pin) {
	case RA0:
		LATAbits.LATA0 = ~LATAbits.LATA0;
		break;
	case RA1:
		LATAbits.LATA1 = ~LATAbits.LATA1;
		break;
	case RA2:
		LATAbits.LATA2 = ~LATAbits.LATA2;
		break;
	case RA3:
		LATAbits.LATA3 = ~LATAbits.LATA3;
		break;
	case RA4:
		LATAbits.LATA4 = ~LATAbits.LATA4;
		break;
	case RB0:
		LATBbits.LATB0 = ~LATBbits.LATB0;
		break;
	case RB1:
		LATBbits.LATB1 = ~LATBbits.LATB1;
		break;
	case RB2:
		LATBbits.LATB2 = ~LATBbits.LATB2;
		break;
	case RB3:
		LATBbits.LATB3 = ~LATBbits.LATB3;
		break;
	case RB4:
		LATBbits.LATB4 = ~LATBbits.LATB4;
		break;
	case RB5:
		LATBbits.LATB5 = ~LATBbits.LATB5;
		break;
	case RB6:
		LATBbits.LATB6 = ~LATBbits.LATB6;
		break;
	case RB7:
		LATBbits.LATB7 = ~LATBbits.LATB7;
		break;
	case RB8:
		LATBbits.LATB8 = ~LATBbits.LATB8;
		break;
	case RB9:
		LATBbits.LATB9 = ~LATBbits.LATB9;
		break;
	case RB10:
		LATBbits.LATB10 = ~LATBbits.LATB10;
		break;
	case RB11:
		LATBbits.LATB11 = ~LATBbits.LATB11;
		break;
	case RB12:
		LATBbits.LATB12 = ~LATBbits.LATB12;
		break;
	case RB13:
		LATBbits.LATB13 = ~LATBbits.LATB13;
		break;
	case RB14:
		LATBbits.LATB14 = ~LATBbits.LATB14;
		break;
	case RB15:
		LATBbits.LATB15 = ~LATBbits.LATB15;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	return(0);
}

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

#endif // #if defined(__dsPIC33EP256GP502__)

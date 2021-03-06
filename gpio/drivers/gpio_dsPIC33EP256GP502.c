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
		LATAbits.LATA0   = value;
		ANSELAbits.ANSA0 = analog;
		ODCAbits.ODCA0   = opendrain;
		TRISAbits.TRISA0 = direction;
		break;
	case RA1:
		LATAbits.LATA1   = value;
		ANSELAbits.ANSA1 = analog;
		ODCAbits.ODCA1   = opendrain;
		TRISAbits.TRISA1 = direction;
		break;
	case RA2:
		LATAbits.LATA2   = value;
		TRISAbits.TRISA2 = direction;
		ODCAbits.ODCA2   = opendrain;
		break;
	case RA3:
		LATAbits.LATA3   = value;
		TRISAbits.TRISA3 = direction;
		ODCAbits.ODCA3   = opendrain;
		break;
	case RA4:
		LATAbits.LATA4   = value;
		TRISAbits.TRISA4 = direction;
		ODCAbits.ODCA4   = opendrain;
		break;
	case RB0:
		LATBbits.LATB0   = value;
		ANSELBbits.ANSB0 = analog;
		ODCBbits.ODCB0   = opendrain;
		TRISBbits.TRISB0 = direction;
		break;
	case RB1:
		LATBbits.LATB1   = value;
		ANSELBbits.ANSB1 = analog;
		ODCBbits.ODCB1   = opendrain;
		TRISBbits.TRISB1 = direction;
		break;
	case RB2:
		LATBbits.LATB2   = value;
		ANSELBbits.ANSB2 = analog;
		ODCBbits.ODCB2   = opendrain;
		TRISBbits.TRISB2 = direction;
		break;
	case RB3:
		LATBbits.LATB3   = value;
		ANSELBbits.ANSB3 = analog;
		ODCBbits.ODCB3   = opendrain;
		TRISBbits.TRISB3 = direction;
		break;
	case RB4:
		LATBbits.LATB4   = value;
		ODCBbits.ODCB4   = opendrain;
		TRISBbits.TRISB4 = direction;
		break;
	case RB5:
		LATBbits.LATB5   = value;
		ODCBbits.ODCB5   = opendrain;
		TRISBbits.TRISB5 = direction;
		break;
	case RB6:
		LATBbits.LATB6   = value;
		ODCBbits.ODCB6   = opendrain;
		TRISBbits.TRISB6 = direction;
		break;
	case RB7:
		LATBbits.LATB7   = value;
		ODCBbits.ODCB7   = opendrain;
		TRISBbits.TRISB7 = direction;
		break;
	case RB8:
		LATBbits.LATB8   = value;
		ANSELBbits.ANSB8 = analog;
		ODCBbits.ODCB8   = opendrain;
		TRISBbits.TRISB8 = direction;
		break;
	case RB9:
		LATBbits.LATB9   = value;
		ODCBbits.ODCB9   = opendrain;
		TRISBbits.TRISB9 = direction;
		break;
	case RB10:
		LATBbits.LATB10   = value;
		ODCBbits.ODCB10   = opendrain;
		TRISBbits.TRISB10 = direction;
		break;
	case RB11:
		LATBbits.LATB11   = value;
		ODCBbits.ODCB11   = opendrain;
		TRISBbits.TRISB11 = direction;
		break;
	case RB12:
		LATBbits.LATB12   = value;
		ODCBbits.ODCB12   = opendrain;
		TRISBbits.TRISB12 = direction;
		break;
	case RB13:
		LATBbits.LATB13   = value;
		ODCBbits.ODCB13   = opendrain;
		TRISBbits.TRISB13 = direction;
		break;
	case RB14:
		LATBbits.LATB14   = value;
		ODCBbits.ODCB14   = opendrain;
		TRISBbits.TRISB14 = direction;
		break;
	case RB15:
		LATBbits.LATB15   = value;
		ODCBbits.ODCB15   = opendrain;
		TRISBbits.TRISB15 = direction;
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

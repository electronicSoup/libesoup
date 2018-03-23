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

#if defined(__dsPIC33EP256MU806__)
result_t gpio_set(enum pin_t pin, uint16_t mode, uint8_t value)
{
	uint8_t  direction;
	uint8_t  analog;
	uint8_t  opendrain;

	analog = 0b0;
	if((mode & GPIO_MODE_ANALOG_INPUT) || (mode & GPIO_MODE_ANALOG_OUTPUT)) analog = 0b1;
	
	direction = 0b0;
	if(mode & GPIO_MODE_DIGITAL_INPUT) direction = 0b1;
	
	opendrain = 0b0;
	if((mode & GPIO_MODE_OPENDRAIN_INPUT) || (mode & GPIO_MODE_OPENDRAIN_OUTPUT)) opendrain = 0b1;
	
	switch(pin) {
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
		ANSELBbits.ANSB4 = analog;
		TRISBbits.TRISB4 = direction;
		LATBbits.LATB4   = value;
		break;
	case RB5:
		ANSELBbits.ANSB5 = analog;
		TRISBbits.TRISB5 = direction;
		LATBbits.LATB5   = value;
		break;
	case RB6:
		ANSELBbits.ANSB6 = analog;
		TRISBbits.TRISB6 = direction;
		LATBbits.LATB6   = value;
		break;
	case RB7:
		ANSELBbits.ANSB7 = analog;
		TRISBbits.TRISB7 = direction;
		LATBbits.LATB7   = value;
		break;
	case RB8:
		ANSELBbits.ANSB8 = analog;
		TRISBbits.TRISB8 = direction;
		LATBbits.LATB8   = value;
		break;
	case RB9:
		ANSELBbits.ANSB9 = analog;
		TRISBbits.TRISB9 = direction;
		LATBbits.LATB9   = value;
		break;
	case RB10:
		ANSELBbits.ANSB10 = analog;
		TRISBbits.TRISB10 = direction;
		LATBbits.LATB10   = value;
		break;
	case RB11:
		ANSELBbits.ANSB11 = analog;
		TRISBbits.TRISB11 = direction;
		LATBbits.LATB11   = value;
		break;
	case RB12:
		ANSELBbits.ANSB12 = analog;
		TRISBbits.TRISB12 = direction;
		LATBbits.LATB12   = value;
		break;
	case RB13:
		ANSELBbits.ANSB13 = analog;
		TRISBbits.TRISB13 = direction;
		LATBbits.LATB13   = value;
		break;
	case RB14:
		ANSELBbits.ANSB14 = analog;
		TRISBbits.TRISB14 = direction;
		LATBbits.LATB14   = value;
		break;
	case RB15:
		ANSELBbits.ANSB15 = analog;
		TRISBbits.TRISB15 = direction;
		LATBbits.LATB15   = value;
		break;
	case RC12:
		TRISCbits.TRISC12 = direction;
		LATCbits.LATC12   = value;
		break;
	case RC13:
		ANSELCbits.ANSC13 = analog;
		TRISCbits.TRISC13 = direction;
		LATCbits.LATC13   = value;
		break;
	case RC14:
		ANSELCbits.ANSC14 = analog;
		TRISCbits.TRISC14 = direction;
		LATCbits.LATC14   = value;
		break;
	case RC15:
		TRISCbits.TRISC15 = direction;
		LATCbits.LATC15   = value;
		break;
	case RD0:
		ODCDbits.ODCD0   = opendrain;
		TRISDbits.TRISD0 = direction;
		LATDbits.LATD0   = value;
		break;
	case RD1:
		ODCDbits.ODCD1   = opendrain;
		TRISDbits.TRISD1 = direction;
		LATDbits.LATD1   = value;
		break;
	case RD2:
		ODCDbits.ODCD2   = opendrain;
		TRISDbits.TRISD2 = direction;
		LATDbits.LATD2   = value;
		break;
	case RD3:
		ODCDbits.ODCD3   = opendrain;
		TRISDbits.TRISD3 = direction;
		LATDbits.LATD3   = value;
		break;
	case RD4:
		ODCDbits.ODCD4   = opendrain;
		TRISDbits.TRISD4 = direction;
		LATDbits.LATD4   = value;
		break;
	case RD5:
		ODCDbits.ODCD5   = opendrain;
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
		ODCDbits.ODCD8   = opendrain;
		TRISDbits.TRISD8 = direction;
		LATDbits.LATD8   = value;
		break;
	case RD9:
		ODCDbits.ODCD9   = opendrain;
		TRISDbits.TRISD9 = direction;
		LATDbits.LATD9   = value;
		break;
	case RD10:
		ODCDbits.ODCD10  = opendrain;
		TRISDbits.TRISD10 = direction;
		LATDbits.LATD10   = value;
		break;
	case RD11:
		ODCDbits.ODCD11  = opendrain;
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
		ODCFbits.ODCF0   = opendrain;
		TRISFbits.TRISF0 = direction;
		LATFbits.LATF0   = value;
		break;
	case RF1:
		ODCFbits.ODCF1   = opendrain;
		TRISFbits.TRISF1 = direction;
		LATFbits.LATF1   = value;
		break;
	case RF3:
		ODCFbits.ODCF3   = opendrain;
		TRISFbits.TRISF3 = direction;
		LATFbits.LATF3   = value;
		break;
	case RF4:
		ODCFbits.ODCF4   = opendrain;
		TRISFbits.TRISF4 = direction;
		LATFbits.LATF4   = value;
		break;
	case RF5:
		ODCFbits.ODCF5   = opendrain;
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
#endif  // #if defined(__dsPIC33EP256MU806__)


#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
result_t gpio_set(enum pin_t pin, uint16_t mode, uint8_t value)
{
	uint8_t  direction;
	uint8_t  analog;
	uint8_t  opendrain;
	
	direction = 0b0;
	opendrain = 0b0;
	analog = 0b1;
	if((mode & GPIO_MODE_ANALOG_INPUT) || (mode & GPIO_MODE_ANALOG_OUTPUT)) analog = 0b0;
	
	if(mode & GPIO_MODE_DIGITAL_INPUT) direction = 0b1;
	if((mode & GPIO_MODE_OPENDRAIN_INPUT) || (mode & GPIO_MODE_OPENDRAIN_OUTPUT)) opendrain = 0b1;
	
	switch(pin) {
	case RB0:
		AD1PCFGLbits.PCFG0 = analog;
		ODCBbits.ODB0    = opendrain;
		TRISBbits.TRISB0 = direction;
		LATBbits.LATB0   = value;
		break;
	case RB1:
		AD1PCFGLbits.PCFG1 = analog;
		ODCBbits.ODB1    = opendrain;
		TRISBbits.TRISB1 = direction;
		LATBbits.LATB1   = value;
		break;
	case RB2:
		AD1PCFGLbits.PCFG2 = analog;
		ODCBbits.ODB2    = opendrain;
		TRISBbits.TRISB2 = direction;
		LATBbits.LATB2   = value;
		break;
	case RB3:
		AD1PCFGLbits.PCFG3 = analog;
		ODCBbits.ODB3    = opendrain;
		TRISBbits.TRISB3 = direction;
		LATBbits.LATB3   = value;
		break;
	case RB4:
		AD1PCFGLbits.PCFG4 = analog;
		ODCBbits.ODB4    = opendrain;
		TRISBbits.TRISB4 = direction;
		LATBbits.LATB4   = value;
		break;
	case RB5:
		AD1PCFGLbits.PCFG5 = analog;
		ODCBbits.ODB5    = opendrain;
		TRISBbits.TRISB5 = direction;
		LATBbits.LATB5   = value;
		break;
	case RB6:
		AD1PCFGLbits.PCFG6 = analog;
		ODCBbits.ODB6    = opendrain;
		TRISBbits.TRISB6 = direction;
		LATBbits.LATB6   = value;
		break;
	case RB7:
		AD1PCFGLbits.PCFG7 = analog;
		ODCBbits.ODB7    = opendrain;
		TRISBbits.TRISB7 = direction;
		LATBbits.LATB7   = value;
		break;
	case RB8:
		AD1PCFGLbits.PCFG8 = analog;
		ODCBbits.ODB8    = opendrain;
		TRISBbits.TRISB8 = direction;
		LATBbits.LATB8   = value;
		break;
	case RB9:
		AD1PCFGLbits.PCFG9 = analog;
		ODCBbits.ODB9    = opendrain;
		TRISBbits.TRISB9 = direction;
		LATBbits.LATB9   = value;
		break;
	case RB10:
		AD1PCFGLbits.PCFG10 = analog;
		ODCBbits.ODB10    = opendrain;
		TRISBbits.TRISB10 = direction;
		LATBbits.LATB10   = value;
		break;
	case RB11:
		AD1PCFGLbits.PCFG11 = analog;
		ODCBbits.ODB11    = opendrain;
		TRISBbits.TRISB11 = direction;
		LATBbits.LATB11   = value;
		break;
	case RB12:
		AD1PCFGLbits.PCFG12 = analog;
		ODCBbits.ODB12    = opendrain;
		TRISBbits.TRISB12 = direction;
		LATBbits.LATB12   = value;
		break;
	case RB13:
		AD1PCFGLbits.PCFG13 = analog;
		ODCBbits.ODB13    = opendrain;
		TRISBbits.TRISB13 = direction;
		LATBbits.LATB13   = value;
		break;
	case RB14:
		AD1PCFGLbits.PCFG14 = analog;
		ODCBbits.ODB14    = opendrain;
		TRISBbits.TRISB14 = direction;
		LATBbits.LATB14   = value;
		break;
	case RB15:
		AD1PCFGLbits.PCFG15 = analog;
		ODCBbits.ODB15    = opendrain;
		TRISBbits.TRISB15 = direction;
		LATBbits.LATB15   = value;
		break;
	case RC12:
		ODCCbits.ODC12    = opendrain;
		TRISCbits.TRISC12 = direction;
		LATCbits.LATC12   = value;
		break;
	case RC13:
		ODCCbits.ODC13    = opendrain;
		TRISCbits.TRISC13 = direction;
		LATCbits.LATC13   = value;
		break;
	case RC14:
		ODCCbits.ODC14    = opendrain;
		TRISCbits.TRISC14 = direction;
		LATCbits.LATC14   = value;
		break;
	case RC15:
		ODCCbits.ODC15    = opendrain;
		TRISCbits.TRISC15 = direction;
		LATCbits.LATC15   = value;
		break;
	case RD0:
		ODCDbits.ODD0    = opendrain;
		TRISDbits.TRISD0 = direction;
		LATDbits.LATD0   = value;
		break;
	case RD1:
		ODCDbits.ODD1    = opendrain;
		TRISDbits.TRISD1 = direction;
		LATDbits.LATD1   = value;
		break;
	case RD2:
		ODCDbits.ODD2    = opendrain;
		TRISDbits.TRISD2 = direction;
		LATDbits.LATD2   = value;
		break;
	case RD3:
		ODCDbits.ODD3    = opendrain;
		TRISDbits.TRISD3 = direction;
		LATDbits.LATD3   = value;
		break;
	case RD4:
		ODCDbits.ODD4    = opendrain;
		TRISDbits.TRISD4 = direction;
		LATDbits.LATD4   = value;
		break;
	case RD5:
		ODCDbits.ODD5    = opendrain;
		TRISDbits.TRISD5 = direction;
		LATDbits.LATD5   = value;
		break;
	case RD6:
		ODCDbits.ODD6    = opendrain;
		TRISDbits.TRISD6 = direction;
		LATDbits.LATD6   = value;
		break;
	case RD7:
		ODCDbits.ODD7    = opendrain;
		TRISDbits.TRISD7 = direction;
		LATDbits.LATD7   = value;
		break;
	case RD8:
		ODCDbits.ODD8    = opendrain;
		TRISDbits.TRISD8 = direction;
		LATDbits.LATD8   = value;
		break;
	case RD9:
		ODCDbits.ODD9    = opendrain;
		TRISDbits.TRISD9 = direction;
		LATDbits.LATD9   = value;
		break;
	case RD10:
		ODCDbits.ODD11   = opendrain;
		TRISDbits.TRISD10 = direction;
		LATDbits.LATD10   = value;
		break;
	case RD11:
		ODCDbits.ODD11   = opendrain;
		TRISDbits.TRISD11 = direction;
		LATDbits.LATD11   = value;
		break;
	case RE0:
		ODCEbits.ODE0    = opendrain;
		TRISEbits.TRISE0 = direction;
		LATEbits.LATE0   = value;
		break;
	case RE1:
		ODCEbits.ODE1    = opendrain;
		TRISEbits.TRISE1 = direction;
		LATEbits.LATE1   = value;
		break;
	case RE2:
		ODCEbits.ODE2    = opendrain;
		TRISEbits.TRISE2 = direction;
		LATEbits.LATE2   = value;
		break;
	case RE3:
		ODCEbits.ODE3    = opendrain;
		TRISEbits.TRISE3 = direction;
		LATEbits.LATE3   = value;
		break;
	case RE4:
		ODCEbits.ODE4    = opendrain;
		TRISEbits.TRISE4 = direction;
		LATEbits.LATE4   = value;
		break;
	case RE5:
		ODCEbits.ODE5    = opendrain;
		TRISEbits.TRISE5 = direction;
		LATEbits.LATE5   = value;
		break;
	case RE6:
		ODCEbits.ODE6    = opendrain;
		TRISEbits.TRISE6 = direction;
		LATEbits.LATE6   = value;
		break;
	case RE7:
		ODCEbits.ODE7    = opendrain;
		TRISEbits.TRISE7 = direction;
		LATEbits.LATE7   = value;
		break;
	case RF0:
		ODCFbits.ODF0    = opendrain;
		TRISFbits.TRISF0 = direction;
		LATFbits.LATF0   = value;
		break;
	case RF1:
		ODCFbits.ODF1    = opendrain;
		TRISFbits.TRISF1 = direction;
		LATFbits.LATF1   = value;
		break;
	case RF3:
		ODCFbits.ODF3    = opendrain;
		TRISFbits.TRISF3 = direction;
		LATFbits.LATF3   = value;
		break;
	case RF4:
		ODCFbits.ODF3    = opendrain;
		TRISFbits.TRISF4 = direction;
		LATFbits.LATF4   = value;
		break;
	case RF5:
		ODCFbits.ODF3    = opendrain;
		TRISFbits.TRISF5 = direction;
		LATFbits.LATF5   = value;
		break;
	case RG2:
		ODCGbits.ODG2    = opendrain;
		TRISGbits.TRISG2 = direction;
		LATGbits.LATG2   = value;
		break;
	case RG3:
		ODCGbits.ODG3    = opendrain;
		TRISGbits.TRISG3 = direction;
		LATGbits.LATG3   = value;
		break;
	case RG6:
		ODCGbits.ODG6    = opendrain;
		TRISGbits.TRISG6 = direction;
		LATGbits.LATG6   = value;
		break;
	case RG7:
		ODCGbits.ODG7    = opendrain;
		TRISGbits.TRISG7 = direction;
		LATGbits.LATG7   = value;
		break;
	case RG8:
		ODCGbits.ODG8    = opendrain;
		TRISGbits.TRISG8 = direction;
		LATGbits.LATG8   = value;
		break;
	case RG9:
		ODCGbits.ODG9    = opendrain;
		TRISGbits.TRISG9 = direction;
		LATGbits.LATG9   = value;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	
	return(0);
}
#endif // #if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)


#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
result_t gpio_get(enum pin_t pin)
{
	switch(pin) {
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
	case RC12:
		return(PORTCbits.RC12);
		break;
	case RC13:
		return(PORTCbits.RC13);
		break;
	case RC14:
		return(PORTCbits.RC14);
		break;
	case RC15:
		return(PORTCbits.RC15);
		break;
	case RD0:
		return(PORTDbits.RD0);
		break;
	case RD1:
		return(PORTDbits.RD1);
		break;
	case RD2:
		return(PORTDbits.RD2);
		break;
	case RD3:
		return(PORTDbits.RD3);
		break;
	case RD4:
		return(PORTDbits.RD4);
		break;
	case RD5:
		return(PORTDbits.RD5);
		break;
	case RD6:
		return(PORTDbits.RD6);
		break;
	case RD7:
		return(PORTDbits.RD7);
		break;
	case RD8:
		return(PORTDbits.RD8);
		break;
	case RD9:
		return(PORTDbits.RD9);
		break;
	case RD10:
		return(PORTDbits.RD10);
		break;
	case RD11:
		return(PORTDbits.RD11);
		break;
	case RE0:
		return(PORTEbits.RE0);
		break;
	case RE1:
		return(PORTEbits.RE1);
		break;
	case RE2:
		return(PORTEbits.RE2);
		break;
	case RE3:
		return(PORTEbits.RE3);
		break;
	case RE4:
		return(PORTEbits.RE4);
		break;
	case RE5:
		return(PORTEbits.RE5);
		break;
	case RE6:
		return(PORTEbits.RE6);
		break;
	case RE7:
		return(PORTEbits.RE7);
		break;
	case RF0:
		return(PORTFbits.RF0);
		break;
	case RF1:
		return(PORTFbits.RF1);
		break;
	case RF3:
		return(PORTFbits.RF3);
		break;
	case RF4:
		return(PORTFbits.RF4);
		break;
	case RF5:
		return(PORTFbits.RF5);
		break;
	case RG6:
		return(PORTGbits.RG6);
		break;
	case RG7:
		return(PORTGbits.RG7);
		break;
	case RG8:
		return(PORTGbits.RG8);
		break;
	case RG9:
		return(PORTGbits.RG9);
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	
	return(0);
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)


#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
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
        return(0);	
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)

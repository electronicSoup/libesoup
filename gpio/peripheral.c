/**
 *
 * \file libesoup/gpio/peripheral.c
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
#include "libesoup/gpio/gpio.h"

#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
int16_t set_peripheral_input(enum pin_t pin)
{
	int16_t  ppin;
	
	switch(pin) {	
	case RB0:
#if defined(__dsPIC33EP256MU806__)
		ppin = 32;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 0;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB1:
#if defined(__dsPIC33EP256MU806__)
		ppin = 33;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 1;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB2:
#if defined(__dsPIC33EP256MU806__)
		ppin = 34;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 13;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB3:
#if defined(__dsPIC33EP256MU806__)
		ppin = 35;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB4:
#if defined(__dsPIC33EP256MU806__)
		ppin = 36;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 28;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB5:
#if defined(__dsPIC33EP256MU806__)
		ppin = 37;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 18;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB6:
#if defined(__dsPIC33EP256MU806__)
		ppin = 38;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 6;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB7:
#if defined(__dsPIC33EP256MU806__)
		ppin = 39;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 7;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB8:
#if defined(__dsPIC33EP256MU806__)
		ppin = 40;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 8;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB9:
#if defined(__dsPIC33EP256MU806__)
		ppin = 41;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 9;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB10:
#if defined(__dsPIC33EP256MU806__)
		ppin = 42;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 9;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB11:
#if defined(__dsPIC33EP256MU806__)
		ppin = 43;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB12:
#if defined(__dsPIC33EP256MU806__)
		ppin = 44;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB13:
#if defined(__dsPIC33EP256MU806__)
		ppin = 45;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB14:
#if defined(__dsPIC33EP256MU806__)
		ppin = 46;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 14;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB15:
#if defined(__dsPIC33EP256MU806__)
		ppin = 47;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 29;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RC12:
#if defined(__dsPIC33EP256MU806__)
		ppin = 60;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RC13:
#if defined(__dsPIC33EP256MU806__)
		ppin = 61;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RC14:
#if defined(__dsPIC33EP256MU806__)
		ppin = 62;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 37;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RC15:
#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#endif
		break;
	case RD0:
#if defined(__dsPIC33EP256MU806__)
		ppin = 64;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 11;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD1:
#if defined(__dsPIC33EP256MU806__)
		ppin = 65;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 24;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD2:
#if defined(__dsPIC33EP256MU806__)
		ppin = 66;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 23;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD3:
#if defined(__dsPIC33EP256MU806__)
		ppin = 67;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 22;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD4:
#if defined(__dsPIC33EP256MU806__)
		ppin = 68;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 25;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD5:
#if defined(__dsPIC33EP256MU806__)
		ppin = 69;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 20;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD6:
#if defined(__dsPIC33EP256MU806__)
		ppin = 70;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD7:
#if defined(__dsPIC33EP256MU806__)
		ppin = 71;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD8:
#if defined(__dsPIC33EP256MU806__)
		ppin = 72;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 2;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD9:
#if defined(__dsPIC33EP256MU806__)
		ppin = 73;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 4;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD10:
#if defined(__dsPIC33EP256MU806__)
		ppin = 74;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 3;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD11:
#if defined(__dsPIC33EP256MU806__)
		ppin = 75;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 12;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE0:
#if defined(__dsPIC33EP256MU806__)
		ppin = 80;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE1:
#if defined(__dsPIC33EP256MU806__)
		ppin = 81;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE2:
#if defined(__dsPIC33EP256MU806__)
		ppin = 82;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE3:
#if defined(__dsPIC33EP256MU806__)
		ppin = 83;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE4:
#if defined(__dsPIC33EP256MU806__)
		ppin = 84;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE5:
#if defined(__dsPIC33EP256MU806__)
		ppin = 85;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE6:
#if defined(__dsPIC33EP256MU806__)
		ppin = 86;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE7:
#if defined(__dsPIC33EP256MU806__)
		ppin = 87;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF0:
#if defined(__dsPIC33EP256MU806__)
		ppin = 96;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF1:
#if defined(__dsPIC33EP256MU806__)
		ppin = 97;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF3:
#if defined(__dsPIC33EP256MU806__)
		ppin = 99;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 16;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF4:
#if defined(__dsPIC33EP256MU806__)
		ppin = 100;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 10;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF5:
#if defined(__dsPIC33EP256MU806__)
		ppin = 101;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 17;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RG6:
#if defined(__dsPIC33EP256MU806__)
		ppin = 118;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 21;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RG7:
#if defined(__dsPIC33EP256MU806__)
		ppin = 119;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 26;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RG8:
#if defined(__dsPIC33EP256MU806__)
		ppin = 120;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 19;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RG9:
#if defined(__dsPIC33EP256MU806__)
		ppin = 121;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		ppin = 27;
#else
#error Uncoded Peripheral functions
#endif
		break;
		
	default:
		ppin = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(ppin);
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)

#if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
int16_t set_peripheral_output(enum pin_t pin, uint16_t function)
{
	result_t rc = 0;
	
	switch(pin) {
	case RB0:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR0bits.RP0R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB1:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR0bits.RP1R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB2:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR6bits.RP13R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB4:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR14bits.RP28R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB5:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR9bits.RP18R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB6:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR3bits.RP6R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB7:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR3bits.RP7R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB8:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR4bits.RP8R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB9:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR4bits.RP9R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB14:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR7bits.RP14R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RB15:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR14bits.RP29R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD0:
#if defined(__dsPIC33EP256MU806__)
		RPOR0bits.RP64R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR5bits.RP11R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD1:
#if defined(__dsPIC33EP256MU806__)
		RPOR0bits.RP65R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR12bits.RP24R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD2:
#if defined(__dsPIC33EP256MU806__)
		RPOR1bits.RP66R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR11bits.RP23R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD3:
#if defined(__dsPIC33EP256MU806__)
		RPOR1bits.RP67R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR11bits.RP22R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD4:
#if defined(__dsPIC33EP256MU806__)
		RPOR2bits.RP68R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR12bits.RP25R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD5:
#if defined(__dsPIC33EP256MU806__)
		RPOR2bits.RP69R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR10bits.RP20R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD6:
#if defined(__dsPIC33EP256MU806__)
		RPOR3bits.RP70R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD7:
#if defined(__dsPIC33EP256MU806__)
		RPOR3bits.RP71R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;		
	case RD8:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR1bits.RP2R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD9:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR2bits.RP4R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD10:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR1bits.RP3R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RD11:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR6bits.RP12R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE0:
#if defined(__dsPIC33EP256MU806__)
		RPOR4bits.RP80R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE2:
#if defined(__dsPIC33EP256MU806__)
		RPOR5bits.RP82R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE4:
#if defined(__dsPIC33EP256MU806__)
		RPOR5bits.RP84R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE5:
#if defined(__dsPIC33EP256MU806__)
		RPOR6bits.RP85R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RE7:
#if defined(__dsPIC33EP256MU806__)
		RPOR6bits.RP87R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF0:
#if defined(__dsPIC33EP256MU806__)
		RPOR7bits.RP96R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF1:
#if defined(__dsPIC33EP256MU806__)
		RPOR7bits.RP97R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF3:
#if defined(__dsPIC33EP256MU806__)
		RPOR8bits.RP99R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR8bits.RP16R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF4:
#if defined(__dsPIC33EP256MU806__)
		RPOR9bits.RP100R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR5bits.RP10R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RF5:
#if defined(__dsPIC33EP256MU806__)
		RPOR9bits.RP101R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR8bits.RP17R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;
	case RG6:
#if defined(__dsPIC33EP256MU806__)
		RPOR13bits.RP118R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR10bits.RP21R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;		
	case RG7:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR13bits.RP26R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;		
	case RG8:
#if defined(__dsPIC33EP256MU806__)
		RPOR14bits.RP120R = function;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR9bits.RP19R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;		
	case RG9:
#if defined(__dsPIC33EP256MU806__)
		rc = -ERR_BAD_INPUT_PARAMETER;
#elif defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
		RPOR13bits.RP27R = function;
#else
#error Uncoded Peripheral functions
#endif
		break;		
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	
	return(rc);
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)

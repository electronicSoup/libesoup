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

int16_t set_peripheral_input(enum pin_t pin)
{
	int16_t  ppin;
	
	switch(pin) {
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
	case RC12:
		ppin = 60;
		break;
	case RC13:
		ppin = 61;
		break;
	case RC14:
		ppin = 62;
		break;
	case RD0:
		ppin = 64;
		break;
	case RD1:
		ppin = 65;
		break;
	case RD2:
		ppin = 66;
		break;
	case RD3:
		ppin = 67;
		break;
	case RD4:
		ppin = 68;
		break;
	case RD5:
		ppin = 69;
		break;
	case RD6:
		ppin = 70;
		break;
	case RD7:
		ppin = 71;
		break;
	case RD8:
		ppin = 72;
		break;
	case RD9:
		ppin = 73;
		break;
	case RD10:
		ppin = 74;
		break;
	case RD11:
		ppin = 75;
		break;
	case RE0:
		ppin = 80;
		break;
	case RE1:
		ppin = 81;
		break;
	case RE2:
		ppin = 82;
		break;
	case RE3:
		ppin = 83;
		break;
	case RE4:
		ppin = 84;
		break;
	case RE5:
		ppin = 85;
		break;
	case RE6:
		ppin = 86;
		break;
	case RE7:
		ppin = 87;
		break;
	case RF0:
		ppin = 96;
		break;
	case RF1:
		ppin = 97;
		break;
	case RF3:
		ppin = 99;
		break;
	case RF4:
		ppin = 100;
		break;
	case RF5:
		ppin = 101;
		break;
	case RG6:
		ppin = 118;
		break;
	case RG7:
		ppin = 119;
		break;
	case RG8:
		ppin = 120;
		break;
	case RG9:
		ppin = 121;
		break;
		
	default:
		ppin = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(ppin);
}

int16_t set_peripheral_output(enum pin_t pin, uint16_t function)
{
	result_t rc = 0;
	
	switch(pin) {
	case RD0:
		RPOR0bits.RP64R = function;
		break;
	case RD1:
		RPOR0bits.RP65R = function;
		break;
	case RD2:
		RPOR1bits.RP66R = function;
		break;
	case RD3:
		RPOR1bits.RP67R = function;
		break;
	case RD4:
		RPOR2bits.RP68R = function;
		break;
	case RD5:
		RPOR2bits.RP69R = function;
		break;
	case RD6:
		RPOR3bits.RP70R = function;
		break;
	case RD7:
		RPOR3bits.RP71R = function;
		break;		
	case RE0:
		RPOR4bits.RP80R = function;
		break;
	case RE2:
		RPOR5bits.RP82R = function;
		break;
	case RE4:
		RPOR5bits.RP84R = function;
		break;
	case RE5:
		RPOR6bits.RP85R = function;
		break;
	case RE7:
		RPOR6bits.RP87R = function;
		break;
	case RF0:
		RPOR7bits.RP96R = function;
		break;
	case RF1:
		RPOR7bits.RP97R = function;
		break;
	case RF3:
		RPOR8bits.RP99R = function;
		break;
	case RF4:
		RPOR9bits.RP100R = function;
		break;
	case RF5:
		RPOR9bits.RP101R = function;
		break;
	case RG6:
		RPOR13bits.RP118R = function;
		break;		
	case RG8:
		RPOR14bits.RP120R = function;
		break;		
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	
	return(rc);
}

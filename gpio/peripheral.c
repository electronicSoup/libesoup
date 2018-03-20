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
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	
	return(rc);
}

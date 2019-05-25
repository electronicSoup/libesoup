/**
 *
 * @file libesoup/processors/pic18/es-dsPIC18F45K40.h
 *
 * @author John Whitmore
 *
 * This file contains mcu specific definitions for the pic18f45k40. 
 *
 * Copyright 2019 electronicSoup Limited
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _ES_PIC18F45K40_H
#define _ES_PIC18F45K40_H

#include <p18cxxx.h>

#define TIMER_0 0
#define TIMER_1 1

#define NUMBER_HW_TIMERS  2

enum gpio_pin {
	RA0,     ///< Port A Bit 0
	RA1,     ///< Port A Bit 1
	RA2,     ///< Port A Bit 2
	RA3,     ///< Port A Bit 3
	RA4,     ///< Port A Bit 4
	RA5,     ///< Port A Bit 5
	RA6,     ///< Port A Bit 6
	RA7,     ///< Port A Bit 7

	RB0,     ///< Port B Bit 0
	RB1,     ///< Port B Bit 1
	RB2,     ///< Port B Bit 2
	RB3,     ///< Port B Bit 3
	RB4,     ///< Port B Bit 4
	RB5,     ///< Port B Bit 5
	RB6,     ///< Port B Bit 6
	RB7,     ///< Port B Bit 7

	RC0,     ///< Port C Bit 0
	RC1,     ///< Port C Bit 1
	RC2,     ///< Port C Bit 2
	RC3,     ///< Port C Bit 3
	RC4,     ///< Port C Bit 4
	RC5,     ///< Port C Bit 5
	RC6,     ///< Port C Bit 6
	RC7,     ///< Port C Bit 7

	RE3,     ///< Port E Bit 3

	INVALID_GPIO_PIN = 0xff,     ///< Dummy Value used to represent no GPIO Pin
};

extern void pic18f45k40_init(void);

#endif // _ES_PIC18F45K40_H

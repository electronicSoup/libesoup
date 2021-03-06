/**
 * @file libesoup/utils/rand.c
 *
 * @author John Whitmore
 *
 * @brief Random initialisation function for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017-2018 electronicSoup Limited
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

#include <stdlib.h>    // needed for srand()
#include "libesoup_config.h"

#ifdef SYS_RAND

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "RAND";
#include "libesoup/logger/serial_log.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

/*
 * Function to generate a random seed to initialise the microchip random library
 */
void random_init(void)
{
	uint16_t  loop;
	uint16_t  seed;

#if defined(__dsPIC33EP256MU806__)
	uint16_t  *data;

	data = (uint16_t *)&TMR1;

	seed = 0;

	for(loop = 0; loop < (&OC16TMR - &TMR1); loop++) {
		asm("CLRWDT");
		seed = seed + *data;
		data++;
	}
#elif defined (__PIC24FJ256GB106__)
	uint8_t  *data;

	data = (uint8_t *)&IC1TMR;

	seed = 0;

	for(loop = 0; loop < 0x1D8; loop++) {
		asm("CLRWDT");
		seed = seed + *data;
		data++;
	}
#endif
	
	LOG_D("Seed 0x%lx\n\r", seed);
	srand(seed);
}

#endif // SYS_RAND

/**
 *
 * \file es_lib/utils/rand.c
 *
 * Random initialisation function for the electronicSoup Cinnamon Bun
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "system.h"
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "RAND"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif

static uint8_t initialised = FALSE;

/*
 * Function to generate a random seed to initialise the microchip random library
 */
void random_init(void)
{
	uint16_t  loop;
	u32       seed;
	uint8_t  *data;

	if(initialised) {
		return;
	}

#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_INFO))
	log_i("random_init()\n\r");
#endif
	data = (uint8_t *)&IC1TMR;

	seed = 0;

	for(loop = 0; loop < 0x1D8; loop++) {
		asm("CLRWDT");
		seed = seed + *data;
		data++;
	}

#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Seed 0x%lx\n\r", seed);
#endif
	srand(seed);
}

/**
 *
 * \file libesoup/utils/rand.c
 *
 * Random initialisation function for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017 electronicSoup Limited
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("random_init()\n\r");
#endif
	data = (uint8_t *)&IC1TMR;

	seed = 0;

	for(loop = 0; loop < 0x1D8; loop++) {
		asm("CLRWDT");
		seed = seed + *data;
		data++;
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Seed 0x%lx\n\r", seed);
#endif
	srand(seed);
}

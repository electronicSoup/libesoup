/**
 *
 * \file es_lib/utils/rand.c
 *
 * Random initialisation function for the electronicSoup Cinnamon Bun
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

//#include <libpic30.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#include "system.h"
//#include "es_lib/utils/utils.h"
//#ifdef HEARTBEAT
//#include "es_lib/timers/timer_sys.h"
//#endif
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "RAND"


void random_init(void)
{
	u16  loop;
	u32  seed;
	u8  *data;

	LOG_D("random_init()\n\r");
	data = (u8 *)&IC1TMR;  //0x146

	seed = 0;

	for(loop = 0; loop < 0x1D8; loop++) {
		asm("CLRWDT");
		seed = seed + *data;
		data++;
	}

	LOG_D("Seed 0x%lx\n\r", seed);
	srand(seed);
}


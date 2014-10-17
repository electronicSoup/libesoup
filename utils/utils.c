/**
 *
 * \file es_lib/utils/utils.c
 *
 * General utility functions of the electronicSoup CAN code Library
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

#include <libpic30.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "system.h"
#include "es_lib/utils/utils.h"
#ifdef HEARTBEAT
#include "es_lib/timers/timer_sys.h"
#endif
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "UTILS"

#define NUM_INSTRUCTION_PER_ROW 64

#ifdef HEARTBEAT
void heartbeat_on(union sigval data);
void heartbeat_off(union sigval data);
#endif

unsigned char SPIWriteByte(unsigned char write);

#ifdef HEARTBEAT
void heartbeat_on(union sigval data)
{
	es_timer timer;

	Heartbeat_on();

	start_timer(HEARTBEAT_ON_TIME, heartbeat_off, data, &timer);
}
#endif

#ifdef HEARTBEAT
void heartbeat_off(union sigval data)
{
	es_timer timer;

	Heartbeat_off();

	start_timer(HEARTBEAT_OFF_TIME, heartbeat_on, data, &timer);
}
#endif


void random_init(void)
{
	u8   loop;
	u32  seed;
	u8  *data;

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

/**
 * Various string values, for example device manufacturer,
 * are stored in the Program Flash memory during project building. In
 * order to operate on these strings they must first be copies to the
 * system's RAM memory. This function carries out this function.
 *
 * This code is specific to the PIC24 Processor and C30 Compiler
 *
 * @param dest: Destination String location in RAM
 * @param source: The source String located in Program Flash Memory
 * @return Function returns the number of characters copied across.
 */
UINT16 psv_strcpy(char *dst, __prog__ char *src, UINT16 len)
{
	char *ptr = dst;
	UINT16 i = 0;

	while ((*src != 0x00) && (*src != 0xff) && (i < len - 1)) {
		*ptr++ = *src++;
		i++;
	}
	*ptr = 0x00;

	return i;
}


/**
 *
 * \file es_can/logger/serial.c
 *
 * Functions for logging to the Serial Port of the electronicSoup CAN code Library
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
#include "system.h"

#include <stdio.h>

#if (LOG_LEVEL < NO_LOGGING)
void serial_init(void)
{
	/*
	 * Serial Outout pin should be defined in include file system.h
	 */
#if defined(SERIAL_PORT_GndTxRx)
	RPOR12bits.RP25R = 3;
#elif defined(SERIAL_PORT_GndRxTx)
	RPOR10bits.RP20R = 3;
#endif

#if 0
#if SERIAL_PORT == 20  // <-
	RPOR10bits.RP20R = 3;
#elif SERIAL_PORT == 6
	RPOR3bits.RP6R = 3;
#elif SERIAL_PORT == 26
	RPOR13bits.RP26R = 3;
#endif
#endif //0

	U1MODE = 0x8800;
	U1STA = 0x0410;

	/*
	 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
	 *
	 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
	 *
	 * UxBRG = ((CLOCK/SERIAL_BAUD)/16) -1
	 *
	 */
	U1BRG = ((CLOCK_FREQ / SERIAL_BAUD) / 16) - 1;
}
#endif // (LOG_LEVEL < NO_LOGGING)


#if (LOG_LEVEL < NO_LOGGING)
#ifdef __C30__
void serial_log(log_level_t level, char* tag, char* fmt, ...)
#elif defined(__18CXX)
void serial_log(log_level_t level, const rom char* tag, const rom char* fmt, ...)
#endif
{
	va_list args;

	switch (level) {
		case Debug:
			printf(" D :");
			break;

		case Info:
			printf(" I :");
			break;

		case Warning:
			printf(" W :");
			break;

		case Error:
			printf("***\n\r");
			printf("*E*:");
			break;
	}
	printf(tag);
	printf(":");

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	if (level == Error) {
		printf("***\n\r");
	}
}
#endif //(LOG_LEVEL < NO_LOGGING)

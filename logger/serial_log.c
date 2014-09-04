/**
 *
 * \file es_can/logger/serial_log.c
 *
 * Functions for logging to the Serial Port.
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
void serial_log(log_level_t level, char* tag, char* fmt, ...)
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

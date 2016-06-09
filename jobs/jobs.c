/**
 *
 * \file es_lib/jobs/jobs.c
 *
 * Functions for using a MODBUS Comms.
 *
 * The first uart port is used by the logger. See es_lib/logger
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
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
#define DEBUG_FILE
#define TAG "JOBS"

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/jobs/jobs.h"

struct job jobs[NUMBER_OF_JOBS];

void jobs_init(void)
{
	u16 loop;
	
	for(loop = 0; loop < NUMBER_OF_JOBS; loop++) {
		jobs[loop].function = NULL;
		jobs[loop].data = NULL;
	}
}

result_t jobs_add(void (*function)(void *), void *data)
{
	result_t rc = SUCCESS;
	u16      loop = 0;

	while((jobs[loop].function) && (loop < NUMBER_OF_JOBS)) {
		loop++;
	}

	if(loop < NUMBER_OF_JOBS) {
		jobs[loop].function = function;
		jobs[loop].data = data;
	} else {
		LOG_E("No space for system job\n\r");
		rc = ERR_NO_RESOURCES;
	}
	return(rc);
}

result_t jobs_execute(void)
{
	result_t rc = SUCCESS;
	u16      loop = 0;

	while((jobs[loop].function) && (loop < NUMBER_OF_JOBS)) {
		jobs[loop].function(jobs[loop].data);
		jobs[loop].function = NULL;
		jobs[loop].data = NULL;
		loop++;
	}

	return(rc);
}

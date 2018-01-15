/**
 *
 * \file libesoup/jobs/jobs.c
 *
 * Functions for using system Jobs.
 *
 * The first uart port is used by the logger. See libesoup/logger
 *
 * Copyright 2017 2018 electronicSoup Limited
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

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "JOBS";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/jobs/jobs.h"

struct job {
    void (*function)(void *);
    void *data;
};

struct job jobs[SYS_NUMBER_OF_JOBS];
static uint8_t write_index;
static uint8_t read_index;
static uint8_t count;

void jobs_init(void)
{
	uint16_t loop;

        write_index = 0;
        read_index = 0;
        count = 0;

	for(loop = 0; loop < SYS_NUMBER_OF_JOBS; loop++) {
		jobs[loop].function = NULL;
		jobs[loop].data = NULL;
	}
}

result_t jobs_add(void (*function)(void *), void *data)
{
	result_t rc = SUCCESS;

        __builtin_disi(0x3FFF); /* disable interrupts */
        jobs[write_index].function = function;
        jobs[write_index].data = data;
        write_index = (write_index + 1) % SYS_NUMBER_OF_JOBS;
        count++;
        __builtin_disi(0x0000); /* enable interrupts */
	return(rc);
}

result_t jobs_execute(void)
{
	result_t  rc = SUCCESS;
        void    (*function)(void *);
        void     *data;

	while(count) {
                if(jobs[read_index].function) {
                        function = jobs[read_index].function;
                        data     = jobs[read_index].data;

                        jobs[read_index].function = NULL;
                        jobs[read_index].data = NULL;
                        read_index = (read_index + 1) % SYS_NUMBER_OF_JOBS;
                        count--;

                        function(data);
                } else {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
                        LOG_E("Bad job at %d\n\r", read_index);
#endif
                        rc = ERR_GENERAL_ERROR;
                }
	}

	return(rc);
}

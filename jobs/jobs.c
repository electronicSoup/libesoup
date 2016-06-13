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

struct job {
    void (*function)(void *);
    void *data;
};

struct job jobs[NUMBER_OF_JOBS];
static u8 write_index;
static u8 read_index;
static u8 count;

void jobs_init(void)
{
	u16 loop;

        write_index = 0;
        read_index = 0;
        count = 0;
	
	for(loop = 0; loop < NUMBER_OF_JOBS; loop++) {
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
        write_index = (write_index + 1) % NUMBER_OF_JOBS;
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
                        LOG_D("Execute Job(%d)\n\r", read_index);
                        function = jobs[read_index].function;
                        data     = jobs[read_index].data;

                        jobs[read_index].function = NULL;
                        jobs[read_index].data = NULL;
                        read_index = (read_index + 1) % NUMBER_OF_JOBS;
                        count--;
                        
                        function(data);
                } else {
                        LOG_E("Bad job at %d\n\r", read_index);
                        rc = ERR_GENERAL_ERROR;
                }
	}

	return(rc);
}

/**
 * @file libesoup/examples/main_job.c
 *
 * @author John Whitmore
 * 
 * Example main.c file to demonstrate jobs. 
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
#include "libesoup_config.h"

#include "libesoup/jobs/jobs.h"

int main(void)
{
	result_t rc;
	/*
	 * Initialise the libesoup library
	 */
	rc = libesoup_init();
	if(rc < 0) {
		// Error Condition?
	}

        while(1) {
        }
        return 0;
}

/**
 *
 * \file libesoup/jobs/jobs.h
 *
 * \brief Module for creating Jobs which will wake up uC from sleep and execute
 *
 * The first uart port is used by the logger. See libesoup/logger
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
#ifndef __JOBS_H
#define __JOBS_H

#ifdef SYS_JOBS

/**
 * \
 */
extern void     jobs_init(void);
extern result_t jobs_add(void (*function)(void *), void *data);
extern result_t jobs_execute(void);

#endif // SYS_JOBS
#endif // __JOBS_H

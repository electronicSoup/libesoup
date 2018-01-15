/**
 *
 * \file libesoup/utils/heartbeat.c
 *
 * General utility functions of the electronicSoup CAN code Library
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

//#include <libpic30.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdio.h>
#include "libesoup_config.h"
//#include "libesoup/utils/utils.h"
//#ifdef HEARTBEAT
//#include "libesoup/timers/timer_sys.h"
//#endif
#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "HEARTBEAT";
#include "libesoup/logger/serial_log.h"
#endif

void heartbeat_on(union sigval data);
void heartbeat_off(union sigval data);

void heartbeat_on(union sigval data)
{
	es_timer timer;

	Heartbeat_on();

	start_timer(HEARTBEAT_ON_TIME, heartbeat_off, data, &timer);
}

void heartbeat_off(union sigval data)
{
	es_timer timer;

	Heartbeat_off();

	start_timer(HEARTBEAT_OFF_TIME, heartbeat_on, data, &timer);
}

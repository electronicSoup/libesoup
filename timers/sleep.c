/**
 *
 * \file es_lib/utils/sleep.c
 *
 * Functionality for sleep
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

#define DEBUG_FILE
#define TAG "SLEEP"

#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/hw_timers.h"

static volatile u8 sleep_over;

static void hw_expiry_function(u8 data)
{
	sleep_over = TRUE;
}

void sleep(ty_time_units units, u16 time)
{
	u8 hw_timer;

	sleep_over = FALSE;
	hw_timer = hw_timer_start(units, time, FALSE, hw_expiry_function, 0);

	while(!sleep_over) {
		asm ("CLRWDT");
	}
}

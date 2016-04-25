/**
 *
 * \file es_lib/timers/hw_timers.c
 *
 * Hardware Timer functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
 *******************************************************************************
 *
 */
#define DEBUG_FILE
#define TAG "HW_TIMERS"

#include "system.h"
#include "es_lib/timers/hw_timers.h"
#include "es_lib/logger/serial_log.h"

u8 hw_timer_start(ty_time_units units, u16 time, u8 repeat, void (*expiry_function)(void))
{
	LOG_D("start_hw_timer()\n\r");
}


void hw_timer_cancel(u8 timer)
{

}

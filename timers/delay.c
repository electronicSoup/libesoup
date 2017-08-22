/**
 *
 * \file libesoup/utils/sleep.c
 *
 * Functionality for sleep
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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

#define DEBUG_FILE
#define TAG "SLEEP"

#include "libesoup/timers/hw_timers.h"

static volatile uint8_t delay_over;

static void hw_expiry_function(void *data)
{
	delay_over = TRUE;
}

void delay(ty_time_units units, uint16_t duration)
{
	uint8_t  hw_timer;

        delay_over = FALSE;
        hw_timer = hw_timer_start(units, duration, FALSE, hw_expiry_function, NULL);

        while(!delay_over) {
                __asm__ ("CLRWDT");
        }
}

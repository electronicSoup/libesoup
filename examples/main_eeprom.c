/*
 *
 * @file libesoup/utils/eeprom.c
 *
 * @author John Whitmore
 *
 * Copyright 2017-2019 electronicSoup Limited
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
#ifdef SYS_EXAMPLE_EEPROM

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "Main";

#include "libesoup/logger/serial_log.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif
#endif // SYS_SERIAL_LOGGING

#include "libesoup/hardware/eeprom.h"
#include "libesoup/timers/sw_timers.h"

static struct timer_req timer_request;

/*
 * Forward declaration of the timer expiry function
 */
void exp_func(timer_id timer, union sigval);

/*
 * Local variables, static to file
 */
static uint8_t eeprom_address = 0;
static uint8_t eeprom_test_rd = 0;

int main(void)
{
        result_t      rc;
	timer_id      timer;

	rc = libesoup_init();
	if(rc < 0) {
		rc = (~rc) + 1;
		LOG_E("libesoup Failed -%d\n\r", rc);
		while(1){}
	}

	eeprom_address = 0;
	eeprom_test_rd = 0;

	timer_request.period.units = Seconds;
	timer_request.period.duration = 1;
	timer_request.type = repeat_expiry;
	timer_request.data.sival_int = 0;
	timer_request.exp_fn = exp_func;

	rc = sw_timer_start(&timer_request);
	RC_CHECK
	timer = (uint8_t)rc;

	LOG_D("Entering main loop\n\r");

        while(1) {
		libesoup_tasks();
        }
        return 0;
}

void exp_func(timer_id timer, union sigval data)
{
	result_t rc = 0;

	if(!eeprom_test_rd) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("write to address %d\n\r", eeprom_address);
#endif
                rc = eeprom_write((uint16_t)eeprom_address, (uint8_t)eeprom_address);
		if(rc < 0) {
			LOG_E("Failed to Write\n\r");
		}
		eeprom_test_rd = 1;
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("read from address %d\n\r", eeprom_address);
#endif
                rc = eeprom_read((uint16_t)eeprom_address);
		if(rc < 0) {
			LOG_E("Failed to Read\n\r");
		} else {
			LOG_D("Read back a value of %d\n\r", (uint8_t)rc);
		}
		eeprom_address++;
		if(eeprom_address == BRD_EEPROM_MAX_ADDRESS) eeprom_address = 0;
		eeprom_test_rd = 0;
	}
}

#endif // SYS_EXAMPLE_EEPROM

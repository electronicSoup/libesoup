/**
 *
 * \file libesoup/processors/dsPIC33/change_notification/change_notification.c
 *
 * Copyright 2018 electronicSoup Limited
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

#ifdef SYS_CHANGE_NOTIFICATION
#if defined(__dsPIC33EP256MU806__)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
__attribute__((unused)) static const char *TAG = "CHANGE"; 
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#ifndef SYS_CHANGE_NOTIFICATION_MAX_PINS
#error SYS_CHANGE_NOTIFICATION_MAX_PINS Not defined required by SYS_CHANGE_NOTIFICATION
#endif

#include "libesoup/errno.h"
#include "libesoup/processors/dsPIC33/change_notification/change_notification.h"

static result_t enable_change(uint8_t *port, uint8_t bit);
static result_t disable_change(uint8_t *port, uint8_t bit);

struct change_entry {
	uint8_t         monitored:1;
	uint8_t         previous_value:1;
	uint8_t         bit:4;
	uint8_t        *port;
	change_notifier notify;
};

struct change_entry pins[SYS_CHANGE_NOTIFICATION_MAX_PINS];

void __attribute__((__interrupt__, __no_auto_psv__)) _CNInterrupt(void)
{
	boolean  current_value;
	uint16_t loop;

        IFS1bits.CNIF = 0;

	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(pins[loop].monitored) {
			
			current_value = *pins[loop].port & (0b1 << pins[loop].bit);
			if(pins[loop].previous_value == current_value) {
				pins[loop].previous_value = current_value;
				pins[loop].notify(pins[loop].port, pins[loop].bit);
			}
		}
	}
}

result_t change_notifier_init()
{
	uint16_t loop;

	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		pins[loop].monitored = FALSE;
		pins[loop].notify    = (change_notifier)NULL;
	}
	
	IEC1bits.CNIE = ENABLED;
	return(0);
}

result_t change_notifier_register(uint8_t *port, uint8_t bit, change_notifier notifier)
{
	uint16_t loop;

	/*
	 * See is the pin already being monitored by someone
	 */
	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(pins[loop].monitored && (pins[loop].port == port) && (pins[loop].bit == bit)) {
			return(-ERR_BAD_INPUT_PARAMETER);
		}
	}

	/*
	 * Find an inactive entry and fill it in
	 */
	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(!pins[loop].monitored) {
			pins[loop].monitored = TRUE;
			pins[loop].port      = port;
			pins[loop].bit       = bit;
			pins[loop].notify    = notifier;

			return(enable_change(port, bit));
		}
	}

	return(-ERR_NO_RESOURCES);
}

static result_t enable_change(uint8_t *port, uint8_t bit)
{
	if(port == (uint8_t *)&PORTD) {
		CNEND |= (0b1 << bit);		
		return(0);
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

result_t change_notifier_deregister(uint8_t *port, uint8_t bit)
{
	uint16_t loop;
	
	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(pins[loop].monitored && (pins[loop].port == port) && (pins[loop].bit == bit)) {
			pins[loop].monitored = FALSE;

			return(disable_change(port, bit));
		}
	}
	
	return(0);
}

static result_t disable_change(uint8_t *port, uint8_t bit)
{
	if(port == (uint8_t *)&PORTD) {
		CNEND &= ~(0b1 << bit);
		return(0);
	}
	return(-ERR_BAD_INPUT_PARAMETER);
}

#endif // __dsPIC33EP256MU806__
#endif // SYS_CHANGE_NOTIFICATION

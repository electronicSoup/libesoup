/**
 * @file libesoup/gpio/change_notification.c
 *
 * @author John Whitmore
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
#if defined(__dsPIC33EP256MU806__) || defined(__dsPIC33EP128GS702__)

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
#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/change_notification.h"

static result_t enable_change(enum gpio_pin);
static result_t disable_change(enum gpio_pin);

struct change_entry {
	uint8_t          monitored:1;
	uint8_t          previous_value:1;
	enum gpio_pin       pin;
	change_notifier  notify;
};

struct change_entry pins[SYS_CHANGE_NOTIFICATION_MAX_PINS];

void __attribute__((__interrupt__, __no_auto_psv__)) _CNInterrupt(void)
{
	boolean  current_value;
	uint16_t loop;

        IFS1bits.CNIF = 0;

	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(pins[loop].monitored) {

			current_value = gpio_get(pins[loop].pin);
			if(pins[loop].previous_value != current_value) {
				pins[loop].previous_value = current_value;
				pins[loop].notify(pins[loop].pin);
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

result_t change_notifier_register(enum gpio_pin pin, change_notifier notifier)
{
	uint8_t     loop;

	/*
	 * See is the pin already being monitored by someone
	 */
	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(pins[loop].monitored && (pins[loop].pin == pin)) {
			return(-ERR_BAD_INPUT_PARAMETER);
		}
	}

	/*
	 * Find an inactive entry and fill it in
	 */
	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(!pins[loop].monitored) {
			pins[loop].monitored      = TRUE;
			pins[loop].pin            = pin;
			pins[loop].notify         = notifier;
			pins[loop].previous_value = gpio_get(pin);

			return(enable_change(pin));
		}
	}

	return(-ERR_NO_RESOURCES);
}

#if defined(__dsPIC33EP256MU806__)
static result_t enable_change(enum gpio_pin pin)
{
	result_t rc = 0;

	switch(pin) {
	case RB0:
		CNENBbits.CNIEB0 = 1;
		break;
	case RB1:
		CNENBbits.CNIEB1 = 1;
		break;
	case RB2:
		CNENBbits.CNIEB2 = 1;
		break;
	case RB3:
		CNENBbits.CNIEB3 = 1;
		break;
	case RB4:
		CNENBbits.CNIEB4 = 1;
		break;
	case RB5:
		CNENBbits.CNIEB5 = 1;
		break;
	case RB6:
		CNENBbits.CNIEB6 = 1;
		break;
	case RB7:
		CNENBbits.CNIEB7 = 1;
		break;
	case RB8:
		CNENBbits.CNIEB8 = 1;
		break;
	case RB9:
		CNENBbits.CNIEB9 = 1;
		break;
	case RB10:
		CNENBbits.CNIEB10 = 1;
		break;
	case RB11:
		CNENBbits.CNIEB11 = 1;
		break;
	case RB12:
		CNENBbits.CNIEB12 = 1;
		break;
	case RB13:
		CNENBbits.CNIEB13 = 1;
		break;
	case RB14:
		CNENBbits.CNIEB14 = 1;
		break;
	case RB15:
		CNENBbits.CNIEB15 = 1;
		break;
	case RC12:
		CNENCbits.CNIEC12 = 1;
		break;
	case RC13:
		CNENCbits.CNIEC13 = 1;
		break;
	case RC14:
		CNENCbits.CNIEC14 = 1;
		break;
	case RC15:
		CNENCbits.CNIEC15 = 1;
		break;
	case RD0:
		CNENDbits.CNIED0 = 1;
		break;
	case RD1:
		CNENDbits.CNIED1 = 1;
		break;
	case RD2:
		CNENDbits.CNIED2 = 1;
		break;
	case RD3:
		CNENDbits.CNIED3 = 1;
		break;
	case RD4:
		CNENDbits.CNIED4 = 1;
		break;
	case RD5:
		CNENDbits.CNIED5 = 1;
		break;
	case RD6:
		CNENDbits.CNIED6 = 1;
		break;
	case RD7:
		CNENDbits.CNIED7 = 1;
		break;
	case RD8:
		CNENDbits.CNIED8 = 1;
		break;
	case RD9:
		CNENDbits.CNIED9 = 1;
		break;
	case RD10:
		CNENDbits.CNIED10 = 1;
		break;
	case RD11:
		CNENDbits.CNIED11 = 1;
		break;
	case RE0:
		CNENEbits.CNIEE0 = 1;
		break;
	case RE1:
		CNENEbits.CNIEE1 = 1;
		break;
	case RE2:
		CNENEbits.CNIEE2 = 1;
		break;
	case RE3:
		CNENEbits.CNIEE3 = 1;
		break;
	case RE4:
		CNENEbits.CNIEE4 = 1;
		break;
	case RE5:
		CNENEbits.CNIEE5 = 1;
		break;
	case RE6:
		CNENEbits.CNIEE6 = 1;
		break;
	case RE7:
		CNENEbits.CNIEE7 = 1;
		break;
	case RF0:
		CNENFbits.CNIEF0 = 1;
		break;
	case RF1:
		CNENFbits.CNIEF1 = 1;
		break;
	case RF3:
		CNENFbits.CNIEF3 = 1;
		break;
	case RF4:
		CNENFbits.CNIEF4 = 1;
		break;
	case RF5:
		CNENFbits.CNIEF5 = 1;
		break;
	case RG2:
		CNENGbits.CNIEG2 = 1;
		break;
	case RG3:
		CNENGbits.CNIEG3 = 1;
		break;
	case RG6:
		CNENGbits.CNIEG6 = 1;
		break;
	case RG7:
		CNENGbits.CNIEG7 = 1;
		break;
	case RG8:
		CNENGbits.CNIEG8 = 1;
		break;
	case RG9:
		CNENGbits.CNIEG9 = 1;
		break;
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(rc);
}
#elif defined(__dsPIC33EP128GS702__)
static result_t enable_change(enum gpio_pin pin)
{
	result_t rc = 0;

	switch(pin) {
	case RB0:
		CNENBbits.CNIEB0 = 1;
		break;
	case RB1:
		CNENBbits.CNIEB1 = 1;
		break;
	case RB2:
		CNENBbits.CNIEB2 = 1;
		break;
	case RB3:
		CNENBbits.CNIEB3 = 1;
		break;
	case RB4:
		CNENBbits.CNIEB4 = 1;
		break;
	case RB5:
		CNENBbits.CNIEB5 = 1;
		break;
	case RB6:
		CNENBbits.CNIEB6 = 1;
		break;
	case RB7:
		CNENBbits.CNIEB7 = 1;
		break;
	case RB8:
		CNENBbits.CNIEB8 = 1;
		break;
	case RB9:
		CNENBbits.CNIEB9 = 1;
		break;
	case RB11:
		CNENBbits.CNIEB11 = 1;
		break;
	case RB12:
		CNENBbits.CNIEB12 = 1;
		break;
	case RB13:
		CNENBbits.CNIEB13 = 1;
		break;
	case RB14:
		CNENBbits.CNIEB14 = 1;
		break;
	case RB15:
		CNENBbits.CNIEB15 = 1;
		break;
	case RC12:
		CNENCbits.CNIEC12 = 1;
		break;
	case RC13:
		CNENCbits.CNIEC13 = 1;
		break;
	case RC14:
		CNENCbits.CNIEC14 = 1;
		break;
	case RC15:
		CNENCbits.CNIEC15 = 1;
		break;
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(rc);
}
#endif

result_t change_notifier_deregister(enum gpio_pin pin)
{
	uint16_t   loop;

	for(loop = 0; loop < SYS_CHANGE_NOTIFICATION_MAX_PINS; loop++) {
		if(pins[loop].monitored && (pins[loop].pin == pin)) {
			pins[loop].monitored = FALSE;

			return(disable_change(pin));
		}
	}

	return(-ERR_BAD_INPUT_PARAMETER);
}

static result_t disable_change(enum gpio_pin pin)
{
	result_t rc = 0;
	switch(pin) {
	case RB0:
		CNENBbits.CNIEB0 = 0;
		break;
	case RB1:
		CNENBbits.CNIEB1 = 0;
		break;
	case RB2:
		CNENBbits.CNIEB2 = 0;
		break;
	case RB3:
		CNENBbits.CNIEB3 = 0;
		break;
	case RB4:
		CNENBbits.CNIEB4 = 0;
		break;
	case RB5:
		CNENBbits.CNIEB5 = 0;
		break;
	case RB6:
		CNENBbits.CNIEB6 = 0;
		break;
	case RB7:
		CNENBbits.CNIEB7 = 0;
		break;
	case RB8:
		CNENBbits.CNIEB8 = 0;
		break;
	case RB9:
		CNENBbits.CNIEB9 = 0;
		break;
	case RB10:
		CNENBbits.CNIEB10 = 0;
		break;
	case RB11:
		CNENBbits.CNIEB11 = 0;
		break;
	case RB12:
		CNENBbits.CNIEB12 = 0;
		break;
	case RB13:
		CNENBbits.CNIEB13 = 0;
		break;
	case RB14:
		CNENBbits.CNIEB14 = 0;
		break;
	case RB15:
		CNENBbits.CNIEB15 = 0;
		break;
	case RC12:
		CNENCbits.CNIEC12 = 0;
		break;
	case RC13:
		CNENCbits.CNIEC13 = 0;
		break;
	case RC14:
		CNENCbits.CNIEC14 = 0;
		break;
	case RC15:
		CNENCbits.CNIEC15 = 0;
		break;
	case RD0:
		CNENDbits.CNIED0 = 0;
		break;
	case RD1:
		CNENDbits.CNIED1 = 0;
		break;
	case RD2:
		CNENDbits.CNIED2 = 0;
		break;
	case RD3:
		CNENDbits.CNIED3 = 0;
		break;
	case RD4:
		CNENDbits.CNIED4 = 0;
		break;
	case RD5:
		CNENDbits.CNIED5 = 0;
		break;
	case RD6:
		CNENDbits.CNIED6 = 0;
		break;
	case RD7:
		CNENDbits.CNIED7 = 0;
		break;
	case RD8:
		CNENDbits.CNIED8 = 0;
		break;
	case RD9:
		CNENDbits.CNIED9 = 0;
		break;
	case RD10:
		CNENDbits.CNIED10 = 0;
		break;
	case RD11:
		CNENDbits.CNIED11 = 0;
		break;
	case RE0:
		CNENEbits.CNIEE0 = 0;
		break;
	case RE1:
		CNENEbits.CNIEE1 = 0;
		break;
	case RE2:
		CNENEbits.CNIEE2 = 0;
		break;
	case RE3:
		CNENEbits.CNIEE3 = 0;
		break;
	case RE4:
		CNENEbits.CNIEE4 = 0;
		break;
	case RE5:
		CNENEbits.CNIEE5 = 0;
		break;
	case RE6:
		CNENEbits.CNIEE6 = 0;
		break;
	case RE7:
		CNENEbits.CNIEE7 = 0;
		break;
	case RF0:
		CNENFbits.CNIEF0 = 0;
		break;
	case RF1:
		CNENFbits.CNIEF1 = 0;
		break;
	case RF3:
		CNENFbits.CNIEF3 = 0;
		break;
	case RF4:
		CNENFbits.CNIEF4 = 0;
		break;
	case RF5:
		CNENFbits.CNIEF5 = 0;
		break;
	case RG2:
		CNENGbits.CNIEG2 = 0;
		break;
	case RG3:
		CNENGbits.CNIEG3 = 0;
		break;
	case RG6:
		CNENGbits.CNIEG6 = 0;
		break;
	case RG7:
		CNENGbits.CNIEG7 = 0;
		break;
	case RG8:
		CNENGbits.CNIEG8 = 0;
		break;
	case RG9:
		CNENGbits.CNIEG9 = 0;
		break;
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(rc);
}

#endif // __dsPIC33EP256MU806__
#endif // SYS_CHANGE_NOTIFICATION

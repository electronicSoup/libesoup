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

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "OneWire";
#include "libesoup/logger/serial_log.h"
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#include "libesoup/processors/dsPIC33/change_notification/change_notification.h"

result_t change_notifier_init()
{
	
}

result_t change_notifier_register(enum pin_t pin, change_notifier notifier)
{
	
}

result_t change_notifier_deregister(enum pin_t pin)
{
	
}

#endif // SYS_CHANGE_NOTIFICATION

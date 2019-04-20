/**
 * \file   main_change_notification.c
 *
 * Copyright 2018-2019 electronicSoup Limited
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
static const char *TAG = "Main";

#include "libesoup/logger/serial_log.h"
#include "libesoup/gpio/change_notification.h"

void change_notification(enum gpio_pin pin)
{
        LOG_D("Change\n\r");
}

int main(void)
{
	result_t         rc;
        
	rc = libesoup_init();
	if(rc < 0) {
		LOG_E("libesoup_init()\n\r");
	}

	rc = change_notifier_register(RD2, change_notification);
	if(rc < 0) {
		LOG_E("change_notifier_register()\n\r");
	}

        LOG_D("Starting \n\r");

        while(1) {
                Nop();
        }
        return 0;
}

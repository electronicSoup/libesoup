/**
 * \file   main_change_notification.c
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

#define DEBUG_FILE
static const char *TAG = "Main";

#include "libesoup/logger/serial_log.h"
#include "libesoup/processors/dsPIC33/change_notification/change_notification.h"

void change_notification(uint8_t *port, uint8_t bit);

int main(void)
{
	result_t         rc;
        
	rc = libesoup_init();
	if(rc < 0) {
#if ((defined SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("libesoup_init()\n\r");
#endif		
	}

	rc = change_notifier_register((uint8_t *)&LATD , 2, change_notification);
	if(rc < 0) {
#if ((defined SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("change_notifier_register()\n\r");
#endif
	}

#if ((defined SYS_SERIAL_LOGGING) && (defined DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Starting \n\r");
#endif

        while(1) {
                Nop();
        }
        return 0;
}

void change_notification(uint8_t *port, uint8_t bit)
{
#if ((defined SYS_SERIAL_LOGGING) && (defined DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Change\n\r");
#endif	
}

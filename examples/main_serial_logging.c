/**
 * \file libesoup/examples/main_serial_logging.c
 *
 * Example main.c file for using serial logging.
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
#define DEBUG_FILE TRUE
#include "libesoup_config.h"

#include "libesoup/comms/uart/uart.h"

#if defined(SYS_SERIAL_LOGGING)
#include <stdio.h>

static const char *TAG = "Main";
#endif


#include "libesoup/logger/serial_log.h"

/*
 *
 */
int main() 
{
        result_t rc;
        uint8_t  x = 0x12;
        uint16_t y = 0x1234;
        uint32_t z = 0x12345678;
        
        rc = libesoup_init();

        /*
         * Initialise the UART management data structures. Needed for Serial
         * Logging.
         */
#ifdef SYS_SERIAL_LOGGING
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        serial_log(LOG_DEBUG, TAG, "Testing\n\r");
	LOG_D("string %s\n\r", "Bingo");
	LOG_D("uint8_t  Hex 0x%x\n\r", x);
	LOG_D("uint8_t  Dec %d\n\r", x);
#endif // DEBUG
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
	LOG_I("uint16_t Hex 0x%x\n\r", y);
	LOG_I("uint16_t Dec %d\n\r", y);
#endif // INFO
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
	LOG_W("uint32_t 0x%lx\n\r", z);
#endif
#if (SYS_LOG_LEVEL <= LOG_ERROR)
	LOG_E("ERROR Oops\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
        while(1) {
        }

        return(0);
}

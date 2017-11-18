/**
 * \file libesoup/examples/main_serial_logging.c
 *
 * Example main.c file for using serial logging.
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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

#define DEBUG_FILE TRUE
#define TAG "Main"

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
        
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        serial_log(TAG, "Debug logging statement uint_8 x is 0x%x\n\r", x);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
        LOG_I("Info logging statement uin16_t y is 0x%x\n\r", y);
#endif
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
        LOG_W("Warning logging statement uin32_t z is 0x%lx\n\r", z);
#endif
#if (SYS_LOG_LEVEL <= LOG_ERROR)
        LOG_W("ERROR logging statement\n\r");
#endif
        while(1) {
        }

        /*
         * Cleanup serial logging.
         */
        rc = serial_logging_exit();
        
        return(0);
}

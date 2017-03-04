/*
 * File:   main.c
 * Author: john
 *
 * Created on 26 September 2016, 20:12
 */
//#include <p33EP256MU806.h>

/*
 * Set up the configuration words of the processor:
 */

#include "es_lib/../system.h"

#define DEBUG_FILE TRUE
#define TAG "Main"

#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/hw_timers.h"
#include "es_lib/timers/sw_timers.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif

static void expiry(timer_t timer_id, union sigval);

int main(void)
{
        result_t     rc;
        union sigval data;
	timer_t      timer;

	cpu_init();
#ifdef MCP
        /*
         * Microchip platforms depend on a Hardware timer for the regular
         * tick so Hardware timers have to be initialised, and Serial logging
	 * if required.
         */
#if (SYS_LOG_LEVEL != NO_LOGGING)
        serial_logging_init();
#endif
        hw_timer_init();
#endif
        sw_timer_init();

        rc = sw_timer_start(SECONDS_TO_TICKS(30), expiry, data, &timer);
        
        if(rc != SUCCESS) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
	        log_e(TAG, "Failed to start HW Timer\n\r");
#endif
        }
        
        while(1) {
#ifdef MCP
                CHECK_TIMERS();
#endif
        }
        return 0;
}

static void expiry(timer_t timer_id  __attribute__((unused)), union sigval data __attribute__((unused)))
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "exp_func()\n\r");
#endif
}

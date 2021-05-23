/**
 * @file libesoup/examples/main_serial_logging.c
 *
 * @author John Whitmore
 *
 * @brief Example main.c file for using serial logging.
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
 */
#include "libesoup_config.h"
#ifdef SYS_EXAMPLE_SERIAL_LOGGING

#include "libesoup/comms/uart/uart.h"

#if defined(SYS_SERIAL_LOGGING)
#define DEBUG_FILE
static const char *TAG = "Main";
#include "libesoup/logger/serial_log.h"
#endif
#include "libesoup/timers/delay.h"
#include "libesoup/timers/sw_timers.h"

#ifdef SYS_SW_TIMERS
static void expiry(timer_id timer, union sigval);
#endif

/*
 *
 */
int main()
{
        result_t      rc;
        uint8_t       x = 0x12;
        uint16_t      y = 0x1234;
        uint32_t      z = 0x12345678;
	struct period period;

#ifdef SYS_SW_TIMERS
	timer_id         timer;
	struct timer_req request;
#endif

        rc = libesoup_init();

	/*
	 * Allow the clock to stabalise.
	 */
	period.units    = mSeconds;
	period.duration = 500;
	delay(&period);

	if (rc < 0) {
		LOG_E("ERROR Oops\n\r");
	}

        LOG_D("Testing\n\r");
	LOG_D("string %s\n\r", "Bingo");
	LOG_D("uint8_t Hex [0x12] 0x%x\n\r", x);
	LOG_D("uint8_t Dec [18] %d\n\r", x);
	LOG_I("uint16_t Hex [0x1234] 0x%x\n\r", y);
	LOG_I("uint16_t Dec [4660] %d\n\r", y);
	LOG_W("uint32_t Hex [0x12345678] 0x%lx\n\r", z);
	LOG_W("uint32_t Dec [305419896] %ld\n\r", z);
	LOG_E("Test of ERROR Message\n\r");

	/*
	 * If the build includes SW Timers start a ping pong timer one
	 * can load the serial logging buffer and the other can check
	 * that it's emptied
	 */
#ifdef SYS_SW_TIMERS
	request.period.units    = Seconds;
	request.period.duration = 10;
	request.type            = repeat_expiry;
	request.exp_fn          = expiry;
	request.data.sival_int  = 0x00;

	timer = sw_timer_start(&request);
	if(timer < 0) {
		LOG_E("Failed to start SW Timer\n\r");
	}
#endif

        while(1) {
		libesoup_tasks();
        }

        return(0);
}

/*
 * Expiry Function if SYS_SW_TIMERS defined
 */
#ifdef SYS_SW_TIMERS
static void expiry(timer_id timer, union sigval data)
{
	static uint16_t  count = 0;

	if(count % 2 == 0) {
		/*
		 * Ping so load up the logging serial buffers
		 */
		LOG_D("Expiry - 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,\n\r");
	}

#ifdef XC16
#ifdef SYS_TEST_BUILD
        serial_printf("%d\n\r", serial_buffer_count());
#endif
#endif  // XC16

	count++;
}
#endif // SYS_SW_TIMERS
#endif // SYS_EXAMPLE_SERIAL_LOGGING

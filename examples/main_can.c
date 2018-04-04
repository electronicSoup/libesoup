/**
 * @file libesoup/examples/main_can.c
 *
 * @author John Whitmore
 *
 * Copyright 2017-2018 electronicSoup
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 3 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */
//#define TX_NODE

#include "libesoup_config.h"

#include <stdlib.h>

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "Main";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include "libesoup/timers/sw_timers.h"
#include "libesoup/timers/delay.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/hardware/eeprom.h"
#include "libesoup/status/status.h"

void system_status_handler(status_source_t source, int16_t status, int16_t data);

#ifdef SYS_SW_TIMERS
static void expiry(timer_id timer, union sigval);
#endif
static void frame_handler(can_frame *);

int main(void)
{
	result_t         rc = 0;
	can_l2_target_t  target;
#ifdef SYS_SW_TIMERS
	timer_id         timer;
	struct timer_req request;
#endif

	rc = libesoup_init();
	if(rc < 0) {
		// Error Condition
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to init libesoup\n\r");
#endif
	}

	/*
	 * Allow the clock to settle
	 */
	delay(mSeconds, 500);
	
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("************************\n\r");
	LOG_D("***   CAN Bus Node   ***\n\r");
	LOG_D("***   %ldMHz         ***\n\r", sys_clock_freq);
	LOG_D("************************\n\r");
#endif

	delay(mSeconds, 500);
#if (defined(SYS_ISO15765) || defined(SYS_ISO11783)) || defined(SYS_TEST_L3_ADDRESS)
	rc = can_init(baud_250K, 0xff, system_status_handler, normal);  // Includes L3 Address
#else
 	rc = can_init(baud_250K, system_status_handler, normal);
#endif
	if(rc < 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to initialise CAN Bus\n\r");
#endif
	}

	/*
	 * If the build includes SW Timers start a ping pong timer one
	 * can load the serial logging buffer and the other can check
	 * that it's emptied
	 */
#if (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID))
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Tx Node\n\r");
#endif
#ifdef SYS_SW_TIMERS
	request.units = Seconds;
	request.duration = 10;
	request.type = repeat;
	request.exp_fn = expiry;
	request.data.sival_int = 0x00;
	
	timer = sw_timer_start(&request);
	if(timer < 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start SW Timer\n\r");
#endif		
	}
#endif	//  SYS_SW_TIMERS
#endif  //  (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID))

	/*
	 * Register a frame handler
	 */
#if defined(SYS_CAN_PING_FRAME_ID)
	target.filter = SYS_CAN_PING_FRAME_ID;
#else
	target.filter = 0x777;
#endif
	target.mask   = CAN_SFF_MASK;
	target.handler = frame_handler;

	rc = frame_dispatch_reg_handler(&target);
	if(rc < 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to register frame handler\n\r");
#endif		
	}
	/*
	 * Enter the main loop
	 */
	LOG_D("Entering the main loop\n\r");
	LOG_D("***   %ldMHz         ***\n\r", sys_clock_freq);
	while(TRUE) {
#ifdef SYS_SW_TIMERS
		CHECK_TIMERS();
#endif
		can_tasks();
	}
}

void system_status_handler(status_source_t source, int16_t status, int16_t data)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("status_handler()\n\r");
#endif
	switch(source) {
	case can_bus_l2_status:
		switch(status) {
		case(can_l2_connecting):
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Connecting\n\r");
#endif
			break;
		case can_l2_connected:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Connected\n\r");
#endif
			break;
		default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Status? %d\n\r", status);
#endif		
			break;
		}
		break;
#if defined(SYS_CAN_DCNCP)
	case can_bus_dcncp_status:
		break;
#endif
#if defined(ISO15765)
	case iso15765_status:
		break;
#endif
#if defined(ISO11783)
	case iso11783_status:
		break;
#endif
	default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Status Src? %d\n\r", source);
#endif		
	}
}

/*
 * Expiry Function if SYS_SW_TIMERS defined
 */
#if (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID) && defined (SYS_SW_TIMERS))
static void expiry(timer_id timer, union sigval data)
{
	result_t  rc;
	can_frame frame;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Tx Frame\n\r");
#endif
	frame.can_id = 0x777;
	frame.can_dlc = 0x00;

	rc = can_l2_tx_frame(&frame);
	if(rc < 0) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to send CAN Frame\n\r");
#endif		
	}
}
#endif // (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID) && defined (SYS_SW_TIMERS))

static void frame_handler(can_frame *frame)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("handle(0x%lx)\n\r", frame->can_id);
#endif
}

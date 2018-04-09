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
#ifdef SYS_CAN_DCNCP
#include "libesoup/comms/can/dcncp/dcncp_can.h"
#endif

void system_status_handler(status_source_t source, int16_t status, int16_t data);

#ifdef SYS_SW_TIMERS
#if (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID))
static void expiry(timer_id timer, union sigval);
#endif
#endif
static void frame_handler(can_frame *);

int main(void)
{
	result_t         rc = 0;
	can_l2_target_t  target;
#ifdef SYS_SW_TIMERS
#if (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID))
	timer_id         timer;
	struct timer_req request;
#endif
#endif
	rc = libesoup_init();
	if(rc < 0) {
		// Error Condition
		LOG_E("Failed to init libesoup\n\r");
	}

	/*
	 * Allow the clock to settle
	 */
	delay(mSeconds, 500);
	
	LOG_D("************************\n\r");
	LOG_D("***   CAN Bus Node   ***\n\r");
	LOG_D("***   %ldMHz         ***\n\r", sys_clock_freq);
	LOG_D("************************\n\r");

	delay(mSeconds, 500);
#ifdef SYS_CAN_BAUD_AUTO_DETECT
#if (defined(SYS_ISO15765) || defined(SYS_ISO11783)) || defined(SYS_TEST_L3_ADDRESS)
	rc = can_init(no_baud, 0xff, system_status_handler, normal);  // Includes L3 Address
#else
 	rc = can_init(no_baud, system_status_handler, normal);
#endif //  SYS_ISO15765 || SYS_ISO11783 || SYS_TEST_L3_ADDRESS
#else
#if (defined(SYS_ISO15765) || defined(SYS_ISO11783)) || defined(SYS_TEST_L3_ADDRESS)
	/*
	 * A Layer 3 noded address of 0xff is the ISO11783 Broadcast address so
	 * it should not be used as a Layer 3 address. If the can_init() function
	 * is passed this L3 address it will randomly select an address and 
	 * attempt to register that address on the network
	 */
//	rc = can_init(baud_250K, BROADCAST_NODE_ADDRESS, system_status_handler, normal);  // Includes L3 Address
	rc = can_init(baud_250K, 0x12, system_status_handler, normal);  // Includes L3 Address
#else
 	rc = can_init(baud_250K, system_status_handler, normal);
#endif //  SYS_ISO15765 || SYS_ISO11783 || SYS_TEST_L3_ADDRESS
#endif //  SYS_CAN_BAUD_AUTO_DETECT
	if(rc < 0) {
		LOG_E("Failed to initialise CAN Bus\n\r");
	}

	/*
	 * If the build includes SW Timers start a ping pong timer one
	 * can load the serial logging buffer and the other can check
	 * that it's emptied
	 */
#if (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID))
	LOG_D("Tx Node\n\r");
#ifdef SYS_SW_TIMERS
	request.units = Seconds;
	request.duration = 10;
	request.type = repeat;
	request.exp_fn = expiry;
	request.data.sival_int = 0x00;
	
	timer = sw_timer_start(&request);
	if(timer < 0) {
		LOG_E("Failed to start SW Timer\n\r");
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
		LOG_E("Failed to register frame handler\n\r");
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
	LOG_D("status_handler()\n\r");
	switch(source) {
	case can_bus_l2_status:
		switch(status) {
		case can_l2_detecting_baud:
			LOG_D("Bit Rate Auto Detect\n\r");
			break;
		case can_l2_connecting:
			LOG_D("Connecting\n\r");
			break;
		case can_l2_connected:
			LOG_D("Connected - %s\n\r", can_baud_rate_strings[data]);
			break;
		default:
			LOG_E("Status? %d\n\r", status);
			break;
		}
		break;
#if defined(SYS_CAN_DCNCP)
	case can_bus_dcncp_status:
		switch(status) {
		case can_dcncp_l3_address_registered:
			LOG_D("CAN L3 Address registered 0x%x\n\r", (uint8_t)data);
			break;
		}
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
		LOG_E("Status Src? %d\n\r", source);
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

	LOG_D("Tx Frame\n\r");
	frame.can_id = 0x777;
	frame.can_dlc = 0x00;

	rc = can_l2_tx_frame(&frame);
	if(rc < 0) {
		LOG_E("Failed to send CAN Frame\n\r");
	}
}
#endif // (defined(TX_NODE) && !defined(SYS_CAN_PING_FRAME_ID) && defined (SYS_SW_TIMERS))

static void frame_handler(can_frame *frame)
{
	LOG_D("handle(0x%lx)\n\r", frame->can_id);
}

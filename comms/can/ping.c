/*
 *
 * libesoup/comms/can/ping.c
 *
 * CAN Bus Ping Protocol functionality
 *
 * Copyright 2017-2018 electronicSoup Limited
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
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

#ifdef SYS_CAN_PING_PROTOCOL

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
const char *TAG = "CAN_PING";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include <stdlib.h>
#include "libesoup/comms/can/can.h"
#include "libesoup/timers/sw_timers.h"

#define CAN_PING_FRAME_ID 0x555

/**
 * \brief Network Idle functionality
 *
 * We need to have CAN Bus Network traffic so that when devices connect
 * to listen to the Network to establish the CAN Bus Baud Rate. As a result
 * we'll keep a timer and if nothing has been received or transmitted in this
 * time we'll fire a ping message.
 */

static  timer_id         ping_timer;
static 	struct timer_req timer_request;


void restart_ping_timer(void);
static void ping_network(timer_id timer, union sigval data);

void can_ping_init(void)
{
	uint16_t duration;
	
	duration = (uint16_t) ((rand() % 500) + 1000);
	
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("CAN ping duration - %d mSeconds\n\r", duration);
#endif

	TIMER_INIT(ping_timer);
	timer_request.units          = mSeconds;
	timer_request.duration       = duration;
	timer_request.type           = single_shot;
	timer_request.exp_fn         = ping_network;
	timer_request.data.sival_int = 0x00;
	
        restart_ping_timer();
}

static void ping_network(timer_id timer, union sigval data)
{
	can_frame frame;

	TIMER_INIT(ping_timer);

	frame.can_id = CAN_PING_FRAME_ID;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame); // Transmission of frame will cause timer to restart
}

void restart_ping_timer(void)
{
	result_t  rc;

	if(ping_timer != BAD_TIMER_ID) {
		rc = sw_timer_cancel(ping_timer);
		if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Failed to cancel SW Timer\n\r");
#endif
		}
		TIMER_INIT(ping_timer);
	}
	
	rc = sw_timer_start(&ping_timer, &timer_request);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start SW Timer\n\r");
#endif		
	}
}

#endif // #ifdef SYS_CAN_PING_PROTOCOL

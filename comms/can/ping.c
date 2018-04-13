/**
 * @file libesoup/comms/can/ping.c
 *
 * @author John Whitmore
 * 
 * @brief CAN Bus Ping Protocol functionality
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

#include <stdlib.h>   // rand()
#include "libesoup/errno.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/utils/rand.h"

#ifndef SYS_CAN_PING_FRAME_ID
#error libesoup_config.h should define the SYS_CAN_PING_FRAME_ID
#endif

#ifndef SYS_CAN_PING_IDLE_SPREAD
#error libesoup_config.h should define the SYS_CAN_PING_IDLE_SPREAD
#endif

#ifndef SYS_CAN_PING_IDLE_INTERVAL
#error libesoup_config.h should define the SYS_CAN_PING_IDLE_INTERVAL
#endif

#ifndef SYS_RAND
#error libesoup_config.h should define the SYS_RAND
#endif

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


result_t    restart_ping_timer(void);
static void ping_network(timer_id timer, union sigval data);

void can_ping_init(void)
{
	uint16_t duration;
	int16_t  tmp;
	
	tmp = (rand() % SYS_CAN_PING_IDLE_SPREAD) - (SYS_CAN_PING_IDLE_SPREAD/2);
	duration = (uint16_t) SYS_CAN_PING_IDLE_INTERVAL + tmp;
	
	ping_timer = 0xFF;
	
	LOG_D("CAN ping duration - %d mSeconds\n\r", duration);

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

	ping_timer = 0xFF;
		
//	LOG_D("CAN Ping\n\r");
	frame.can_id = SYS_CAN_PING_FRAME_ID;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame); // Transmission of frame will cause timer to restart
}

result_t restart_ping_timer(void)
{
	result_t  rc;

	if(ping_timer != 0xff) {
		rc = sw_timer_cancel(ping_timer);
		RC_CHECK
	}
	
	rc = sw_timer_start(&timer_request);
	RC_CHECK
	ping_timer = rc;
	return(rc);
}

#endif // #ifdef SYS_CAN_PING_PROTOCOL

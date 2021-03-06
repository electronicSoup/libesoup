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

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_SLAVE)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
//#undef DEBUG_FILE
const char *TAG = "CAN_PING";
#include "libesoup/logger/serial_log.h"
#endif // SYS_SERIAL_LOGGING

#include <stdlib.h>   // rand()
#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/comms/can/es_control/es_control.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/utils/rand.h"

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

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
static  timer_id         ping_timer;
static 	struct timer_req timer_request;
#endif

result_t    restart_ping_timer(void);

#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
static void status_led_off(timer_id timer, union sigval data)
{
	gpio_set(BRD_STATUS_LED_PIN, GPIO_MODE_DIGITAL_OUTPUT, 0);
}
#endif

#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
static void status_led_on(void)
{
	result_t         rc;
	struct timer_req led_off_timer;

	rc = gpio_set(BRD_STATUS_LED_PIN, GPIO_MODE_DIGITAL_OUTPUT, 1);

	led_off_timer.period.units    = mSeconds;
	led_off_timer.period.duration = 50;
	led_off_timer.type            = single_shot;
	led_off_timer.exp_fn          = status_led_off;
	led_off_timer.data.sival_int  = 0x00;
	
	rc = sw_timer_start(&led_off_timer);	
}
#endif

#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
void ping_frame_handler(can_frame *frame)
{
	status_led_on();
}
#endif

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
static void ping_network(timer_id timer, union sigval data)
{
	can_frame frame;

	ping_timer = BAD_TIMER_ID;
		
	LOG_D("CAN Ping\n\r");
	frame.can_id = ESC_PING_PROTOCOL_ID;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame); // Transmission of frame will cause timer to restart

#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
	status_led_on();
#endif
}
#endif

result_t can_ping_init(void)
{
	result_t               rc;
#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
	uint16_t               duration;
#endif
#if defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
	can_frame frame;
#endif
#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER)
	int16_t                tmp;
#endif
#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
	can_l2_target_t        target;
#endif	

	LOG_D("init()\n\r");
#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
	/*
	 * If required set up the LED Status indication LED
	 */
	rc = gpio_set(BRD_STATUS_LED_PIN, GPIO_MODE_DIGITAL_OUTPUT, 0);

	/*
	 * And set up a handler for a PING Frame on the network
	 */
	target.filter  = ESC_PING_PROTOCOL_ID;
	target.mask    = ESC_RTR_MASK | ESC_TYPE_MASK;
	target.handler = ping_frame_handler;
	rc = frame_dispatch_reg_handler(&target);
#endif
#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER)
	tmp = (rand() % SYS_CAN_PING_IDLE_SPREAD) - (SYS_CAN_PING_IDLE_SPREAD/2);
	duration = (uint16_t) SYS_CAN_PING_IDLE_INTERVAL + tmp;
#elif defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
	duration = (uint16_t) SYS_CAN_PING_IDLE_INTERVAL;
#endif
	/*
	 * If this node is the centralised master just send an initial
	 * ping frame to wake up the network
	 */
#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
	ping_timer = BAD_TIMER_ID;
	
	LOG_D("CAN ping duration - %d mSeconds\n\r", duration);
	timer_request.period.units    = mSeconds;
	timer_request.period.duration = duration;
	timer_request.type            = single_shot;
	timer_request.exp_fn          = ping_network;
	timer_request.data.sival_int  = 0x00;
	
	rc = sw_timer_start(&timer_request);
	RC_CHECK
	ping_timer = rc;
#endif
#if defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
	frame.can_id = ESC_PING_PROTOCOL_ID;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame); // Transmission of frame will cause timer to restart

#if defined(BRD_STATUS_LED_PIN) && defined (SYS_CAN_PING_PROTOCOL_LED)
	status_led_on();
#endif
#endif
	return(0);
}

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER)
result_t restart_ping_timer(void)
{
	result_t  rc;

	LOG_D("Ping rst\n\r");
	if (ping_timer != BAD_TIMER_ID) {
		rc = sw_timer_cancel(&ping_timer);
		RC_CHECK
	}
	
	rc = sw_timer_start(&timer_request);
	RC_CHECK
	ping_timer = rc;
	return(rc);
}
#endif

#endif // defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_SLAVE)


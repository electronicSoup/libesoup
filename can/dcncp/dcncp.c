/**
 *
 * \file es_lib/can/dcncp/dcncp.c
 *
 * Dynamic CAN Node Configuration Protocol 
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "es_lib/core.h"
#include "system.h"

#if defined(MCP)
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"
#include "es_lib/can/es_can.h"
#elif defined(ES_LINUX)
#include "serial.h"
#endif

#include "es_lib/can/dcncp/dcncp.h"
#include "es_lib/timers/timers.h"
#if defined(CAN_LAYER_3)
#include "es_lib/logger/net.h"
#endif

#define TAG "CAN_DCNCP"

static es_timer dcncp_network_baudrate_req_timer;
static void exp_resend_network_baud_chage_req(timer_t timer_id, union sigval data);
static void exp_network_baud_chage_req(timer_t timer_id, union sigval data);

#if defined(CAN_LAYER_3)
static es_timer l3_send_reg_req_timer;
static es_timer l3_node_reg_timer;

#ifdef CAN_NET_LOGGER
static can_frame local_net_logger_frame;
static es_timer local_net_logger_timer;

#define LOCAL_NET_LOGGER_MSG_PERIOD SECONDS_TO_TICKS(5)
#endif // CAN_NET_LOGGER

static u8 dcncp_l3_address;
u8 dcncp_get_can_l3_address(void);

#ifdef CAN_NET_LOGGER
static void exp_net_logger_ping(timer_t timer_id __attribute__((unused)), union sigval);
#endif //CAN_NET_LOGGER

static void exp_send_address_register_request(timer_t timer_id, union sigval data);
static void exp_node_address_registered(timer_t timer_id __attribute__((unused)), union sigval data);
static void exp_send_node_addr_report(timer_t timer_id, union sigval data);
#endif  // CAN_LAYER_3

static void can_l2_msg_handler(can_frame *msg);

#ifdef TEST
void send_test_msg(timer_t timer_id, union sigval data);
BYTE other_node = 0xff;
#endif

static can_status_t status;
static void (*status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud) = NULL;

void dcncp_init(void (*arg_status_handler)(u8 mask, can_status_t status, can_baud_rate_t baud))
{
	can_l2_target_t target;
#if defined(CAN_LAYER_3)
	result_t rc;
#endif

        status_handler = arg_status_handler;
        status.byte = 0x00;

	TIMER_INIT(dcncp_network_baudrate_req_timer);

#if defined(CAN_LAYER_3)
	TIMER_INIT(l3_send_reg_req_timer);
	TIMER_INIT(l3_node_reg_timer);
#endif

#ifdef CAN_NET_LOGGER
	TIMER_INIT(local_net_logger_timer);
#endif // CAN_NET_LOGGER

	/*
	 * Add the Layer 2 and Layer 3 Can Message Handlers
	 */
	target.mask = (u32)CAN_DCNCP_MASK;
	target.filter = (u32)CAN_DCNCP_FILTER;
	target.handler = can_l2_msg_handler;

	LOG_D("Node Address Register handler Mask 0x%lx, Filter 0x%lx\n\r", target.mask, target.filter);
	can_l2_reg_handler(&target);

#if defined(CAN_LAYER_3)
	/*
	 * If we're going to use layer 3 we need to initialise a Layer 3 address to use
	 * Create a random timer for firing node register message. If all network nodes
	 * powerup at the same time we don't want to flood the network with register
	 * requests so we'll hold off. Random time between 1 and 5 seconds.
	 */
	rc = timer_start(MILLI_SECONDS_TO_TICKS( (u16)((rand() % 4000) + 1000)),
		         exp_send_address_register_request,
			 (union sigval)(void *) NULL,
			 &l3_send_reg_req_timer);
	if(rc != SUCCESS) {
		LOG_E("Failed to start Register Timer\n\r");
	}
#endif
        status.bit_field.dcncp_initialised = 1;

        if(status_handler)
		status_handler(DCNCP_INIT_STATUS_MASK, status, no_baud);
}

void dcncp_request_network_baud_change(can_baud_rate_t baud)
{
	result_t     rc;
	can_frame    msg;
	union sigval data;

	LOG_D("dcncp_request_network_baud_change()\n\r");

        if(!status.bit_field.dcncp_initialised) {
		LOG_W("Ignoring DCNCP not ready!\n\r");
		return;
	}

	if(baud == can_l2_get_baudrate()) {
		LOG_W("Ignoring spurious request!\n\r");
		return;
	}
	
	/*
	 * Send a Layer 2 message to inform network of impending change!
	 */
	TIMER_INIT(dcncp_network_baudrate_req_timer);

	data.sival_int = baud;
	data.sival_int = (data.sival_int << 8) | 2;

	/*
	 * Create a 1 Second timer to repeat this message.
	 */
	rc = timer_start(SECONDS_TO_TICKS(1),
	                 exp_resend_network_baud_chage_req,
			 data,
			 &dcncp_network_baudrate_req_timer);
	if (rc != SUCCESS) {
		LOG_E("Failed to start BaudRate change Request Timer\n\r");
		return;
	}

	LOG_D("send network baudrate change request\n\r");

	msg.can_id = CAN_DCNCP_NetworkChangeBaudRateReq;
	msg.can_dlc = 2;
	msg.data[0] = baud;
	msg.data[1] = 3;      // 3 Seconds till Change

	can_l2_tx_frame(&msg);
}

static void exp_resend_network_baud_chage_req(timer_t timer_id, union sigval data)
{
	can_frame msg;
	result_t rc;
	u8 time_left;
	can_baud_rate_t baud;

	LOG_D("exp_resend_network_baud_chage_req()\n\r");
	/*
	 * Resend a Layer 2 message to inform network of impending change!
	 */
	TIMER_INIT(dcncp_network_baudrate_req_timer);

	time_left = data.sival_int & 0xff;
	baud = (data.sival_int >> 8) & 0xff;

	LOG_D("Time left %d\n\r", time_left);

	if(time_left > 0) {
		data.sival_int--;

		/*
		 * Create a 1 Second timer to repeat this message.
		 */
		rc = timer_start(SECONDS_TO_TICKS(1),
		                 exp_resend_network_baud_chage_req,
				 data,
				 &dcncp_network_baudrate_req_timer);
		if (rc != SUCCESS) {
			LOG_E("Failed to start BaudRate change Request Timer\n\r");
			return;
		}

		LOG_D("send network baudrate change request\n\r");

		msg.can_id = CAN_DCNCP_NetworkChangeBaudRateReq;
		msg.can_dlc = 2;
		msg.data[0] = baud;
		msg.data[1] = time_left;

		can_l2_tx_frame(&msg);
	} else {
		LOG_D("TIME UP Change Baud Rate\n\r");
		can_l2_set_node_baudrate(baud);
	}
}

static void exp_network_baud_chage_req(timer_t timer_id, union sigval data)
{
	LOG_D("!!! exp_network_baud_chage_req() !!!\n\r");
	can_l2_set_node_baudrate((can_baud_rate_t)(data.sival_int & 0xff));
}

#if defined(CAN_LAYER_3)
u8 dcncp_get_can_l3_address(void)
{
	return(dcncp_l3_address);
}
#endif

#if defined(CAN_LAYER_3)
void exp_send_address_register_request(timer_t timer_id __attribute__((unused)), union sigval data)
{
	can_frame msg;
	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;

	TIMER_INIT(l3_send_reg_req_timer);

	dcncp_l3_address = node_get_can_l3_address();

	LOG_D("sendRegisterReq(%x)\n\r", (u16) dcncp_l3_address);

	msg.can_id = CAN_DCNCP_AddressRegisterReq;
	msg.can_dlc = 1;
	msg.data[0] = dcncp_l3_address;

	can_l2_tx_frame(&msg);

	/*
	 *Create a 2 Second timer if no reject is recieved in that time
	 * this node shall consider itself registered
	 */
	result = timer_start(SECONDS_TO_TICKS(2), 
	                     exp_node_address_registered,
			     (union sigval)(void *) NULL,
			     &l3_node_reg_timer);
	if (result != SUCCESS) {
		LOG_E("Failed to start Node Registered Timer\n\r");
	}
}
#endif

#if defined(CAN_LAYER_3)
void exp_node_address_registered(timer_t timer_id __attribute__((unused)), union sigval data)
{
//	u8 address;
//	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;
	TIMER_INIT(l3_node_reg_timer);

	LOG_D("nodeRegistered()\n\r");

        status.bit_field.dcncp_l3_valid = 1;

        if(status_handler) {
		status_handler(DCNCP_L3_ADDRESS_STATUS_MASK, status, no_baud);
	}
#ifdef TEST
	result = start_timer(SECONDS_TO_TICKS(1), send_test_msg, (union sigval)(void *) NULL, &send_reg_req_timer);
	if (result != SUCCESS) {
		DEBUG_E("Failed to start Send Register Request Timer\n\r");
	}
#endif
}
#endif

void can_l2_msg_handler(can_frame *msg)
{
	result_t rc;
	union sigval data;
#if defined(CAN_LAYER_3)
	can_frame txMsg;
	es_timer timer;
#endif
	LOG_D("DCNCP message received 0x%lx\n\r", msg->can_id);
	if (msg->can_id == CAN_DCNCP_AddressRegisterReq) {
#if defined(CAN_LAYER_3)
		if(msg->data[0] == dcncp_l3_address) {
			if(status.bit_field.dcncp_l3_valid) {
				LOG_D("reject Register Request\n\r");
				txMsg.can_id = CAN_DCNCP_AddressRegisterReject;
				txMsg.can_dlc = 1;
				txMsg.data[0] = dcncp_l3_address;

				can_l2_tx_frame(&txMsg);
			} else {
				LOG_D("Register Node Address clash\n\r");
				/*
				 *Have to create a new node address for this node
				 *cancel the timers
				 */
				rc = timer_cancel(&l3_send_reg_req_timer);
				rc = timer_cancel(&l3_node_reg_timer);

				dcncp_l3_address = node_get_can_l3_address();

				exp_send_address_register_request(0xff, (union sigval)(void *)NULL);
			}
		}
#endif  // CAN_LAYER_3
	} else if(msg->can_id == CAN_DCNCP_AddressRegisterReject) {
#if defined(CAN_LAYER_3)
		if(msg->data[0] == dcncp_l3_address) {
			if(status.bit_field.dcncp_l3_valid) {
				LOG_E("Sending Can Error Message\n\r");
				txMsg.can_id = CAN_DCNCP_NodeAddressError;
				txMsg.can_dlc = 1;
				txMsg.data[0] = dcncp_l3_address;

				can_l2_tx_frame(&txMsg);
			} else {
				/*
				 *Have to create a new node address for this node
				 *cancel the timers
				 */
				rc = timer_cancel(&l3_send_reg_req_timer);
				rc = timer_cancel(&l3_node_reg_timer);

				dcncp_l3_address = node_get_can_l3_address();

				exp_send_address_register_request(0xff, (union sigval)(void *)NULL);
			}
		}
#endif
	} else if (msg->can_id == CAN_DCNCP_NodeAddressReportReq) {
#if defined(CAN_LAYER_3)
		// Create a random timer between 100 and  1000 miliSeconds for firing node report message
		rc = timer_start(MILLI_SECONDS_TO_TICKS((u16) ((rand() % 900) + 100)), exp_send_node_addr_report, (union sigval)(void *)NULL, &timer);
		if (rc != SUCCESS) {
			LOG_E("Failed to start Node Registered Timer\n\r");
		}
#endif
	} else if (msg->can_id == CAN_DCNCP_NodeAddressReporting) {
		if(msg->data[0]) {
			LOG_D("Foreign Node Rep Registered Node Address 0x%x\n\r", msg->data[1]);
		} else {
			LOG_D("Foreign Node Rep UN-Registered Node Address 0x%x\n\r", msg->data[1]);
		}
	} else if (msg->can_id == CAN_DCNCP_NetworkChangeBaudRateReq) {
		LOG_D("***Baud Rate Change Request New Baud Rate %s Time left %dS\n\r", can_baud_rate_strings[msg->data[0]], msg->data[1]);

		timer_cancel(&dcncp_network_baudrate_req_timer);
		TIMER_INIT(dcncp_network_baudrate_req_timer);

		data.sival_int = msg->data[0];   // Baudrate

		/*
		 * Create a timer to for time given in message.
		 */
		rc = timer_start(SECONDS_TO_TICKS(msg->data[1]), exp_network_baud_chage_req, data, &dcncp_network_baudrate_req_timer);
		if (rc != SUCCESS) {
			LOG_E("Failed to start BaudRate change Request Timer\n\r");
			return;
		}

		//		can_l2_set_node_baudrate(msg->data[0]);
	} else if (msg->can_id == CAN_DCNCP_NodePingMessage) {
#if DEBUG_LEVEL <= LOG_DEBUG
		printf(".");
#endif
	} else if (msg->can_id == CAN_DCNCP_RegisterNetLogger) {
		LOG_D("Received NetLogger Registration Message\n\r");
#ifdef CAN_NET_LOGGING
		net_logger_register_remote(msg->data[0], msg->data[1]);
#endif // CAN_NET_LOGGING
	} else if (msg->can_id == CAN_DCNCP_UnRegisterNetLogger) {
		LOG_D("Received NetLogger UnRegistration Message\n\r");
#ifdef CAN_NET_LOGGING
		net_logger_unregister_remote(msg->data[0]);
#endif // CAN_NET_LOGGING
	} else {
		LOG_W("Node Unrecognised Request %lx \n\r", msg->can_id);
	}
}

#if defined(CAN_LAYER_3)
void exp_send_node_addr_report(timer_t timer_id __attribute__((unused)), union sigval data)
{
	can_frame txMsg;

	/*
	 * Clear the compiler warning
	 */
	data = data;

	LOG_D("exp_send_node_addr_report(Address %x)\n\r", dcncp_l3_address);

	txMsg.can_id = CAN_DCNCP_NodeAddressReporting;
	txMsg.can_dlc = 2;
	txMsg.data[1] = dcncp_l3_address;

	if (status.bit_field.dcncp_l3_valid)
		txMsg.data[0] = TRUE;
	else
		txMsg.data[0] = FALSE;

	can_l2_tx_frame(&txMsg);
}
#endif

/*
 * Net Logger Stuff
 */
#if defined(CAN_NET_LOGGER)
result_t dcncp_register_this_node_net_logger(log_level_t level)
{
	LOG_D("register_this_node_net_logger()\n\r");

	if(!l3_initialised())
		return(ERR_L3_UNINITIALISED);

	local_net_logger_frame.can_id = CAN_DCNCP_RegisterNetLogger;
	local_net_logger_frame.can_dlc = 2;
	local_net_logger_frame.data[0] = dcncp_l3_address;
	local_net_logger_frame.data[1] = level;

	can_l2_tx_frame(&local_net_logger_frame);
	LOG_D("NetLogger message sent\n\r");
	timer_start(LOCAL_NET_LOGGER_MSG_PERIOD, exp_net_logger_ping, (union sigval)(void *) NULL, &local_net_logger_timer);

	return (SUCCESS);
}
#endif // CAN_NET_LOGGER

#if defined(CAN_NET_LOGGER)
void exp_net_logger_ping(timer_t timer_id __attribute__((unused)), union sigval data)
{
	data = data;

	can_l2_tx_frame(&local_net_logger_frame);
	LOG_D("NetLogger message sent\n\r");
	timer_start(LOCAL_NET_LOGGER_MSG_PERIOD, exp_net_logger_ping, (union sigval)(void *)NULL, &local_net_logger_timer);
}
#endif // CAN_NET_LOGGER

#if defined(CAN_NET_LOGGER)
result_t dcncp_unregister_this_node_net_logger()
{
	can_frame txMsg;

	LOG_D("DeRegAsNetLogger()\n\r");

	txMsg.can_id = CAN_DCNCP_UnRegisterNetLogger;
	txMsg.can_dlc = 1;
	txMsg.data[0] = dcncp_l3_address;

	can_l2_tx_frame(&txMsg);
	LOG_D("CancelNetLogger message sent\n\r");

	if(local_net_logger_timer.status == ACTIVE)
		timer_cancel(&local_net_logger_timer);

	return (SUCCESS);
}
#endif // CAN_NET_LOGGER

void dcncp_send_ping(void)
{
	can_frame frame;

	frame.can_id = CAN_DCNCP_NodePingMessage;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame);
	LOG_D("Ping message sent\n\r");
}

#if defined(CAN_LAYER_3)
#ifdef TEST
void send_test_msg(timer_t timer_id __attribute__((unused)), union sigval data __attribute__((unused)))
{
	u8 buffer[70];
	u8 loop;
	result_t result;

	static u8 sizeToSend = 13;
	l3_can_msg_t msg;

	for(loop = 0; loop < 70; loop++)
		buffer[loop] = loop + 1;

	DEBUG_D("\n\r\n\r*** sendTestL3Msg() size %d\n\r", sizeToSend);

	if(other_node != 0xff) {
		msg.address = other_node;
		msg.data = buffer;
		msg.size = sizeToSend;
		msg.protocol = 10;

		l3_tx_msg(&msg);
		sizeToSend++;
		result = start_timer(SECONDS_TO_TICKS(10), send_test_msg, (union sigval)(void *)NULL, &send_reg_rq_timer);
		if (result != SUCCESS) {
			DEBUG_D("Failed to start Send Register Request Timer\n\r");
		}
	} else {
		DEBUG_D("No Other Node\n\r");
	}
}
#endif
#endif

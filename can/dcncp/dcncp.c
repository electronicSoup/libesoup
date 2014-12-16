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
#include "es_lib/can/es_can.h"
#if defined(CAN_LAYER_3)
#include "es_lib/logger/net.h"
#endif

#define TAG "CAN_DCNCP"

//#define NodeInfo 0x01;

//extern void rxNetLogRegReq(u8 address, log_level_t level);
//extern void rxNetLogUnRegReq(u8 address);

#if defined(CAN_LAYER_3)
static es_timer send_reg_req_timer;
static es_timer node_reg_timer;
#endif

#if defined(CAN_LAYER_3)
static can_frame local_net_logger_frame;
static es_timer local_net_logger_timer;

#define LOCAL_NET_LOGGER_MSG_PERIOD SECONDS_TO_TICKS(5)

static void exp_net_logger_ping(timer_t timer_id __attribute__((unused)), union sigval);

static void exp_send_addr_reg_req(timer_t timer_id, union sigval data);
static void exp_node_addr_regd(timer_t timer_id, union sigval data);
static void exp_send_node_addr_report(timer_t timer_id, union sigval data);
#endif

static void can_l2_msg_handler(can_frame *msg);

#ifdef TEST
void send_test_msg(timer_t timer_id, union sigval data);
BYTE other_node = 0xff;
#endif

static can_status_t status;
static void (*status_handler)(u8 mask, can_status_t status, baud_rate_t baud) = NULL;

void dcncp_init(void (*arg_status_handler)(u8 mask, can_status_t status, baud_rate_t baud))
{
	can_l2_target_t target;
//	result_t result;

        status_handler = arg_status_handler;
        status.byte = 0x00;

#if defined(CAN_LAYER_3)
	TIMER_INIT(send_reg_req_timer);
	TIMER_INIT(local_net_logger_timer);
	TIMER_INIT(node_reg_timer);
#endif
	/*
	 * Add the Layer 2 and Layer 3 Can Message Handlers
	 */
	target.mask = (u32)CAN_DCNCP_MASK;
	target.filter = (u32)CAN_DCNCP_FILTER;
	target.handler = can_l2_msg_handler;

	LOG_D("Node Address Register handler Mask 0x%x, Filter 0x%x\n\r", target.mask, target.filter);
	can_l2_reg_handler(&target);

#if defined(CAN_LAYER_3)
	/*
	 * If we're going to use layer 3 we need to initialise a Layer 3 address to use
	 */
	// Create a random timer between 1 and 5 seconds for firing node register message
	result = timer_start(MILLI_SECONDS_TO_TICKS( (u16)((rand() % 4000) + 1000)), exp_send_addr_reg_req, (union sigval)(void *)NULL, &send_reg_req_timer);
	if(result != SUCCESS) {
		LOG_E("Failed to start Register Timer\n\r");
	}
#endif
        status.bit_field.dcncp_status |= DCNCP_Initialised;

        if(status_handler)
		status_handler(DCNCP_STATUS_MASK, status, no_baud);
}

#if defined(CAN_LAYER_3)
void exp_send_addr_reg_req(timer_t timer_id __attribute__((unused)), union sigval data)
{
	u8 address;
	can_frame msg;
	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;

	TIMER_INIT(send_reg_req_timer);

	get_l3_node_address(&address);

	LOG_D("sendRegisterReq(%x)\n\r", (u16) address);

	msg.can_id = CAN_DCNCP_AddressRegisterReq;
	msg.can_dlc = 1;
	msg.data[0] = address;

	can_l2_tx_frame(&msg);

	// Create a 2 Second timer if no reject is recieved in that time
	// this node shall consider itself registered
	result = timer_start(SECONDS_TO_TICKS(2), exp_nodeAddressRegistered, (union sigval)(void *) NULL, &nodeRegisteredTimer);
	if (result != SUCCESS) {
		LOG_E("Failed to start Node Registered Timer\n\r");
	}
}
#endif

#if defined(CAN_LAYER_3)
void exp_node_addr_regd(timer_t timer_id __attribute__((unused)), union sigval data)
{
//	u8 address;
//	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;
	TIMER_INIT(node_reg_timer);

	LOG_D("nodeRegistered()\n\r");

        status.bit_field.dcncp_status |= DCNCP_L3_Address_Not_Final;

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
#if defined(CAN_LAYER_3)
	u8 address;
	can_frame txMsg;
	es_timer timer;
	result_t result;
#endif
	LOG_D("Node Adress message received 0x%lx\n\r", msg->can_id);
	if (msg->can_id == CAN_DCNCP_AddressRegisterReq) {
#if defined(CAN_LAYER_3)
		get_l3_node_address(&address);

		if(msg->data[0] == address) {
			if(status.bit_field.dcncp_status & DCNCP_L3_Address_Finalised) {
				LOG_D("reject Register Request\n\r");
				txMsg.can_id = CAN_DCNCP_AddressRegisterReject;
				txMsg.can_dlc = 1;
				txMsg.data[0] = address;

				can_l2_tx_frame(&txMsg);
			} else {
				LOG_D("Register Node Address clash\n\r");
				//Have to create a new node address for this node
				//cancel the timers
				result = timer_cancel(&send_reg_req_timer);
				result = timer_cancel(&node_reg_timer);

				get_new_l3_node_address(&address);
				exp_send_addr_reg_req(0xff, (union sigval)(void *)NULL);
			}
		}
#endif
	} else if(msg->can_id == CAN_DCNCP_AddressRegisterReject) {
#if defined(CAN_LAYER_3)
		get_l3_node_address(&address);

		if(msg->data[0] == address) {
			if(status.bit_field.dcncp_status & DCNCP_L3_Address_Finalised) {
				LOG_E("Sending Can Error Message\n\r");
				txMsg.can_id = CAN_DCNCP_NodeAddressError;
				txMsg.can_dlc = 1;
				txMsg.data[0] = address;

				can_l2_tx_frame(&txMsg);
			} else {
				//Have to create a new node address for this node
				//cancel the timers
				timer_cancel(&send_reg_req_timer);
				timer_cancel(&node_reg_timer);

				get_new_l3_node_address(&address);
				exp_send_addr_reg_req(0xff, (union sigval)(void *)NULL);
			}
		}
#endif
	} else if (msg->can_id == CAN_DCNCP_NodeAddressReportReq) {
#if defined(CAN_LAYER_3)
		// Create a random timer between 100 and  1000 miliSeconds for firing node report message
		result = timer_start(MILLI_SECONDS_TO_TICKS((u16) ((rand() % 900) + 100)), exp_send_node_addr_report, (union sigval)(void *)NULL, &timer);
		if (result != SUCCESS) {
			LOG_E("Failed to start Node Registered Timer\n\r");
		}
#endif
	} else if (msg->can_id == CAN_DCNCP_NodeAddressReporting) {
		if(msg->data[0]) {
			LOG_D("Foreign Node Rep Registered Node Address 0x%x\n\r", msg->data[1]);
		} else {
			LOG_D("Foreign Node Rep UN-Registered Node Address 0x%x\n\r", msg->data[1]);
		}
	} else if (msg->can_id == CAN_DCNCP_NodeSetBaudRate) {
		LOG_D(Debug, TAG, "***Baud Rate Change Request New Baud Rate %s\n\r", baud_rate_strings[msg->data[0]]);
//TODO		L2_SetCanNodeBuadRate(msg->data[0]);
	} else if (msg->can_id == CAN_DCNCP_NodePingMessage) {
#if DEBUG_LEVEL <= LOG_DEBUG
		printf(".");
#endif
	} else if (msg->can_id == CAN_DCNCP_NetLogger) {
		LOG_D("Received NetLogger Message\n\r");
#if defined(CAN_LAYER_3)
		net_logger_foreign_register(msg->data[0], msg->data[1]);
#else
		LOG_D("Ignoring NetLogger Message NO LAYER 3!\n\r");
#endif
	} else if (msg->can_id == CAN_DCNCP_CancelNetLogger) {
		LOG_D("Received CancelNetLogger Message\n\r");
#if defined(CAN_LAYER_3)
		net_logger_foreign_cancel(msg->data[0]);
#else
		LOG_D("Ignoring NetLogger Message NO LAYER 3!\n\r");
#endif
	} else {
		LOG_W("Node Unrecognised Request %lx \n\r", msg->can_id);
	}
}

#if defined(CAN_LAYER_3)
void exp_send_node_addr_report(timer_t timer_id __attribute__((unused)), union sigval data)
{
	u8 address;
	can_frame txMsg;

	/*
	 * Clear the compiler warning
	 */
	data = data;

	get_l3_node_address(&address);

	LOG_D("exp_send_node_addr_report(Address %x)\n\r", address);

	txMsg.can_id = CAN_DCNCP_NodeAddressReporting;
	txMsg.can_dlc = 2;
	txMsg.data[1] = address;

	if (status.bit_field.dcncp_status & DCNCP_L3_Address_Finalised)
		txMsg.data[0] = TRUE;
	else
		txMsg.data[0] = FALSE;

	can_l2_tx_frame(&txMsg);
}
#endif

/*
 * Net Logger Stuff
 */
#if defined(CAN_LAYER_3)
result_t register_this_node_net_logger(log_level_t level)
{
	u8 address;

	if(!l3_initialised()) 
		return(ERR_L3_UNINITIALISED);

	get_l3_node_address(&address);

	LOG_D("register_this_node_net_logger()\n\r");

	if(status.bit_field.dcncp_status  & DCNCP_L3_Address_Finalised) {
		local_net_logger_frame.can_id = NetLogger;
		local_net_logger_frame.can_dlc = 2;
		local_net_logger_frame.data[0] = address;
		local_net_logger_frame.data[1] = level;

		can_l2_tx_frame(&local_net_logger_frame);
		LOG_D("NetLogger message sent\n\r");
		timer_start(LOCAL_NET_LOGGER_MSG_PERIOD, exp_net_logger_ping, (union sigval)(void *)NULL, &local_net_logger_timer);

		return(SUCCESS);
	} else {
		LOG_D("NetLogger message not sent Node not Registered yet\n\r");
		return(ERR_GENERAL_L3_ERROR);
	}
}
#endif

#if defined(CAN_LAYER_3)
void exp_net_logger_ping(timer_t timer_id __attribute__((unused)), union sigval data)
{
	data = data;

	can_l2_tx_frame(&local_net_logger_frame);
	LOG_D("NetLogger message sent\n\r");
	timer_start(LOCAL_NET_LOGGER_MSG_PERIOD, exp_net_logger_ping, (union sigval)(void *)NULL, &local_net_logger_timer);
}
#endif

#if defined(CAN_LAYER_3)
result_t unregister_this_node_net_logger()
{
	u8 address;
	can_frame txMsg;

	LOG_D("DeRegAsNetLogger()\n\r");
	get_l3_node_address(&address);

	txMsg.can_id = CAN_DCNCP_CancelNetLogger;
	txMsg.can_dlc = 1;
	txMsg.data[0] = address;

	can_l2_tx_frame(&txMsg);
	LOG_D("CancelNetLogger message sent\n\r");

	if(local_net_logger_timer.status == ACTIVE)
		timer_cancel(&local_net_logger_timer);

	return (SUCCESS);
}
#endif

void send_ping_message(void)
{
	can_frame txMsg;

	txMsg.can_id = CAN_DCNCP_NodePingMessage;
	txMsg.can_dlc = 0;

	can_l2_tx_frame(&txMsg);
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
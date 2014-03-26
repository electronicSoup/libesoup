/**
 *
 * \file es_can/dcncp/dcncp.h
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

#include "es_can/core.h"
#include "system.h"

#if defined(MCP)
//#include "can/l2_can_types.h"
#include "can/es_can.h"
#include "utils/utils.h"
#elif defined(ES_LINUX)
#include "serial.h"
#endif

#include "dcncp/l2_dcncp.h"
#include "timers/timer_sys.h"
#include "can/es_can.h"
#include "logger/net.h"


#if DEBUG_LEVEL < NO_LOGGING
#define TAG "NODE_ADDRESS"
#endif

#define NodeInfo 0x01;

extern void rxNetLogRegReq(u8 address, log_level_t level);
extern void rxNetLogUnRegReq(u8 address);

#if defined(CAN_LAYER_3)
static es_timer sendRegisterReqTimer;
#endif
static es_timer nodeRegisteredTimer;

static u8 l3_node_registered = FALSE;
//static bool this_node_net_logger = FALSE;

static can_frame local_net_logger_frame;
static es_timer local_net_logger_timer;

#define LOCAL_NET_LOGGER_MSG_PERIOD SECONDS_TO_TICKS(5)

static void exp_net_logger_ping(union sigval);


static void exp_sendAddressRegisterReq(union sigval data);
static void exp_nodeAddressRegistered(union sigval data);
static void exp_sendNodeAddressReport(union sigval data);

static void l2MsgHandler(can_frame *msg);

#ifdef TEST
void sendTestMsg(union sigval data);
BYTE otherNode = 0xff;
#endif

void l2_dcncp_init(void)
{
	can_target_t target;
	result_t result;

#if defined(CAN_LAYER_3)
	sendRegisterReqTimer.status = INACTIVE;
#endif
	nodeRegisteredTimer.status = INACTIVE;
	local_net_logger_timer.status = INACTIVE;

	/*
	 * Add the Layer 2 and Layer 3 Can Message Handlers
	 */
	target.mask = (u32)NODE_MASK;
	target.filter = (u32)NODE_FILTER;
	target.handler = l2MsgHandler;

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "Node Address Register handler Mask 0x%x, Filter 0x%x\n\r", target.mask, target.filter);
#endif
	l2_reg_handler(&target);

#if defined(CAN_LAYER_3)
	/*
	 * If we're going to use layer 3 we need to initialise a Layer 3 address to use
	 */
	// Create a random timer between 1 and 5 seconds for firing node register message
	result = start_timer(MILLI_SECONDS_TO_TICKS( (u16)((rand() % 4000) + 1000)), exp_sendAddressRegisterReq, (union sigval)(void *)NULL, &sendRegisterReqTimer);
	if(result != SUCCESS) {
#if DEBUG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Failed to start Register Timer\n\r");
#endif
	}
#endif
}

#if defined(CAN_LAYER_3)
void exp_sendAddressRegisterReq(union sigval data)
{
	u8 address;
	can_frame msg;
	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;

	sendRegisterReqTimer.status = INACTIVE;

	result = get_l3_node_address(&address);

	if(result == SUCCESS) {

#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "sendRegisterReq(%x)\n\r", (u16)address);
#endif

		msg.can_id = AddressRegisterReq;
		msg.can_dlc = 1;
		msg.data[0] = address;

		l2_tx_frame(&msg);

		// Create a 2 Second timer if no reject is recieved in that time
		// this node shall consider itself registered
		result = start_timer(SECONDS_TO_TICKS(2), exp_nodeAddressRegistered, (union sigval)(void *)NULL, &nodeRegisteredTimer);
		if(result != SUCCESS) {
#if DEBUG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "Failed to start Node Registered Timer\n\r");
#endif
		}
	} else {
#if DEBUG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Failed to get the system Layer 3 node address\n\r");
#endif
	}
}
#endif

#if defined(CAN_LAYER_3)
void exp_nodeAddressRegistered(union sigval data)
{
	u8 address;
	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;
	nodeRegisteredTimer.status = INACTIVE;

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "nodeRegistered()\n\r");
#endif

	l3_node_registered = TRUE;

	result = get_l3_node_address(&address);

	if(result == SUCCESS) {
		l3_finalise_address(address);

#ifdef TEST
		result = start_timer(SECONDS_TO_TICKS(1), sendTestMsg, (union sigval)(void *)NULL, &sendRegisterReqTimer);
		if(result != SUCCESS) {
#if DEBUG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "Failed to start Send Register Request Timer\n\r");
#endif
		}
#endif
	} else {
#if DEBUG_LEVEL <= LOG_ERROR
		serial_log(Error, TAG, "Failed to get the system Layer 3 node address\n\r");
#endif
	}
}
#endif

void l2MsgHandler(can_frame *msg)
{
	u8 address;
	can_frame txMsg;
	es_timer timer;
	result_t result;

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "Node Adress message received 0x%lx\n\r", msg->can_id);
#endif
	if (msg->can_id == AddressRegisterReq) {
#if defined(CAN_LAYER_3)
		get_l3_node_address(&address);

		if(msg->data[0] == address) {
			if(l3_node_registered){
#if DEBUG_LEVEL <= LOG_DEBUG
				serial_log(Debug, TAG, "reject Register Request\n\r");
#endif
				txMsg.can_id = AddressRegisterReject;
				txMsg.can_dlc = 1;
				txMsg.data[0] = address;

				l2_tx_frame(&txMsg);
			} else {
#if DEBUG_LEVEL <= LOG_DEBUG
				serial_log(Debug, TAG, "Register Node Address clash\n\r");
#endif
				//Have to create a new node address for this node
				//cancel the timers
				result = cancel_timer(&sendRegisterReqTimer);
				result = cancel_timer(&nodeRegisteredTimer);

				get_new_l3_node_address(&address);
				exp_sendAddressRegisterReq((union sigval)(void *)NULL);
			}
		}
#endif
	} else if(msg->can_id == AddressRegisterReject) {
#if defined(CAN_LAYER_3)
		get_l3_node_address(&address);

		if(msg->data[0] == address) {
			if(l3_node_registered) {
#if DEBUG_LEVEL <= LOG_ERROR
				serial_log(Error, TAG, "Sending Can Error Message\n\r");
#endif
				txMsg.can_id = NodeAddressError;
				txMsg.can_dlc = 1;
				txMsg.data[0] = address;

				l2_tx_frame(&txMsg);
			} else {
				//Have to create a new node address for this node
				//cancel the timers
				cancel_timer(&sendRegisterReqTimer);
				cancel_timer(&nodeRegisteredTimer);

				get_new_l3_node_address(&address);
				exp_sendAddressRegisterReq((union sigval)(void *)NULL);
			}
		}
#endif
	} else if (msg->can_id == NodeAddressReportReq) {
#if defined(CAN_LAYER_3)
		// Create a random timer between 100 and  1000 miliSeconds for firing node report message
		result = start_timer(MILLI_SECONDS_TO_TICKS((u16) ((rand() % 900) + 100)), exp_sendNodeAddressReport, (union sigval)(void *)NULL, &timer);
		if (result != SUCCESS) {
#if DEBUG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "Failed to start Node Registered Timer\n\r");
#endif
		}
#endif
	} else if (msg->can_id == NodeAddressReporting) {
#if DEBUG_LEVEL <= LOG_DEBUG
		if(msg->data[0]) {
			serial_log(Debug, TAG, "Foreign Node Rep Registered Node Address 0x%x\n\r", msg->data[1]);
		} else {
			serial_log(Debug, TAG, "Foreign Node Rep UN-Registered Node Address 0x%x\n\r", msg->data[1]);
		}
#endif
	} else if (msg->can_id == NodeSetBaudRate) {
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "***Baud Rate Change Request\n\r");
		printf("New Baud Rate %s\n\r", baud_rate_strings[msg->data[0]]);
#endif
//TODO		L2_SetCanNodeBuadRate(msg->data[0]);
	} else if (msg->can_id == NodePingMessage) {
#if DEBUG_LEVEL <= LOG_DEBUG
		printf(".");
//            serial_log(Debug, TAG, "Node Ping Message Received\n\r");
#endif
	} else if (msg->can_id == NetLogger) {
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Received NetLogger Message\n\r");
#endif
#if defined(CAN_LAYER_3)
		net_logger_foreign_register(msg->data[0], msg->data[1]);
#else
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Ignoring NetLogger Message NO LAYER 3!\n\r");
#endif
#endif
	} else if (msg->can_id == CancelNetLogger) {
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Received CancelNetLogger Message\n\r");
#endif
#if defined(CAN_LAYER_3)
		net_logger_foreign_cancel(msg->data[0]);
#else
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "Ignoring NetLogger Message NO LAYER 3!\n\r");
#endif
#endif
	} else {
#if DEBUG_LEVEL <= LOG_WARNING
		serial_log(Warning, TAG, "Node Unrecognised Request %lx \n\r", msg->can_id);
#endif
	}
}

#if defined(CAN_LAYER_3)
void exp_sendNodeAddressReport(union sigval data)
{
	u8 address;
	can_frame txMsg;

	/*
	 * Clear the compiler warning
	 */
	data = data;

	get_l3_node_address(&address);

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "exp_sendNodeAddressReport(Address %x)\n\r", address);
#endif

	txMsg.can_id = NodeAddressReporting;
	txMsg.can_dlc = 2;
	txMsg.data[1] = address;

	if (l3_node_registered)
		txMsg.data[0] = TRUE;
	else
		txMsg.data[0] = FALSE;

	l2_tx_frame(&txMsg);
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

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "register_this_node_net_logger()\n\r");
#endif

	if(l3_node_registered) {
		local_net_logger_frame.can_id = NetLogger;
		local_net_logger_frame.can_dlc = 2;
		local_net_logger_frame.data[0] = address;
		local_net_logger_frame.data[1] = level;

		l2_tx_frame(&local_net_logger_frame);
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "NetLogger message sent\n\r");
#endif
		start_timer(LOCAL_NET_LOGGER_MSG_PERIOD, exp_net_logger_ping, (union sigval)(void *)NULL, &local_net_logger_timer);

		return(SUCCESS);
	} else {
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "NetLogger message not sent Node not Registered yet\n\r");
#endif
		return(ERR_GENERAL_L3_ERROR);
	}
}
#endif

#if defined(CAN_LAYER_3)
void exp_net_logger_ping(union sigval data)
{
	data = data;

	l2_tx_frame(&local_net_logger_frame);
#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "NetLogger message sent\n\r");
#endif
	start_timer(LOCAL_NET_LOGGER_MSG_PERIOD, exp_net_logger_ping, (union sigval)(void *)NULL, &local_net_logger_timer);
}
#endif

#if defined(CAN_LAYER_3)
result_t unregister_this_node_net_logger()
{
	u8 address;
	can_frame txMsg;

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "DeRegAsNetLogger()\n\r");
#endif

	get_l3_node_address(&address);

	txMsg.can_id = CancelNetLogger;
	txMsg.can_dlc = 1;
	txMsg.data[0] = address;

	l2_tx_frame(&txMsg);
#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "CancelNetLogger message sent\n\r");
#endif

	if(local_net_logger_timer.status == ACTIVE)
		cancel_timer(&local_net_logger_timer);

	return (SUCCESS);
}
#endif

void send_ping_message(void)
{
	can_frame txMsg;

	txMsg.can_id = NodePingMessage;
	txMsg.can_dlc = 0;

	l2_tx_frame(&txMsg);
#if DEBUG_LEVEL <= LOG_DEBUG
//    serial_log(Debug, TAG, "Ping message sent\n\r");
#endif
}

#if defined(CAN_LAYER_3)
#ifdef TEST
void sendTestMsg(union sigval data __attribute__((unused)))
{
#if 1
	//L3 Message
	u8 buffer[70];
	u8 loop;
	result_t result;

	static u8 sizeToSend = 13;
	l3_can_msg_t msg;

	for(loop = 0; loop < 70; loop++)
		buffer[loop] = loop + 1;

#if DEBUG_LEVEL <= LOG_DEBUG
	serial_log(Debug, TAG, "\n\r\n\r*** sendTestL3Msg() size %d\n\r", sizeToSend);
#endif

	if(otherNode != 0xff) {
		msg.address = otherNode;
		msg.data = buffer;
		msg.size = sizeToSend;
		msg.protocol = 10;

		l3_tx_msg(&msg);
		sizeToSend++;
		result = start_timer(SECONDS_TO_TICKS(10), sendTestMsg, (union sigval)(void *)NULL, &sendRegisterReqTimer);
		if (result != SUCCESS) {
#if DEBUG_LEVEL <= LOG_ERROR
			serial_log(Error, TAG, "Failed to start Send Register Request Timer\n\r");
#endif
		}
	} else {
#if DEBUG_LEVEL <= LOG_DEBUG
		serial_log(Debug, TAG, "No Other Node\n\r");
#endif
	}
#else
	debug("Send a node Report L1 Message\n\r");
	nodeSendReport(null);
	start_timer(SECONDS_TO_TICKS(1), sendTestMsg, (u8 *) null);
#endif
}
#endif
#endif
/*
 */

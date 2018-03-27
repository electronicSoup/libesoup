/**
 *
 * \file libesoup/can/dcncp/dcncp.c
 *
 * Dynamic SYS_CAN Node Configuration Protocol 
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
 */
#include "libesoup_config.h"

#ifdef SYS_CAN_DCNCP

#include <stdlib.h>
#include <string.h>
//#include <stdio.h>

#if (defined(SYS_SERIAL_LOGGING) && !defined(SYS_LOG_LEVEL))
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "DCNCP_CAN";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/errno.h"
#include "libesoup/comms/can/can.h"
#include "libesoup/comms/can/dcncp/dcncp_can.h"
#include "libesoup/timers/sw_timers.h"
#if defined(ISO15765_LOGGER) || defined(SYS_ISO15765_LOGGING)
#include "libesoup/logger/iso15765_log.h"
#endif

#ifdef SYS_CAN_DCNCP_BAUDRATE
static es_timer dcncp_network_baudrate_req_timer;
static void exp_resend_network_baud_chage_req(timer_t timer_id, union sigval data);
static void exp_network_baud_chage_req(timer_t timer_id, union sigval data);
#endif // SYS_CAN_DCNCP_BAUDRATE

#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)

static boolean dcncp_node_address_valid = FALSE;

/*
 * This function must be defined by your Firmware!
 */
extern uint8_t node_get_address(void);

static timer_id node_send_reg_req_timer;
static timer_id node_reg_timer;

#ifdef SYS_ISO15765_LOGGER
static can_frame local_iso15765_logger_frame;
static es_timer  local_iso15765_logger_timer;
#endif // SYS_ISO15765_LOGGER

static uint8_t dcncp_node_address;
uint8_t dcncp_get_node_address(void);

#ifdef SYS_ISO15765_LOGGER
static void exp_iso15765_logger_ping(timer_t timer_id __attribute__((unused)), union sigval);
#endif // SYS_ISO15765_LOGGER

static void exp_send_address_register_request(timer_id timer, union sigval data);
static void exp_node_address_registered(timer_id timer __attribute__((unused)), union sigval data);
static void exp_send_node_addr_report(timer_id timer, union sigval data);

#endif  // #if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)

static void can_l2_msg_handler(can_frame *msg);

static union ty_status  status;

static status_handler_t  status_handler;

result_t dcncp_init(status_handler_t arg_status_handler)
{
	result_t          rc;
	can_l2_target_t   target;
	struct timer_req  request;

	dcncp_node_address_valid = FALSE;
	
        status_handler = arg_status_handler;
	status.sstruct.source = can_bus_dcncp_status;

	/*
	 * Add the Layer 2 frame Handler
	 */
	target.mask    = (uint32_t)CAN_DCNCP_MASK;
	target.filter  = (uint32_t)CAN_DCNCP_FILTER;
	target.handler = can_l2_msg_handler;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
#if defined(XC16) || defined(__XC8)
	LOG_D("Node Address Register handler Mask 0x%lx, Filter 0x%lx\n\r", target.mask, target.filter);
#elif defined(ES_LINUX)
	LOG_D("Node Address Register handler Mask 0x%x, Filter 0x%x\n\r", target.mask, target.filter);
#endif
#endif
	rc = frame_dispatch_reg_handler(&target);
	RC_CHECK

#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
	/*
	 * If we're going to use layer 3 we need to initialise a Layer 3 address to use
	 * Create a random timer for firing node register message. If all network nodes
	 * powerup at the same time we don't want to flood the network with register
	 * requests so we'll hold off. Random time between 1 and 5 seconds.
	 */
	// struct timer_req  request;
	request.units          = mSeconds;
	request.duration       = (uint16_t)((rand() % 4000) + 1000);
	request.type           = single_shot;
	request.data.sival_int = 0;
	request.exp_fn         = exp_send_address_register_request;

	rc = sw_timer_start(&request);
	RC_CHECK;
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)

	return(0);
}

#ifdef SYS_CAN_DCNCP_BAUDRATE
void dcncp_request_network_baud_change(can_baud_rate_t baud)
{
	result_t     rc;
	can_frame    msg;
	union sigval data;

#if (defined(SYS_SERIAL_LOGGING) defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("dcncp_request_network_baud_change()\n\r");
#endif

        if(!status.bit_field.dcncp_initialised) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		LOG_W("Ignoring DCNCP not ready!\n\r");
#endif
		return;
	}

	if(baud == can_l2_get_baudrate()) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		LOG_W("Ignoring spurious request!\n\r");
#endif
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to start BaudRate change Request Timer\n\r");
#endif
		return;
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("send network baudrate change request\n\r");
#endif

	msg.can_id = CAN_DCNCP_NetworkChangeBaudRateReq;
	msg.can_dlc = 2;
	msg.data[0] = baud;
	msg.data[1] = 3;      // 3 Seconds till Change

	can_l2_tx_frame(&msg);
}
#endif // SYS_CAN_DCNCP_BAUDRATE

#ifdef SYS_CAN_DCNCP_BAUDRATE
static void exp_resend_network_baud_chage_req(timer_t timer_id, union sigval data)
{
	can_frame msg;
	result_t rc;
	uint8_t time_left;
	can_baud_rate_t baud;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("exp_resend_network_baud_chage_req()\n\r");
#endif
	/*
	 * Resend a Layer 2 message to inform network of impending change!
	 */
	TIMER_INIT(dcncp_network_baudrate_req_timer);

	time_left = data.sival_int & 0xff;
	baud = (data.sival_int >> 8) & 0xff;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Time left %d\n\r", time_left);
#endif

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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Failed to start BaudRate change Request Timer\n\r");
#endif
			return;
		}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("send network baudrate change request\n\r");
#endif

		msg.can_id = CAN_DCNCP_NetworkChangeBaudRateReq;
		msg.can_dlc = 2;
		msg.data[0] = baud;
		msg.data[1] = time_left;

		can_l2_tx_frame(&msg);
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("TIME UP Change Baud Rate\n\r");
#endif
		can_l2_set_node_baudrate(baud);
	}
}
#endif // SYS_CAN_DCNCP_BAUDRATE

#ifdef SYS_CAN_DCNCP_BAUDRATE
static void exp_network_baud_chage_req(timer_t timer_id, union sigval data)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("!!! exp_network_baud_chage_req() !!!\n\r");
#endif
	can_l2_set_node_baudrate((can_baud_rate_t)(data.sival_int & 0xff));
}
#endif // SYS_CAN_DCNCP_BAUDRATE

#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
uint8_t dcncp_get_node_address(void)
{
	if(dcncp_node_address_valid) {
		return (dcncp_node_address);
	} else {
		return(BROADCAST_NODE_ADDRESS);
	}
}
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)

#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
void exp_send_address_register_request(timer_id timer, union sigval data)
{
	struct timer_req  request;
	can_frame         frame;
	result_t          rc;

	/*
	 * Clear the compiler warning
	 */
	timer = timer;
	data = data;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("exp_send_address_register_request() Send Initial Register Req\n\r");
#endif

	dcncp_node_address = node_get_address();

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("sendRegisterReq(%x)\n\r", (uint16_t) dcncp_node_address);
#endif

	frame.can_id   = CAN_DCNCP_AddressRegisterReq;
	frame.can_dlc  = 1;
	frame.data[0]  = dcncp_node_address;

	rc = can_l2_tx_frame(&frame);
	RC_CHECK_PRINT_VOID("CAN Tx\n\r");

	/*
	 *Create a 2 Second timer if no reject is recieved in that time
	 * this node shall consider itself registered
	 */
	// struct timer_req  request;
	request.units          = Seconds;
	request.duration       = 2;
	request.type           = single_shot;
	request.data.sival_int = 0;
	request.exp_fn         = exp_node_address_registered;

	rc = sw_timer_start(&request);
	RC_CHECK_PRINT_VOID("SW Timer Start\n\r");
	node_reg_timer = rc;
}
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)

#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
void exp_node_address_registered(timer_id timer __attribute__((unused)), union sigval data)
{
//	uint8_t   address;

	/*
	 * Clear the compiler warning
	 */
	data = data;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("nodeRegistered()\n\r");
#endif
	dcncp_node_address_valid = TRUE;

        if(status_handler) {
		status.sstruct.source = can_bus_dcncp_status;
		status.sstruct.status = can_dcncp_l3_address_registered;
		status_handler(status);
	}
}
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)

void can_l2_msg_handler(can_frame *frame)
{
	result_t rc;
	struct timer_req  request;

#ifdef SYS_CAN_DCNCP_BAUDRATE
	union sigval data;
#endif 
#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
	can_frame tx_frame;
#endif // SYS_ISO15765 || SYS_ISO11783  || defined(SYS_TEST_L3_ADDRESS)

	if (frame->can_id == CAN_DCNCP_AddressRegisterReq) {
#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
		if(frame->data[0] == dcncp_node_address) {
			if(dcncp_node_address_valid) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("reject Register Request\n\r");
#endif
				tx_frame.can_id = CAN_DCNCP_AddressRegisterReject;
				tx_frame.can_dlc = 1;
				tx_frame.data[0] = dcncp_node_address;

				rc = can_l2_tx_frame(&tx_frame);
				RC_CHECK_PRINT_VOID("CAN_L2_TX\n\r");
			} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("Register Node Address clash. Get New Node Address!\n\r");
#endif
				/*
				 *Have to create a new node address for this node
				 *cancel the timers
				 */
				rc = sw_timer_cancel(node_send_reg_req_timer);
				RC_CHECK_PRINT_VOID("SW_Tm_CANCEL\n\r");
				rc = sw_timer_cancel(node_reg_timer);
				RC_CHECK_PRINT_VOID("SW_Tm_CANCEL\n\r");

				dcncp_node_address = node_get_address();

				exp_send_address_register_request((timer_id)0xff, (union sigval)(void *)NULL);
			}
		}
#endif  // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)
	} else if(frame->can_id == CAN_DCNCP_AddressRegisterReject) {
#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
		if(frame->data[0] == dcncp_node_address) {
			if(dcncp_node_address_valid) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
				LOG_E("Sending Can Error Message\n\r");
#endif
				tx_frame.can_id = CAN_DCNCP_NodeAddressError;
				tx_frame.can_dlc = 1;
				tx_frame.data[0] = dcncp_node_address;

				can_l2_tx_frame(&tx_frame);
			} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("Address Regected so get a new one!\n\r");
#endif
				/*
				 *Have to create a new node address for this node
				 *cancel the timers
				 */
				rc = sw_timer_cancel(node_send_reg_req_timer);
				RC_CHECK_PRINT_VOID("Timer Cancel\n\r");
				rc = sw_timer_cancel(node_reg_timer);
				RC_CHECK_PRINT_VOID("Timer Cancel\n\r");

				dcncp_node_address = node_get_address();

				exp_send_address_register_request((timer_id)0xff, (union sigval)(void *)NULL);
			}
		}
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)
	} else if (frame->can_id == CAN_DCNCP_NodeAddressReportReq) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("DCNCP_CAN_NodeAddressReportReq:\n\r");
#endif
#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
		// Create a random timer between 100 and  1000 miliSeconds for firing node report message

//		rc = timer_start(MILLI_SECONDS_TO_TICKS((uint16_t) ((rand() % 900) + 100)), exp_send_node_addr_report, (union sigval)(void *)NULL, &timer);
		// struct timer_req  request;
		request.units          = mSeconds;
		request.duration       = ((rand() % 900) + 100);
		request.type           = single_shot;
		request.data.sival_int = 0;
		request.exp_fn         = exp_send_node_addr_report;
		rc = sw_timer_start(&request);
		RC_CHECK_PRINT_VOID("SW Timer Start\n\r");
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)
	} else if (frame->can_id == CAN_DCNCP_NodeAddressReporting) {
		if(frame->data[0]) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Foreign Node Rep Registered Node Address 0x%x\n\r", frame->data[1]);
#endif
		} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Foreign Node Rep UN-Registered Node Address 0x%x\n\r", frame->data[1]);
#endif
		}
	} else if (frame->can_id == CAN_DCNCP_NetworkChangeBaudRateReq) {
#ifdef SYS_CAN_DCNCP_BAUDRATE
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("***Baud Rate Change Request New Baud Rate %s Time left %dS\n\r", can_baud_rate_strings[frame->data[0]], frame->data[1]);
#endif

		timer_cancel(&dcncp_network_baudrate_req_timer);
		TIMER_INIT(dcncp_network_baudrate_req_timer);

		data.sival_int = frame->data[0];   // Baudrate

		/*
		 * Create a timer to for time given in message.
		 */
		rc = timer_start(SECONDS_TO_TICKS(frame->data[1]), exp_network_baud_chage_req, data, &dcncp_network_baudrate_req_timer);
		if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Failed to start BaudRate change Request Timer\n\r");
#endif
			return;
		}
#else
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("CAN Baudrate change! NO code in place to process request!\n\r");
#endif
#endif // SYS_CAN_DCNCP_BAUDRATE
	} else if (frame->can_id == CAN_DCNCP_NodePingMessage) {
	} else if (frame->can_id == CAN_DCNCP_RegisterNetLogger) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Received NetLogger Registration Message\n\r");
#endif
#ifdef SYS_ISO15765_LOGGING
		iso15765_logger_register_remote(frame->data[0], frame->data[1]);
#endif // SYS_ISO15765_LOGGING
	} else if (frame->can_id == CAN_DCNCP_UnRegisterNetLogger) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Received NetLogger UnRegistration Message\n\r");
#endif
#ifdef SYS_ISO15765_LOGGING
		iso15765_logger_unregister_remote(frame->data[0]);
#endif // SYS_ISO15765_LOGGING
	} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_WARNING))
#if defined(XC16) || defined(__XC8)
		LOG_W("Node Unrecognised Request %lx \n\r", frame->can_id);
#elif defined(ES_LINUX)
		LOG_W("Node Unrecognised Request %x \n\r", frame->can_id);
#endif
#endif
	}
}

#if defined(ISO15765) || defined(ISO11783) || defined(SYS_TEST_L3_ADDRESS)
void exp_send_node_addr_report(timer_id timer __attribute__((unused)), union sigval data)
{
	can_frame frame;

	/*
	 * Clear the compiler warning
	 */
	data = data;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("exp_send_node_addr_report(Address %x)\n\r", dcncp_node_address);
#endif

	frame.can_id = CAN_DCNCP_NodeAddressReporting;
	frame.can_dlc = 2;
	frame.data[1] = dcncp_node_address;

	if (dcncp_node_address_valid)
		frame.data[0] = TRUE;
	else
		frame.data[0] = FALSE;

	can_l2_tx_frame(&frame);
}
#endif // SYS_ISO15765 || SYS_ISO11783 || defined(SYS_TEST_L3_ADDRESS)

/*
 * Net Logger Stuff
 */
#if defined(ISO15765_LOGGER)
result_t dcncp_register_this_node_net_logger(log_level_t level)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("register_this_node_net_logger()\n\r");
#endif

	if(!iso15765_initialised())
		return(ERR_NOT_READY);

	local_iso15765_logger_frame.can_id = CAN_DCNCP_RegisterNetLogger;
	local_iso15765_logger_frame.can_dlc = 2;
	local_iso15765_logger_frame.data[0] = dcncp_node_address;
	local_iso15765_logger_frame.data[1] = level;

	can_l2_tx_frame(&local_iso15765_logger_frame);
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("NetLogger message sent\n\r");
#endif
	TIMER_INIT(local_iso15765_logger_timer);
	timer_start(SECONDS_TO_TICKS(SYS_ISO15765_LOGGER_PING_PERIOD), 
                    exp_iso15765_logger_ping, 
                    (union sigval)(void *) NULL, 
                    &local_iso15765_logger_timer);

	return (SUCCESS);
}
#endif // SYS_ISO15765_LOGGER

#if defined(ISO15765_LOGGER)
void exp_iso15765_logger_ping(timer_t timer_id __attribute__((unused)), union sigval data)
{
	data = data;

	can_l2_tx_frame(&local_iso15765_logger_frame);
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("NetLogger message sent\n\r");
#endif
	TIMER_INIT(local_iso15765_logger_timer);
	timer_start(SECONDS_TO_TICKS(SYS_ISO15765_LOGGER_PING_PERIOD),
                    exp_iso15765_logger_ping, 
                    (union sigval)(void *)NULL, 
                    &local_iso15765_logger_timer);
}
#endif // SYS_ISO15765_LOGGER

#if defined(ISO15765_LOGGER)
result_t dcncp_unregister_this_node_net_logger()
{
	can_frame frame;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("DeRegAsNetLogger()\n\r");
#endif

	frame.can_id = CAN_DCNCP_UnRegisterNetLogger;
	frame.can_dlc = 1;
	frame.data[0] = dcncp_node_address;

	can_l2_tx_frame(&frame);
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("CancelNetLogger message sent\n\r");
#endif

	if(local_iso15765_logger_timer.status == ACTIVE)
		timer_cancel(&local_iso15765_logger_timer);

	return (SUCCESS);
}
#endif // SYS_ISO15765_LOGGER

void dcncp_send_ping(void)
{
	can_frame frame;

	frame.can_id = CAN_DCNCP_NodePingMessage;
	frame.can_dlc = 0;

	can_l2_tx_frame(&frame);

#ifdef SYS_CAN_L2_PING_LOGGING
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Ping message sent\n\r");
#endif
#endif // SYS_CAN_L2_PING_LOGGING
}

#endif // SYS_CAN_DCNCP

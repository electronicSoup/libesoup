/**
 *
 * \file libesoup/can/dcncp/dcncp.c
 *
 * Dynamic SYS_CAN Node Configuration Protocol 
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "libesoup_config.h"

#define DEBUG_FILE TRUE
#include "libesoup/logger/serial_log.h"
#include "libesoup/can/es_can.h"

#include "libesoup/can/dcncp/dcncp_can.h"
#include "libesoup/timers/sw_timers.h"
#if defined(ISO15765_LOGGER) || defined(SYS_ISO15765_LOGGING)
#include "libesoup/logger/iso15765_log.h"
#endif

#define TAG "DCNCP_CAN"

#ifdef SYS_CAN_DCNCP_BAUDRATE
static es_timer dcncp_network_baudrate_req_timer;
static void exp_resend_network_baud_chage_req(timer_t timer_id, union sigval data);
static void exp_network_baud_chage_req(timer_t timer_id, union sigval data);
#endif // SYS_CAN_DCNCP_BAUDRATE

#if defined(ISO15765) || defined(ISO11783)
/*
 * This function must be defined by your Firmware!
 */
extern uint8_t node_get_address(void);

static es_timer node_send_reg_req_timer;
static es_timer node_reg_timer;

#ifdef SYS_ISO15765_LOGGER
static can_frame local_iso15765_logger_frame;
static es_timer  local_iso15765_logger_timer;
#endif // SYS_ISO15765_LOGGER

static uint8_t dcncp_node_address;
uint8_t dcncp_get_node_address(void);

#ifdef SYS_ISO15765_LOGGER
static void exp_iso15765_logger_ping(timer_t timer_id __attribute__((unused)), union sigval);
#endif // SYS_ISO15765_LOGGER

static void exp_send_address_register_request(timer_t timer_id, union sigval data);
static void exp_node_address_registered(timer_t timer_id __attribute__((unused)), union sigval data);
static void exp_send_node_addr_report(timer_t timer_id, union sigval data);

#endif  // SYS_ISO15765 || SYS_ISO11783

static void can_l2_msg_handler(can_frame *msg);

static can_status_t status;
static void (*status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud) = NULL;

void dcncp_init(void (*arg_status_handler)(uint8_t mask, can_status_t status, can_baud_rate_t baud))
{
	can_l2_target_t target;
#if defined(ISO15765) || defined(ISO11783)
	result_t rc;
#endif // SYS_ISO15765 || SYS_ISO11783

        status_handler = arg_status_handler;
        status.byte = 0x00;

#ifdef SYS_CAN_DCNCP_BAUDRATE
	TIMER_INIT(dcncp_network_baudrate_req_timer);
#endif // SYS_CAN_DCNCP_BAUDRATE

#if defined(ISO15765) || defined(ISO11783)
	TIMER_INIT(node_send_reg_req_timer);
	TIMER_INIT(node_reg_timer);
#endif // SYS_ISO15765 || SYS_ISO11783

#ifdef SYS_ISO15765_LOGGER
	local_iso15765_logger_timer.status = INACTIVE;
	TIMER_INIT(local_iso15765_logger_timer);
#endif // SYS_ISO15765_LOGGER

	/*
	 * Add the Layer 2 frame Handler
	 */
	target.mask    = (uint32_t)DCNCP_CAN_MASK;
	target.filter  = (uint32_t)DCNCP_CAN_FILTER;
	target.handler = can_l2_msg_handler;
#ifdef MCP
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Node Address Register handler Mask 0x%lx, Filter 0x%lx\n\r", target.mask, target.filter);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#elif defined(ES_LINUX)
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Node Address Register handler Mask 0x%x, Filter 0x%x\n\r", target.mask, target.filter);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#endif
	can_l2_dispatch_reg_handler(&target);

#if defined(ISO15765) || defined(ISO11783)
	/*
	 * If we're going to use layer 3 we need to initialise a Layer 3 address to use
	 * Create a random timer for firing node register message. If all network nodes
	 * powerup at the same time we don't want to flood the network with register
	 * requests so we'll hold off. Random time between 1 and 5 seconds.
	 */
	rc = timer_start(MILLI_SECONDS_TO_TICKS( (uint16_t)((rand() % 4000) + 1000)),
		         exp_send_address_register_request,
			 (union sigval)(void *) NULL,
			 &node_send_reg_req_timer);
	if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to start Register Timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
	}
#endif // SYS_ISO15765 || SYS_ISO11783
        status.bit_field.dcncp_initialised = 1;

        if(status_handler)
		status_handler(DCNCP_INIT_STATUS_MASK, status, no_baud);
}

#ifdef SYS_CAN_DCNCP_BAUDRATE
void dcncp_request_network_baud_change(can_baud_rate_t baud)
{
	result_t     rc;
	can_frame    msg;
	union sigval data;

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "dcncp_request_network_baud_change()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

        if(!status.bit_field.dcncp_initialised) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		log_w(TAG, "Ignoring DCNCP not ready!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		return;
	}

	if(baud == can_l2_get_baudrate()) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		log_w(TAG, "Ignoring spurious request!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to start BaudRate change Request Timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		return;
	}

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "send network baudrate change request\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

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

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "exp_resend_network_baud_chage_req()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
	/*
	 * Resend a Layer 2 message to inform network of impending change!
	 */
	TIMER_INIT(dcncp_network_baudrate_req_timer);

	time_left = data.sival_int & 0xff;
	baud = (data.sival_int >> 8) & 0xff;

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Time left %d\n\r", time_left);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

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
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Failed to start BaudRate change Request Timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			return;
		}

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "send network baudrate change request\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

		msg.can_id = CAN_DCNCP_NetworkChangeBaudRateReq;
		msg.can_dlc = 2;
		msg.data[0] = baud;
		msg.data[1] = time_left;

		can_l2_tx_frame(&msg);
	} else {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "TIME UP Change Baud Rate\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		can_l2_set_node_baudrate(baud);
	}
}
#endif // SYS_CAN_DCNCP_BAUDRATE

#ifdef SYS_CAN_DCNCP_BAUDRATE
static void exp_network_baud_chage_req(timer_t timer_id, union sigval data)
{
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "!!! exp_network_baud_chage_req() !!!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
	can_l2_set_node_baudrate((can_baud_rate_t)(data.sival_int & 0xff));
}
#endif // SYS_CAN_DCNCP_BAUDRATE

#if defined(ISO15765) || defined(ISO11783)
uint8_t dcncp_get_node_address(void)
{
	if(status.bit_field.dcncp_node_address_valid) {
		return (dcncp_node_address);
	} else {
		return(BROADCAST_NODE_ADDRESS);
	}
}
#endif // SYS_ISO15765 || SYS_ISO11783

#if defined(ISO15765) || defined(ISO11783)
void exp_send_address_register_request(timer_t timer_id, union sigval data)
{
	can_frame msg;
	result_t result;

	/*
	 * Clear the compiler warning
	 */
	timer_id = timer_id;
	data = data;

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "exp_send_address_register_request() Send Initial Register Req\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
	TIMER_INIT(node_send_reg_req_timer);

	dcncp_node_address = node_get_address();

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "sendRegisterReq(%x)\n\r", (uint16_t) dcncp_node_address);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

	msg.can_id = CAN_DCNCP_AddressRegisterReq;
	msg.can_dlc = 1;
	msg.data[0] = dcncp_node_address;

	can_l2_tx_frame(&msg);

	/*
	 *Create a 2 Second timer if no reject is recieved in that time
	 * this node shall consider itself registered
	 */
	result = timer_start(SECONDS_TO_TICKS(2), 
	                     exp_node_address_registered,
			     (union sigval)(void *) NULL,
			     &node_reg_timer);
	if (result != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to start Node Registered Timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
	}
}
#endif // SYS_ISO15765 || SYS_ISO11783

#if defined(ISO15765) || defined(ISO11783)
void exp_node_address_registered(timer_t timer_id __attribute__((unused)), union sigval data)
{
//	uint8_t address;
//	result_t result;

	/*
	 * Clear the compiler warning
	 */
	data = data;
	TIMER_INIT(node_reg_timer);

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "nodeRegistered()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

        status.bit_field.dcncp_node_address_valid = 1;

        if(status_handler) {
		status_handler(DCNCP_NODE_ADDRESS_STATUS_MASK, status, no_baud);
	}
}
#endif // SYS_ISO15765 || SYS_ISO11783

void can_l2_msg_handler(can_frame *frame)
{
#if defined(ISO15765) || defined(ISO11783) || defined(DCNCP_CAN_BAUDRATE)
	result_t rc;
#endif // defined(ISO15765) || defined(ISO11783) || defined(DCNCP_CAN_BAUDRATE)


#ifdef SYS_CAN_DCNCP_BAUDRATE
	union sigval data;
#endif 
#if defined(ISO15765) || defined(ISO11783)
	can_frame tx_frame;
	es_timer timer;
#endif // SYS_ISO15765 || SYS_ISO11783

	if (frame->can_id == CAN_DCNCP_AddressRegisterReq) {
#if defined(ISO15765) || defined(ISO11783)
		if(frame->data[0] == dcncp_node_address) {
			if(status.bit_field.dcncp_node_address_valid) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "reject Register Request\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				tx_frame.can_id = CAN_DCNCP_AddressRegisterReject;
				tx_frame.can_dlc = 1;
				tx_frame.data[0] = dcncp_node_address;

				can_l2_tx_frame(&tx_frame);
			} else {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "Register Node Address clash. Get New Node Address!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				/*
				 *Have to create a new node address for this node
				 *cancel the timers
				 */
				rc = timer_cancel(&node_send_reg_req_timer);
				rc = timer_cancel(&node_reg_timer);

				dcncp_node_address = node_get_address();

				exp_send_address_register_request((timer_t)0xff, (union sigval)(void *)NULL);
			}
		}
#endif  // SYS_ISO15765 || SYS_ISO11783
	} else if(frame->can_id == CAN_DCNCP_AddressRegisterReject) {
#if defined(ISO15765) || defined(ISO11783)
		if(frame->data[0] == dcncp_node_address) {
			if(status.bit_field.dcncp_node_address_valid) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Sending Can Error Message\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				tx_frame.can_id = CAN_DCNCP_NodeAddressError;
				tx_frame.can_dlc = 1;
				tx_frame.data[0] = dcncp_node_address;

				can_l2_tx_frame(&tx_frame);
			} else {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "Address Regected so get a new one!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				/*
				 *Have to create a new node address for this node
				 *cancel the timers
				 */
				rc = timer_cancel(&node_send_reg_req_timer);
				rc = timer_cancel(&node_reg_timer);

				dcncp_node_address = node_get_address();

				exp_send_address_register_request((timer_t)0xff, (union sigval)(void *)NULL);
			}
		}
#endif // SYS_ISO15765 || SYS_ISO11783
	} else if (frame->can_id == CAN_DCNCP_NodeAddressReportReq) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "DCNCP_CAN_NodeAddressReportReq:\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#if defined(ISO15765) || defined(ISO11783)
		// Create a random timer between 100 and  1000 miliSeconds for firing node report message
		TIMER_INIT(timer);
		rc = timer_start(MILLI_SECONDS_TO_TICKS((uint16_t) ((rand() % 900) + 100)), exp_send_node_addr_report, (union sigval)(void *)NULL, &timer);
		if (rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Failed to start Node Registered Timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		}
#endif // SYS_ISO15765 || SYS_ISO11783
	} else if (frame->can_id == CAN_DCNCP_NodeAddressReporting) {
		if(frame->data[0]) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Foreign Node Rep Registered Node Address 0x%x\n\r", frame->data[1]);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		} else {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Foreign Node Rep UN-Registered Node Address 0x%x\n\r", frame->data[1]);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		}
	} else if (frame->can_id == CAN_DCNCP_NetworkChangeBaudRateReq) {
#ifdef SYS_CAN_DCNCP_BAUDRATE
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "***Baud Rate Change Request New Baud Rate %s Time left %dS\n\r", can_baud_rate_strings[frame->data[0]], frame->data[1]);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

		timer_cancel(&dcncp_network_baudrate_req_timer);
		TIMER_INIT(dcncp_network_baudrate_req_timer);

		data.sival_int = frame->data[0];   // Baudrate

		/*
		 * Create a timer to for time given in message.
		 */
		rc = timer_start(SECONDS_TO_TICKS(frame->data[1]), exp_network_baud_chage_req, data, &dcncp_network_baudrate_req_timer);
		if (rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Failed to start BaudRate change Request Timer\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			return;
		}
#else
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "CAN Baudrate change! NO code in place to process request!\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#endif // SYS_CAN_DCNCP_BAUDRATE
	} else if (frame->can_id == CAN_DCNCP_NodePingMessage) {
	} else if (frame->can_id == CAN_DCNCP_RegisterNetLogger) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Received NetLogger Registration Message\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#ifdef SYS_ISO15765_LOGGING
		iso15765_logger_register_remote(frame->data[0], frame->data[1]);
#endif // SYS_ISO15765_LOGGING
	} else if (frame->can_id == CAN_DCNCP_UnRegisterNetLogger) {
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "Received NetLogger UnRegistration Message\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#ifdef SYS_ISO15765_LOGGING
		iso15765_logger_unregister_remote(frame->data[0]);
#endif // SYS_ISO15765_LOGGING
	} else {
#if defined(MCP)
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		log_w(TAG, "Node Unrecognised Request %lx \n\r", frame->can_id);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#elif defined(ES_LINUX)
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_WARNING))
		log_w(TAG, "Node Unrecognised Request %x \n\r", frame->can_id);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#endif
	}
}

#if defined(ISO15765) || defined(ISO11783)
void exp_send_node_addr_report(timer_t timer_id __attribute__((unused)), union sigval data)
{
	can_frame frame;

	/*
	 * Clear the compiler warning
	 */
	data = data;

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "exp_send_node_addr_report(Address %x)\n\r", dcncp_node_address);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

	frame.can_id = CAN_DCNCP_NodeAddressReporting;
	frame.can_dlc = 2;
	frame.data[1] = dcncp_node_address;

	if (status.bit_field.dcncp_node_address_valid)
		frame.data[0] = TRUE;
	else
		frame.data[0] = FALSE;

	can_l2_tx_frame(&frame);
}
#endif // SYS_ISO15765 || SYS_ISO11783

/*
 * Net Logger Stuff
 */
#if defined(ISO15765_LOGGER)
result_t dcncp_register_this_node_net_logger(log_level_t level)
{
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "register_this_node_net_logger()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

	if(!iso15765_initialised())
		return(ERR_NOT_READY);

	local_iso15765_logger_frame.can_id = CAN_DCNCP_RegisterNetLogger;
	local_iso15765_logger_frame.can_dlc = 2;
	local_iso15765_logger_frame.data[0] = dcncp_node_address;
	local_iso15765_logger_frame.data[1] = level;

	can_l2_tx_frame(&local_iso15765_logger_frame);
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "NetLogger message sent\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "NetLogger message sent\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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

#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "DeRegAsNetLogger()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

	frame.can_id = CAN_DCNCP_UnRegisterNetLogger;
	frame.can_dlc = 1;
	frame.data[0] = dcncp_node_address;

	can_l2_tx_frame(&frame);
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "CancelNetLogger message sent\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

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
#if defined(SYS_LOG_LEVEL)
#if ((DEBUG_FILE == TRUE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Ping message sent\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
#endif // SYS_CAN_L2_PING_LOGGING
}

/**
 *
 * \file libesoup/comms/can/can.c
 *
 * Core SYS_CAN_BUS Functionality of electronicSoup CAN code
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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
#ifdef SYS_CAN_BUS

#include "libesoup/comms/can/can.h"

#ifndef SYS_SYSTEM_STATUS
#error "CAN Module relies on System Status module libesoup.h must define SYS_SYSTEM_STATUS"
#endif

#ifdef SYS_CAN_DCNCP
#include "libesoup/can/dcncp/dcncp_can.h"
#endif
#ifdef SYS_ISO15765_DCNCP
#include "libesoup/can/dcncp/dcncp_iso15765.h"
#endif // SYS_ISO15765_DCNCP

#ifdef SYS_CAN_PING_PROTOCOL
#include "libesoup/comms/can/ping.h"
#endif // SYS_CAN_PING_PROTOCOL

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
const char *TAG = "CAN";
#include "libesoup/logger/serial_log.h"

char can_l2_status_strings[5][17] = {
	"L2_Uninitialised",
	"L2_Listening",
	"L2_Connecting",
	"L2_Connected",
	"L2_ChangingBaud"
};

char can_baud_rate_strings[8][10] = {
	"baud_10K",
	"baud_20K",
	"baud_50K",
	"baud_125K",
	"baud_250K",
	"baud_500K",
	"baud_800K",
	"baud_1M"
};
#endif  // SYS_SERIAL_LOGGING

static can_status_t     can_status;

static void can_status_handler(union ty_status status);

status_handler_t app_status_handler = (status_handler_t)NULL;

result_t can_init(can_baud_rate_t baudrate, uint8_t address, status_handler_t status_handler)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("can_init\n\r");
#endif

        /*
         * Clear the stored SYS_CAN Status as nothing is done.
         */
	can_status.byte = 0x00;
	app_status_handler = status_handler;

	can_l2_init(baudrate, can_status_handler);

#ifdef SYS_CAN_PING_PROTOCOL
	can_ping_init();
#endif
	return(SUCCESS);
}

static void can_status_handler(union ty_status status)
{
	can_status_t        can_status;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("status_handler(mask-0x%x, status-0x%x\n\r", status.sword);
#endif
	if (status.sstruct.source == can_bus_status) {
		can_status.byte = status.sstruct.status;
		switch(can_status.bit_field.l2_status) {
			case L2_Uninitialised:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("L2_Uninitialised\n\r");
#endif
				break;
				
			case L2_Listening:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("L2_Listening\n\r");
#endif
				break;
				
			case L2_Connecting:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("L2_Connecting\n\r");
#endif
				break;
				
			case L2_Connected:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("L2_Connected\n\r");
#endif
				break;
				
			case L2_ChangingBaud:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
				LOG_D("L2_ChangingBaud\n\r");
#endif
				break;
			
			default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
				LOG_E("Unrecognised SYS_CAN Layer 2 status\n\r");
#endif
				break;
		}

#ifdef SYS_CAN_BUS_DCNCP
		if ((status.bit_field.l2_status == L2_Connected) && (can_status.bit_field.l2_status != L2_Connected)) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Layer 2 Connected so start DCNCP\n\r");
#endif
			dcncp_init(status_handler);
		}
#endif
		if (app_status_handler)
			app_status_handler(status);
	}

#ifdef SYS_CAN_BUS_DCNCP
	else if (mask == DCNCP_INIT_STATUS_MASK) {
		if(status.bit_field.dcncp_initialised) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("DCNCP_Initialised\n\r");
#endif
		} else {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("DCNCP_Uninitilised\n\r");
#endif
		}

		can_status.bit_field.dcncp_initialised = status.bit_field.dcncp_initialised;
		if (app_status_handler)
			app_status_handler(can_status, baud_status);
	}
#if defined(ISO15765) || defined(ISO11783)
	else if (mask == DCNCP_NODE_ADDRESS_STATUS_MASK) {
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("L3 Status update\n\r");
#endif
		if (status.bit_field.dcncp_node_address_valid && !can_status.bit_field.dcncp_node_address_valid) {
			can_status.bit_field.dcncp_node_address_valid = status.bit_field.dcncp_node_address_valid;
			if (app_status_handler)
				app_status_handler(can_status, baud_status);

#if defined(ISO15765)
			iso15765_init(dcncp_get_node_address());
#if defined(SYS_ISO15765_DCNCP)
			dcncp_iso15765_init();
#endif // SYS_ISO15765_DCNCP
#endif // SYS_ISO15765

		}
	}
#endif // SYS_ISO15765 || SYS_ISO11783
#endif // SYS_CAN_DCNCP

#if defined(ISO11783)
	iso11783_init(185);
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("iso11783 Initialised\n\r");
#endif
#endif  // SYS_ISO11783
}

#if defined(XC16) || defined(__XC8)
void can_tasks(void)
{
	can_l2_tasks();
}
#endif // XC16 || __XC8

#endif // SYS_CAN_BUS

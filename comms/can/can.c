/**
 * @file libesoup/comms/can/can.c
 * 
 * @author John Whitmore
 *
 * @brief Core SYS_CAN_BUS Functionality of electronicSoup CAN code
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
#ifdef SYS_CAN_BUS

#ifdef SYS_SERIAL_LOGGING
#undef DEBUG_FILE
static const char *TAG = "CAN";
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

#include "libesoup/errno.h"
#include "libesoup/status/status.h"
#include "libesoup/comms/can/can.h"

#ifndef SYS_SYSTEM_STATUS
#error "CAN Module relies on System Status module libesoup_config.h must define SYS_SYSTEM_STATUS"
#endif

#ifdef SYS_CAN_DCNCP
#include "libesoup/comms/can/dcncp/dcncp_can.h"
#endif
#ifdef SYS_CAN_ISO15765_DCNCP
#include "libesoup/can/dcncp/dcncp_iso15765.h"
#endif // SYS_CAN_ISO15765_DCNCP

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_SLAVE)
#include "libesoup/comms/can/ping.h"
#endif // SYS_CAN_PING_PROTOCOL

static void can_status_handler(status_source_t source, int16_t status, int16_t data);

#if (defined(SYS_CAN_ISO15765) || defined(SYS_ISO11783) || defined(SYS_TEST_L3_ADDRESS))
static uint8_t l3_address;
#endif

status_handler_t app_status_handler = (status_handler_t)NULL;

#if (defined(SYS_CAN_ISO15765) || defined(SYS_ISO11783) || defined(SYS_TEST_L3_ADDRESS))
result_t can_init(can_baud_rate_t baudrate, uint8_t arg_l3_address, status_handler_t status_handler, ty_can_l2_mode mode)
#else
result_t can_init(can_baud_rate_t baudrate, status_handler_t status_handler,  ty_can_l2_mode mode)
#endif
{
	result_t rc;

        /*
         * Clear the stored SYS_CAN Status as nothing is done.
         */
	app_status_handler = status_handler;
#if (defined(SYS_CAN_ISO15765) || defined(SYS_ISO11783) || defined(SYS_TEST_L3_ADDRESS))
	l3_address = arg_l3_address;
#endif
	/*
	 * Initialise the frame dispatcher
	 */
	frame_dispatch_init();

	/*
	 * Initialise layer 2
	 */
	rc = can_l2_init(baudrate, can_status_handler, mode);
	RC_CHECK_PRINT_CONT("Failed to initialise Layer \n\r");

#if defined(SYS_CAN_PING_PROTOCOL_PEER_TO_PEER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_MASTER) || defined(SYS_CAN_PING_PROTOCOL_CENTRALISED_SLAVE)
	can_ping_init();
#endif
	return(0);
}

static void can_status_handler(status_source_t source, int16_t status, int16_t data)
{
	result_t rc;

	rc = 0;

	switch(source) {
	case can_bus_l2_status:
		switch(status) {
		case can_l2_detecting_baud:
//			LOG_D("Bit Rate Auto Detect\n\r");
			if(app_status_handler) app_status_handler(source, status, data);
			break;
		case can_l2_connecting:
//			LOG_D("Connecting\n\r");
			if(app_status_handler) app_status_handler(source, status, data);
			break;
		case can_l2_connected:
//			LOG_D("Connected - %s\n\r", can_baud_rate_strings[data]);
#if defined(SYS_CAN_DCNCP)
			rc = dcncp_init(can_status_handler, l3_address);
			RC_CHECK_PRINT_VOID("DCNCP Fail\n\r");
#endif
			if(app_status_handler) app_status_handler(source, status, data);
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
			l3_address = data;
#if defined(SYS_CAN_ISO15765)
			rc = iso15765_init(l3_address);
			RC_CHECK_PRINT_VOID("ISO15765 Fail\n\r");
#endif
			if(app_status_handler) app_status_handler(source, status, data);
			break;
		}
		break;
#endif
#if defined(SYS_CAN_ISO15765)
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

#if defined(XC16) || defined(__XC8)
void can_tasks(void)
{
	can_l2_tasks();
}
#endif // XC16 || __XC8

#endif // SYS_CAN_BUS

/**
 *
 * \file libesoup/comms/can/can.c
 *
 * Core SYS_CAN_BUS Functionality of electronicSoup CAN code
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

#include "libesoup/comms/can/can.h"

#ifndef SYS_SYSTEM_STATUS
#error "CAN Module relies on System Status module libesoup_config.h must define SYS_SYSTEM_STATUS"
#endif

//#ifndef SYS_SW_TIMERS
//#error "CAN Module relies on Software Timers and must be enabled in libesoup_config.h"
//#endif

#ifdef SYS_CAN_DCNCP
#include "libesoup/comms/can/dcncp/dcncp_can.h"
#endif
#ifdef SYS_ISO15765_DCNCP
#include "libesoup/can/dcncp/dcncp_iso15765.h"
#endif // SYS_ISO15765_DCNCP

#ifdef SYS_CAN_PING_PROTOCOL
#include "libesoup/comms/can/ping.h"
#endif // SYS_CAN_PING_PROTOCOL

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
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

//static can_status_t     can_status;

static void can_status_handler(status_source_t source, int16_t status, int16_t data);

status_handler_t app_status_handler = (status_handler_t)NULL;

#if (defined(SYS_ISO15765) || defined(SYS_ISO11783)) || defined(SYS_TEST_L3_ADDRESS)
result_t can_init(can_baud_rate_t baudrate, uint8_t address, status_handler_t status_handler)
#else
result_t can_init(can_baud_rate_t baudrate, status_handler_t status_handler,  ty_can_l2_mode mode)
#endif
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("can_init\n\r");
#endif

        /*
         * Clear the stored SYS_CAN Status as nothing is done.
         */
//	can_status.byte = 0x00;
	app_status_handler = status_handler;

	/*
	 * Initialise the frame dispatcher
	 */
	frame_dispatch_init();

	/*
	 * Initialise layer 2
	 */
	can_l2_init(baudrate, can_status_handler, mode);

#ifdef SYS_CAN_PING_PROTOCOL
	can_ping_init();
#endif
	return(0);
}

static void can_status_handler(status_source_t source, int16_t status, int16_t data)
{
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("can_status_handler(src %d)\n\r", source);
#endif
	switch(source) {
	case can_bus_l2_status:
		switch(status) {
		case(can_l2_connecting):
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Connecting\n\r");
#endif
			if(app_status_handler) app_status_handler(source, status, data);
			break;
		case can_l2_connected:
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			LOG_D("Connected\n\r");
#endif
			if(app_status_handler) app_status_handler(source, status, data);
			break;
		default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
			LOG_E("Status? %d\n\r", status);
#endif		
			break;
		}
		break;
#if defined(SYS_CAN_DCNCP)
	case can_bus_dcncp_status:
		break;
#endif
#if defined(ISO15765)
	case iso15765_status:
		break;
#endif
#if defined(ISO11783)
	case iso11783_status:
		break;
#endif
	default:
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Status Src? %d\n\r", source);
#endif		
	}
}

#if defined(XC16) || defined(__XC8)
void can_tasks(void)
{
	can_l2_tasks();
}
#endif // XC16 || __XC8

#endif // SYS_CAN_BUS

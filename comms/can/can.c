/**
 *
 * \file libesoup/can/can.c
 *
 * Core SYS_CAN Functionality of electronicSoup CAN code
 *
 * Copyright 2017 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "libesoup/comms/can/es_can.h"
#ifdef SYS_CAN_DCNCP
#include "libesoup/can/dcncp/dcncp_can.h"
#endif
#ifdef SYS_ISO15765_DCNCP
#include "libesoup/can/dcncp/dcncp_iso15765.h"
#endif // SYS_ISO15765_DCNCP

#define DEBUG_FILE TRUE
#include "libesoup/logger/serial_log.h"

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
#define TAG "CAN"

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
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

static can_status_t can_status;
static can_baud_rate_t  baud_status = no_baud;

static void status_handler(uint8_t mask, can_status_t status, can_baud_rate_t baud);

can_status_handler_t app_status_handler = (can_status_handler_t)NULL;

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

result_t can_init(can_baud_rate_t baudrate,
		  can_status_handler_t arg_status_handler)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "can_init\n\r");
#endif

        /*
         * Clear the stored SYS_CAN Status as nothing is done.
         */
	can_status.byte = 0x00;
	app_status_handler = arg_status_handler;

	can_l2_init(baudrate, status_handler);

	return(SUCCESS);
}

static void status_handler(uint8_t mask, can_status_t status, can_baud_rate_t baud)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "status_handler(mask-0x%x, status-0x%x\n\r", mask, status.byte);
#endif

	if (mask == L2_STATUS_MASK) {
		switch(status.bit_field.l2_status) {
			case L2_Uninitialised:
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "L2_Uninitialised\n\r");
#endif
				break;
				
			case L2_Listening:
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "L2_Listening\n\r");
#endif
				break;
				
			case L2_Connecting:
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "L2_Connecting\n\r");
#endif
				break;
				
			case L2_Connected:
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "L2_Connected\n\r");
#endif
				break;
				
			case L2_ChangingBaud:
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "L2_ChangingBaud\n\r");
#endif
				break;
			
			default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Unrecognised SYS_CAN Layer 2 status\n\r");
#endif
				break;
		}

#ifdef SYS_CAN_DCNCP
		if ((status.bit_field.l2_status == L2_Connected) && (can_status.bit_field.l2_status != L2_Connected)) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "Layer 2 Connected so start DCNCP\n\r");
#endif
			dcncp_init(status_handler);
		}
#endif
		can_status.bit_field.l2_status = status.bit_field.l2_status;
		baud_status = baud;

		if (app_status_handler)
			app_status_handler(can_status, baud_status);
	}

#ifdef SYS_CAN_DCNCP
	else if (mask == DCNCP_INIT_STATUS_MASK) {
		if(status.bit_field.dcncp_initialised) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "DCNCP_Initialised\n\r");
#endif
		} else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "DCNCP_Uninitilised\n\r");
#endif
		}

		can_status.bit_field.dcncp_initialised = status.bit_field.dcncp_initialised;
		if (app_status_handler)
			app_status_handler(can_status, baud_status);
	}
#if defined(ISO15765) || defined(ISO11783)
	else if (mask == DCNCP_NODE_ADDRESS_STATUS_MASK) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "L3 Status update\n\r");
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
	log_d(TAG, "iso11783 Initialised\n\r");
#endif
#endif  // SYS_ISO11783
}

#if defined(XC16) || defined(__XC8)
void can_tasks(void)
{
	can_l2_tasks();
}
#endif // XC16 || __XC8

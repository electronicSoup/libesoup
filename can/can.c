/**
 *
 * \file es_lib/can/can.c
 *
 * Core CAN Functionality of electronicSoup CAN code
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

#include "system.h"
#include "es_lib/can/es_can.h"
#ifdef CAN_DCNCP
#include "es_lib/can/dcncp/dcncp.h"
#endif
#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"
#undef DEBUG_FILE

#if DEBUG_LEVEL < NO_LOGGING
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

static can_status_t can_status = { {}, 0x00 };
static can_baud_rate_t  baud_status = no_baud;

static void status_handler(u8 mask, can_status_t status, can_baud_rate_t baud);

can_status_handler_t app_status_handler = (can_status_handler_t)NULL;

result_t can_init(can_baud_rate_t baudrate,
		  can_status_handler_t arg_status_handler)
{
	LOG_D("can_init\n\r");

        /*
         * Clear the stored CAN Status as nothing is done.
         */
	can_status.byte = 0x00;
	app_status_handler = arg_status_handler;

	can_l2_init(baudrate, status_handler);

	return(SUCCESS);
}

void status_handler(u8 mask, can_status_t status, can_baud_rate_t baud)
{
	LOG_D("status_handler(mask-0x%x, status-0x%x\n\r", mask, status.byte);

	if (mask == L2_STATUS_MASK) {
		switch(status.bit_field.l2_status) {
			case L2_Uninitialised:
				LOG_D("L2_Uninitialised\n\r");
				break;
				
			case L2_Listening:
				LOG_D("L2_Listening\n\r");
				break;
				
			case L2_Connecting:
				LOG_D("L2_Connecting\n\r");
				break;
				
			case L2_Connected:
				LOG_D("L2_Connected\n\r");
				break;
				
			case L2_ChangingBaud:
				LOG_D("L2_ChangingBaud\n\r");
				break;
			
			default:
				LOG_E("Unrecognised CAN Layer 2 status\n\r");
				break;
		}

#ifdef CAN_DCNCP
		if ((status.bit_field.l2_status == L2_Connected) && (can_status.bit_field.l2_status != L2_Connected)) {
			LOG_D("Layer 2 Connected so start DCNCP\n\r");
			dcncp_init(status_handler);
		}
#endif
		can_status.bit_field.l2_status = status.bit_field.l2_status;
		baud_status = baud;

		if (app_status_handler)
			app_status_handler(can_status, baud_status);
	}

#ifdef CAN_DCNCP
	else if (mask == DCNCP_INIT_STATUS_MASK) {
		if(status.bit_field.dcncp_initialised) {
			LOG_D("DCNCP_Initialised\n\r");
		} else {
			LOG_D("DCNCP_Uninitilised\n\r");
		}

		can_status.bit_field.dcncp_initialised = status.bit_field.dcncp_initialised;
		if (app_status_handler)
			app_status_handler(can_status, baud_status);
	}
#if defined(CAN_LAYER_3)
	else if (mask == DCNCP_L3_ADDRESS_STATUS_MASK) {
		LOG_D("L3 Status update\n\r");
		if (status.bit_field.dcncp_l3_valid && !can_status.bit_field.dcncp_l3_valid) {
			can_status.bit_field.dcncp_l3_valid = status.bit_field.dcncp_l3_valid;
			if (app_status_handler)
				app_status_handler(can_status, baud_status);
			l3_init(status_handler);
		}
	}
#endif // CAN_LAYER_3
#endif // CAN_DCNCP
	
#if defined(CAN_LAYER_3)
	if (mask == L3_STATUS_MASK) {
		can_status.bit_field.l3_status = status.bit_field.l3_status;
		if (app_status_handler)
			app_status_handler(can_status, baud_status);
	}
#endif
}

#if defined(MCP)
void can_tasks(void)
{
	can_l2_tasks();
}
#endif

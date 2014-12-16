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

char baud_rate_strings[8][10] = {
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

static can_status_t can_status;
static baud_rate_t  baud_status = no_baud;

static void status_handler(u8 mask, can_status_t status, baud_rate_t baud);

can_status_handler app_status_handler = (can_status_handler)NULL;

result_t can_init(baud_rate_t baudRate,
		  can_status_handler arg_status_handler)
{
	LOG_D("can_init\n\r");

        /*
         * Clear the stored CAN Status as nothing is done.
         */
	can_status.byte = 0x00;
	app_status_handler = arg_status_handler;

	can_l2_init(baudRate, status_handler);

	return(SUCCESS);
}

void status_handler(u8 mask, can_status_t status, baud_rate_t baud)
{
	LOG_D("status_handler(mask-0x%x, status-0x%x\n\r", mask, status);
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
	}

#ifdef CAN_DCNCP
	if (mask == DCNCP_STATUS_MASK) {
		switch(status.bit_field.dcncp_status) {
			case DCNCP_Uninitilised:
				LOG_D("DCNCP_Uninitilised\n\r");
				break;

			case DCNCP_Initialised:
				LOG_D("DCNCP_Initialised\n\r");
				break;

			default:
				LOG_E("unrecognised DCNCP status\n\r");
				break;
		}

		if ((status.bit_field.dcncp_status & DCNCP_L3_Address_Finalised)
			&& (!(can_status.bit_field.dcncp_status & DCNCP_L3_Address_Finalised))) {
#if defined(CAN_LAYER_3)
			l3_init(status_handler);
#endif
		}
		can_status.bit_field.dcncp_status = status.bit_field.dcncp_status;
	}
#endif
	
#if defined(CAN_LAYER_3)
	if (mask == L3_STATUS_MASK) {
		can_status.bit_field.l3_status = status.bit_field.l3_status;
	}
#endif

	if (app_status_handler)
		app_status_handler(can_status, baud_status);
}

#if defined(MCP)
void canTasks(void)
{
	can_l2_tasks();
}
#endif

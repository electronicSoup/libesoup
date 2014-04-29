/**
 *
 * \file es_can/can/can.c
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
#define DEBUG_FILE
#include "es_lib/logger/serial.h"
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

static void status_handler(u8 mask, can_status_t status, baud_rate_t baud);

can_status_handler app_status_handler = (can_status_handler)NULL;

result_t can_init(baud_rate_t baudRate,
		  can_status_handler arg_status_handler)
{
	DEBUG_D("can_init\n\r");

        /*
         * Clear the stored CAN Status as nothing is done.
         */
	can_status.byte = 0x00;
	app_status_handler = arg_status_handler;

	l2_init(baudRate, status_handler);

	return(SUCCESS);
}

void status_handler(u8 mask, can_status_t status, baud_rate_t baud)
{
    if(mask == L2_STATUS_MASK) {
        if((status.bit_field.l2_status == L2_Connected) && (can_status.bit_field.l2_status != L2_Connected)) {
		DEBUG_D("Layer 2 Connected so start DCNCP\n\r");
		l2_dcncp_init(status_handler);
        }
        can_status.bit_field.l2_status = status.bit_field.l2_status;
    }

    if(mask == DCNCP_L3_ADDRESS_STATUS_MASK) {
        if(status.bit_field.dcncp_l3_address_final) {
#if defined(CAN_LAYER_3)
            l3_init(status_handler);
#endif
        }
    }

    if(app_status_handler)
	app_status_handler(can_status, baud);
}

#if defined(MCP)
void canTasks(void)
{
	L2_CanTasks();
}
#endif

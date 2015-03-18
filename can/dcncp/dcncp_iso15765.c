/**
 *
 * \file es_lib/can/network/network_management.c
 *
 * Code for managing CAN_Node OS networked nodes
 *
 * The code in shti file has very string linkage to an Application which only
 * means anything to the CAN Node OS CinnamonBun Firmware
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
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

#include "system.h"

#include "os_api.h"
#include "es_lib/can/es_can.h"
#include "es_lib/utils/flash.h"
#include "es_lib/utils/eeprom.h"
#include "es_lib/can/dcncp/cinnamonbun_info.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "DCNCP_ISO15765"

/*
 * Defined in the CAN_Node Project source? Have to restructure!!!
 */
extern BOOL app_valid;

typedef enum {
    hw_info_request        = 0x01,
    hw_info_response       = 0x02,
    boot_info_request      = 0x03,
    boot_info_response     = 0x04,
    os_info_request        = 0x05,
    os_info_response       = 0x06,
    app_info_request       = 0x07,
    app_info_response      = 0x08,
    app_status_request     = 0x09,
    app_status_response    = 0x0a,
    node_begin_reflash     = 0x0b,
    node_ready_next        = 0x0c,
    node_write_row         = 0x0d,
    node_erase_page        = 0x0e,
    node_reflash_finished  = 0x0f
} dcncp_iso15765_msg_t;

void (*app_status)(char *, u16 *);

//#define NODE_MAGIC              0x55

static void dcncp_iso15765_msg_handler(iso15765_msg_t *msg);
static void send_ready_response(u8 address);

void dcncp_iso15765_init()
{
	result_t          rc;
	iso15765_target_t target;

	LOG_D("dcncp_iso15765_init()\n\r");

	app_status = (void (*)(char *, u16))APP_STATUS_ADDRESS;

	target.protocol = ISO15765_DCNCP_PROTOCOL_ID;
	target.handler = dcncp_iso15765_msg_handler;

	rc = iso15765_dispatch_reg_handler(&target);
	if(rc != SUCCESS) {
		LOG_E("Failed to register Network Management iso15765 handler\n\r");
	}
}

static void dcncp_iso15765_msg_handler(iso15765_msg_t *msg)
{
	result_t       rc;
	iso15765_msg_t response;
	u8             response_buffer[ISO15765_MAX_MSG];
	u8             response_index = 0;
	u8             loop;
	u8             data[60];
	u16            length;
	u8             i = 0x00;
	u32            flash_address;
	char          *status;
	char           buffer[200];

	LOG_D("Message from Node 0x%x\n\r", (UINT16) msg->address);
	LOG_D("Message Protocol 0x%x\n\r", (UINT16) msg->protocol);
	LOG_D("First Data Byte is 0x%x\n\r", (UINT16) msg->data[0]);

	if (msg->protocol != ISO15765_DCNCP_PROTOCOL_ID) {
		LOG_E("Incorrect L3 Protocol received in NodeManagement %x\n\r", msg->protocol);
		return;
	}

	switch (msg->data[0]) {
		case hw_info_request:
			LOG_W("hw_info_request:\n\r");
			length = 200;
			rc = cb_get_hardware_info(buffer, &length);
			if(rc != SUCCESS) {
				LOG_E("Failed to read HW Info\n\r");
				return;
			}

			response_buffer[0] = hw_info_response;
			for (loop = 0; loop < length; loop++) {
				response_buffer[loop + 1] = buffer[loop];
			}


			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = length + 1;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
#if 0
			// The three items of data are 24 bytes long
			// max L3 Message is 74
			// 3*24 = 72
			// Add on L3_ID and Resp Code and max'd out.
			response_index = 0;
			response_buffer[response_index++] = hw_info_response;

			length = 60;
			flash_strcpy((char *) data, manfacturer, length);
			LOG_D("Add Manufacturer - %s\n\r", data);

			for (loop = 0; loop <= length; loop++) {
				if (response_index == ISO15765_MAX_MSG) {
					LOG_E("Response Buffer Overflow\n\r");
					break;
				}
				response_buffer[response_index++] = data[loop];
			}

			length = 60;
			flash_strcpy((char *) data, hardware, &length);
			LOG_D("Add Hardware - %s\n\r", data);
			for (loop = 0; loop <= length; loop++) {
				if (respIndex == ISO15765_MAX_MSG) {
					LOG_E("Response Buffer Overflow\n\r");
					break;
				}
				response_buffer[response_index++] = data[loop];
			}

			respMsg.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			respMsg.address = message->address;
			respMsg.size = response_index;
			respMsg.data = response_buffer;

			iso15765_tx_msg(&respMsg);
#endif
			break;

		case os_info_request:
			LOG_W("os_info_request Ignored for the moment\n\r");
#if 0
			response_index = 0;
			response_buffer[response_index++] = os_info_response;

			length = 60
			flash_strcpy((char *) data, os_code, &length);
			LOG_D("Add OS Code - %s\n\r", data);
			for (loop = 0; loop <= length; loop++) {
				if (respIndex == ISO15765_MAX_MSG) {
					LOG_E("Response Buffer Overflow\n\r");
					break;
				}
				response_buffer[response_index++] = data[loop];
			}
			LOG_D("OS_INFO_RESP size %d\n\r", response_index);

			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = response_index;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
#endif
			break;

		case app_info_request:
			LOG_W("app_info_request Ignored for the moment\n\r");
#if 0
			response_index = 0;
			response_buffer[response_index++] = app_info_response;

			if (app_valid) {
				length = 60;
				flash_strcpy((char *) data, appAuthor, &length);
				LOG_D("Add App Author - %s\n\r", data);
				for (loop = 0; loop <= length; loop++) {
					if (response_index == ISO15765_MAX_MSG) {
						LOG_E("Response Buffer Overflow\n\r");
						break;
					}
					response_buffer[response_index++] = data[loop];
				}

				length = 60;
				flash_strcpy((char *) data, (const rom char *) appSoftware, &length);
				LOG_D("Add App Software - %s\n\r", data);
				for (loop = 0; loop <= length; loop++) {
					if (respIndex == ISO15765_MAX_MSG) {
						LOG_E("Response Buffer Overflow\n\r");
						break;
					}
					response_buffer[response_index++] = data[loop];
				}

				length = 60;
				flash_strcpy((char *) data, appVersion, &length);
				LOG_D("Add app Version - %s\n\r", data);
				for (loop = 0; loop <= length; loop++) {
					if (response_index == ISO15765_MAX_MSG) {
						LOG_E("Response Buffer Overflow\n\r");
						break;
					}
					response_buffer[response_index++] = data[loop];
				}
				LOG_D("APP_INFO_RESP size %d\n\r", (UINT16) respIndex);
			} else {
				length = 60;
				flash_strcpy((char *) data, "Invalid", &length);
				serial_log(Debug, TAG, "App Invalid\n\r");
				for (loop = 0; loop <= length; loop++) {
					if (response_index == ISO15765_MAX_MSG) {
						LOG_E("Response Buffer Overflow\n\r");
						break;
					}
					response_buffer[response_index++] = data[loop];
				}
			}

			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = response_index;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
#endif
			break;

		case app_status_request:
			LOG_D("AppStatusRequest\n\r");
			response_index = 0;
			response_buffer[response_index++] = app_status_response;

			if (app_valid) {
				length = 60;
				app_status(buffer, &length);
				LOG_D("Status returning - %s\n\r", buffer);
			} else {
				LOG_D("Status Req no App installed\n\r");
				length = 60;
				flash_strcpy((char *) buffer, "No App Installed", &length);
			}
			LOG_D("Add App Status - length %d\n\r", length);
			LOG_D("Add App Status - %s\n\r", buffer);

			for (loop = 0; loop < length; loop++) {
				if (response_index == ISO15765_MAX_MSG) {
					LOG_E("Response Buffer Overflow\n\r");
					break;
				}
				response_buffer[response_index++] = buffer[loop];
			}
			LOG_D("APP_STATUS_RESP size %d\n\r", (UINT16) response_index);
			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = response_index;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
			break;

		case node_begin_reflash:
			LOG_D("ReFlash Request\n\r");
			os_remove_current_app();
			rc = eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR_1, 0x00);
			if(rc != SUCCESS) {
				LOG_E("Error writing to eeprom\n\r");
			}
			rc = eeprom_write((EEPROM_APP_VALID_MAGIC_ADDR_2), 0x00);
			if(rc != SUCCESS) {
				LOG_E("Error writing to eeprom\n\r");
			}
			app_valid = FALSE;

			// Respond with a ready message
			send_ready_response(msg->address);
			break;

		case node_reflash_finished:
			LOG_D("ReFlash Finished\n\r");

			/*
			 * Test the installed App to see if it's valid!
			 */
			if(  (flash_strlen((__prog__ char*)APP_AUTHOR_40_ADDRESS) < 40)
			   && (flash_strlen((__prog__ char*)APP_SOFTWARE_50_ADDRESS) < 50)
			   && (flash_strlen((__prog__ char*)APP_VERSION_10_ADDRESS) < 10)) {
				LOG_D("App Strings are valid\n\r");
				CALL_APP_INIT();
				LOG_D("Back from app_init() call app_main()\n\r");
				CALL_APP_MAIN();
				LOG_D("Application is valid\n\r");
				app_valid = TRUE;

				if (eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR_1, APP_VALID_MAGIC_VALUE) != SUCCESS) {
					LOG_E("Bad EEPROM Write\n\r");
				}
				if (eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR_2, (BYTE) (~APP_VALID_MAGIC_VALUE)) != SUCCESS) {
					LOG_E("BAD EEPROM Write\n\r");
				}
				ClrWdt();
			} else {
				LOG_E("Invalid App\n\r");
			}
			break;

		case node_write_row:
			LOG_W("Write Flash Row Ignored for the moment\n\r");
#if 0
			flash_address = 0x00;

			for (i = 0x01; i <= 4; i++) {
				flash_address = flash_address << 8;
				flash_address = flash_address | msg->data[i];
			}

			if ((msg->size - 5) == FLASH_PAGE_SIZE) {
				flash_write(address, &(message->data[5]));
				send_ready_response(message->address);
			} else {
				LOG_D("Bad data Size given %d expected %d\n\r",
                                      (UINT16)(message->size - 5),
                                      (UINT16)FLASH_PAGE_SIZE);
			}
#endif
			break;

		case node_erase_page:
			LOG_D("Erase Flash Page\n\r");
			flash_address = 0x00;

			for (i = 0x01; i <= 4; i++) {
				flash_address = flash_address << 8;
				flash_address = flash_address | msg->data[i];
			}

			flash_erase_page(flash_address);
			send_ready_response(msg->address);
			break;

		default:
			LOG_E("ERROR: Unprocessed NodeManagement message type 0x%x\n\r", (UINT16) msg->data[0]);
			break;
	}
}

void send_ready_response(BYTE address)
{
	iso15765_msg_t response;
	u8             response_buffer[3];

	response_buffer[0] = node_ready_next;

	response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
	response.address = address;
	response.size = 2;
	response.data = response_buffer;
	iso15765_tx_msg(&response);
}

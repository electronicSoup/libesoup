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

#include "system.h"

#include "es_lib/can/es_can.h"
#include "es_lib/utils/flash.h"
#include "es_lib/utils/eeprom.h"
#include "es_lib/can/dcncp/cinnamonbun_info.h"

#define DEBUG_FILE
#define LOG_LEVEL LOG_INFO
#include "es_lib/logger/serial_log.h"

#define TAG "DCNCP_ISO15765"

/*
 * Defined in the CAN_Node Project source? Have to restructure!!!
 */
extern BOOL app_valid;

typedef enum {
    hw_info_request           = 0x01,
    hw_info_response          = 0x02,
    boot_info_request         = 0x03,
    boot_info_response        = 0x04,
    firmware_info_request     = 0x05,
    firmware_info_response    = 0x06,
    node_config_info_request  = 0x07,
    node_config_info_response = 0x08,
    app_info_request          = 0x09,
    app_info_response         = 0x0a,
    app_status_request        = 0x0b,
    app_status_response       = 0x0c,
    node_begin_reflash        = 0x0d,
    node_ready_next           = 0x0e,
    node_write_row            = 0x0f,
    node_erase_page           = 0x10,
    node_reflash_finished     = 0x11
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
#ifdef DCNCP_ISO15765
	app_status = (void (*)(char *, u16))APP_STATUS_ADDRESS;

	target.protocol = ISO15765_DCNCP_PROTOCOL_ID;
	target.handler = dcncp_iso15765_msg_handler;

	rc = iso15765_dispatch_reg_handler(&target);
	if(rc != SUCCESS) {
		LOG_E("Failed to register Network Management iso15765 handler\n\r");
	}
#endif
}

#ifdef DCNCP_ISO15765
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
	u8             buffer[200];

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
			break;

 		case boot_info_request:
			LOG_W("boot_info_request:\n\r");
			length = 200;
			rc = cb_get_boot_info(buffer, &length);
			if(rc != SUCCESS) {
				LOG_E("Failed to read Boot Info\n\r");
				return;
			}

			response_buffer[0] = boot_info_response;
			for (loop = 0; loop < length; loop++) {
				response_buffer[loop + 1] = buffer[loop];
			}

			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = length + 1;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
			break;

		case firmware_info_request:
			LOG_W("firmware_info_request\n\r");
			length = 200;
			rc = cb_get_firmware_info(buffer, &length);
			if(rc != SUCCESS) {
				LOG_E("Failed to read Firmware Info\n\r");
				return;
			}

			response_buffer[0] = firmware_info_response;
			for (loop = 0; loop < length; loop++) {
				response_buffer[loop + 1] = buffer[loop];
			}

			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = length + 1;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
			break;

		case node_config_info_request:
			LOG_W("node_config_info_request\n\r");
			length = 200;
			rc = cb_get_node_config_info(buffer, &length);
			if(rc != SUCCESS) {
				LOG_E("Failed to read Firmware Info\n\r");
				return;
			}

			response_buffer[0] = node_config_info_response;
			for (loop = 0; loop < length; loop++) {
				response_buffer[loop + 1] = buffer[loop];
			}

			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = length + 1;
			response.data = response_buffer;

			iso15765_tx_msg(&response);
			break;

		case app_info_request:
			LOG_D("AppInfoRequest\n\r");
			length = 200;
			rc = cb_get_application_info(buffer, &length);
			if(rc != SUCCESS) {
				LOG_E("Failed to read Application Info\n\r");
				return;
			}

			response_buffer[0] = app_info_response;
			for (loop = 0; loop < length; loop++) {
				response_buffer[loop + 1] = buffer[loop];
			}

			response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
			response.address = msg->address;
			response.size = length + 1;
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
			LOG_I("ReFlash Finished\n\r");

			/*
			 * Test the installed App to see if it's valid!
			 */
			length = flash_strlen((__prog__ char*)APP_AUTHOR_40_ADDRESS);

			if(  (flash_strlen((__prog__ char*)APP_AUTHOR_40_ADDRESS) < 40)
			   && (flash_strlen((__prog__ char*)APP_SOFTWARE_50_ADDRESS) < 50)
			   && (flash_strlen((__prog__ char*)APP_VERSION_10_ADDRESS) < 10)) {
				LOG_I("App Strings are valid\n\r");
				CALL_APP_INIT();
				LOG_I("Back from app_init() call app_main()\n\r");
				CALL_APP_MAIN();
				LOG_I("Application is valid\n\r");
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
			LOG_W("Write Flash Row\n\r");

			flash_address = 0x00;

			for (i = 0x01; i <= 4; i++) {
				flash_address = flash_address << 8;
				flash_address = flash_address | msg->data[i];
			}

			if(  (flash_address < APP_START_FLASH_ADDRESS)
			   &&((flash_address < APP_HANDLE_FLASH_ADDRESS) || (flash_address >= APP_HANDLE_FLASH_ADDRESS + FLASH_PAGE_SIZE))) {
				LOG_E("Bad address to Write to row\n\r");
			} else {
				rc = flash_write_row(flash_address, &msg->data[5]);
				if (rc != SUCCESS) {
					LOG_E("write row to address 0x%lx\n\r", flash_address);
				}
				send_ready_response(msg->address);
			}
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
#endif

#ifdef DCNCP_ISO15765
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
#endif

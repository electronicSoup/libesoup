/**
 *
 * \file libesoup/can/network/network_management.c
 *
 * Code for managing CAN_Node OS networked nodes
 *
 * The code in shti file has very string linkage to an Application which only
 * means anything to the CAN Node OS CinnamonBun Firmware
 *
 * Copyright 2017 electronicSoup Limited
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

#include "libesoup/can/es_can.h"
#include "libesoup/utils/flash.h"
#include "libesoup/utils/eeprom.h"
#include "libesoup/can/dcncp/cinnamonbun_info.h"

#define DEBUG_FILE
//#define SYS_LOG_LEVEL LOG_INFO
#include "libesoup/logger/serial_log.h"

#define TAG "ISO15765_DCNCP"

extern void  os_remove_current_app(void);

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

void (*app_status)(char *, uint16_t *);

//#define NODE_MAGIC              0x55

static void dcncp_iso15765_msg_handler(iso15765_msg_t *msg);
static void send_ready_response(uint8_t address);

void dcncp_iso15765_init()
{
	result_t          rc;
	iso15765_target_t target;

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "dcncp_iso15765_init()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

#ifdef SYS_ISO15765_DCNCP
	app_status = (void (*)(char *, uint16_t))APP_STATUS_ADDRESS;

	target.protocol = ISO15765_DCNCP_PROTOCOL_ID;
	target.handler = dcncp_iso15765_msg_handler;

	rc = iso15765_dispatch_reg_handler(&target);
	if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to register Network Management iso15765 handler\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
	}
#endif
}

#ifdef SYS_ISO15765_DCNCP
static void dcncp_iso15765_msg_handler(iso15765_msg_t *msg)
{
	result_t       rc;
	iso15765_msg_t response;
	uint8_t             response_buffer[ISO15765_MAX_MSG];
	uint8_t             response_index = 0;
	uint8_t             loop;
	uint8_t             data[60];
	uint16_t            length;
	uint8_t             i = 0x00;
	u32            flash_address;
	char          *status;
	uint8_t             buffer[200];

#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Message from Node 0x%x\n\r", (uint16_t) msg->address);
	log_d(TAG, "Message Protocol 0x%x\n\r", (uint16_t) msg->protocol);
	log_d(TAG, "First Data Byte is 0x%x\n\r", (uint16_t) msg->data[0]);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

	if (msg->protocol != ISO15765_DCNCP_PROTOCOL_ID) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Incorrect L3 Protocol received in NodeManagement %x\n\r", msg->protocol);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
		return;
	}

	switch (msg->data[0]) {
 		case hw_info_request:
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
			log_w(TAG, "hw_info_request:\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			length = 200;
			rc = cb_get_hardware_info(buffer, &length);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Failed to read HW Info\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
			log_w(TAG, "boot_info_request:\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			length = 200;
			rc = cb_get_boot_info(buffer, &length);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Failed to read Boot Info\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
			log_w(TAG, "firmware_info_request\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			length = 200;
			rc = cb_get_firmware_info(buffer, &length);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Failed to read Firmware Info\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
			log_w(TAG, "node_config_info_request\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			length = 200;
			rc = cb_get_node_config_info(buffer, &length);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Failed to read Firmware Info\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "AppInfoRequest\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			length = 200;
			rc = cb_get_application_info(buffer, &length);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Failed to read Application Info\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "ReFlash Request\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			os_remove_current_app();
			rc = eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR_1, 0x00);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Error writing to eeprom\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			}
			rc = eeprom_write((EEPROM_APP_VALID_MAGIC_ADDR_2), 0x00);
			if(rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Error writing to eeprom\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			}
			app_valid = FALSE;

			// Respond with a ready message
			send_ready_response(msg->address);
			break;

		case node_reflash_finished:
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
			log_i(TAG, "ReFlash Finished\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

			/*
			 * Test the installed App to see if it's valid!
			 */
			length = flash_strlen((__prog__ char*)APP_AUTHOR_40_ADDRESS);

			if(  (flash_strlen((__prog__ char*)APP_AUTHOR_40_ADDRESS) < 40)
			   && (flash_strlen((__prog__ char*)APP_SOFTWARE_50_ADDRESS) < 50)
			   && (flash_strlen((__prog__ char*)APP_VERSION_10_ADDRESS) < 10)) {
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
				log_i(TAG, "App Strings are valid\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				CALL_APP_INIT();
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
				log_i(TAG, "Back from app_init() call app_main()\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				CALL_APP_MAIN();
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_INFO))
				log_i(TAG, "Application is valid\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				app_valid = TRUE;

				if (eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR_1, APP_VALID_MAGIC_VALUE) != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Bad EEPROM Write\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				}
				if (eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR_2, (uint8_t) (~APP_VALID_MAGIC_VALUE)) != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "BAD EEPROM Write\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
				}
				ClrWdt();
			} else {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Invalid App\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			}
			break;

		case node_write_row:
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_WARNING))
			log_w(TAG, "Write Flash Row\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)

			flash_address = 0x00;

			for (i = 0x01; i <= 4; i++) {
				flash_address = flash_address << 8;
				flash_address = flash_address | msg->data[i];
			}

			if(  (flash_address < APP_START_FLASH_ADDRESS)
			   &&((flash_address < APP_HANDLE_FLASH_ADDRESS) || (flash_address >= APP_HANDLE_FLASH_ADDRESS + FLASH_PAGE_SIZE))) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Bad address to Write to row\n\r");
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			} else {
				rc = flash_write_row(flash_address, &msg->data[5]);
				if (rc != SUCCESS) {
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "write row to address 0x%lx\n\r", flash_address);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
				log_d(TAG, "Bad data Size given %d expected %d\n\r",
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
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
#if defined(SYS_LOG_LEVEL)
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "ERROR: Unprocessed NodeManagement message type 0x%x\n\r", (uint16_t) msg->data[0]);
#endif
#else  //  if defined(SYS_LOG_LEVEL)
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif //  if defined(SYS_LOG_LEVEL)
			break;
	}
}
#endif

#ifdef SYS_ISO15765_DCNCP
void send_ready_response(uint8_t address)
{
	iso15765_msg_t response;
	uint8_t             response_buffer[3];

	response_buffer[0] = node_ready_next;

	response.protocol = ISO15765_DCNCP_PROTOCOL_ID;
	response.address = address;
	response.size = 2;
	response.data = response_buffer;
	iso15765_tx_msg(&response);
}
#endif

/**
 *
 * \file es_lib/usb/android/states/node_connected.c
 *
 * The Android Application Connected state for the Android Communications.
 *
 * State for processing comms from the Android Application.
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
#include <stdio.h>
#include "system.h"
#include "usb/usb.h"
#include "usb/usb_host_android.h"
#include "node_ipc.h"
#include "es_lib/usb/android/android_state.h"
#include "es_lib/usb/android/states/states.h"
#include "es_lib/utils/flash.h"
#include "es_lib/utils/eeprom.h"
#include "es_lib/usb/android/android.h"
//#include "main.h"

#ifdef ANDROID_NODE
#include "os/os_api.h"
#endif //ANDROID_NODE

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"

#define TAG "Android-NodeConnected"

#define HARDWARE_INFO_BASE 0x200
//
// Hardware Info
//
extern __prog__ char hardware_manufacturer[24] __attribute__ ((space(prog),address(HARDWARE_INFO_BASE)));
extern __prog__ char hardware_model[24]        __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24)));
extern __prog__ char hardware_description[50]  __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24)));
extern __prog__ char hardware_version[10]      __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50)));
extern __prog__ char hardware_uri[50]          __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10)));
//
// Bootloader Info
//
extern __prog__ char bootcode_author[40]       __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50)));
extern __prog__ char bootcode_description[50]  __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40)));
extern __prog__ char bootcode_version[10]      __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40 + 50)));
extern __prog__ char bootcode_uri[50]          __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40 + 50 + 10)));

extern __prog__ char firmware_author[40];
extern __prog__ char firmware_description[50];
extern __prog__ char firmware_version[10];
extern __prog__ char firmware_uri[50];

extern __prog__ char app_author[40];
extern __prog__ char app_software[50];
extern __prog__ char app_version[10];
extern __prog__ char app_uri[50];

//void app_connected_process_msg(android_command_t, void *, UINT16);
void app_connected_process_msg(BYTE, void *, UINT16);
void app_connected_main(void);
void app_connected_process_usb_event(USB_EVENT event);

static void transmit_ready(void);
static void transmit_app_type_info(void);
static void transmit_hardware_info(void);
static void transmit_bootcode_info(void);
static void transmit_firmware_info(void);
#ifdef ANDROID_NODE
static void transmit_application_info(void);
static void transmit_node_config_info(void);
#endif //ANDROID_NODE
#ifdef ANDROID_BOOT
extern void jmp_firmware(void);
extern BOOL firmware_valid;
#endif //ANDROID_BOOT


void set_node_connected_state(void)
{
	LOG_D("App Connected State\n\r");
	// Android App connected so cancel the timer.
	// Android App now controls booting
	T2CONbits.TON = 0;

	current_state.process_msg = app_connected_process_msg;
	current_state.main = app_connected_main;
	current_state.process_usb_event = app_connected_process_usb_event;
}

void app_connected_process_msg(BYTE cmd, void *data, UINT16 data_len)
{
	UINT32 address;
	BYTE *byte_data;
	UINT8 loop;
	UINT16 length;
	result_t rc;

	//    DEBUG_D("app_connected_process_msg data lenght %d\n\r", data_len);
	switch (cmd) {
#ifdef ANDROID_BOOT
		case COMMAND_BOOT:
			LOG_D("COMMAND_BOOT Jump to application\n\r");
			jmp_firmware();
			break;
#endif //ANDROID_BOOT

		case COMMAND_BEGIN_FLASH:
			LOG_D("COMMAND_BEGIN_FLASH\n\r");
#ifdef ANDROID_NODE
			os_cancel_all_timers();
			os_clear_app_eeprom();
			eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR, 0x00);
			eeprom_write((EEPROM_APP_VALID_MAGIC_ADDR + 1), 0x00);
			app_valid = FALSE;
#elif defined(ANDROID_BOOT)
			eeprom_write(EEPROM_FIRMWARE_VALID_MAGIC_ADDR, 0x00);
			eeprom_write((EEPROM_FIRMWARE_VALID_MAGIC_ADDR + 1), 0x00);
			eeprom_erase(EEPROM_BOOT_PAGE_SIZE);
			firmware_valid = FALSE;
#endif //ANDROID_NODE / ANDROID_BOOT
			transmit_ready();
			break;

		case COMMAND_ERASE:
			if (data != NULL) {
				byte_data = (BYTE *) data;

				address = 0x00;
				for (loop = 0; loop < 4; loop++) {
					address = (address << 8) | (byte_data[loop] & 0xff);
				}

				LOG_I("COMMAND_ERASE 0x%lx\n\r", address);
#if defined(ANDROID_NODE)
				if(  (address < APP_START_FLASH_ADDRESS)
				   &&(address != APP_HANDLE_FLASH_ADDRESS)) {
#elif defined(ANDROID_BOOT)
				if (address < FLASH_FIRMWARE_START_ADDRESS) {
#endif
					LOG_E("Bad address to Erase\n\r");
				} else {
					if (!flash_page_empty(address)) {
						rc = flash_erase_page(address);
						if(rc != SUCCESS) {
							LOG_E("Failed to erase page 0x%lx", address);
						}
					} else {
						LOG_D("Already empty\n\r");
					}
					transmit_ready();
				}
			}
			break;

		case COMMAND_ROW:
			if (data != NULL) {
				byte_data = (BYTE *) data;

				address = 0x00;
				for (loop = 0; loop < 4; loop++) {
					address = (address << 8) | (byte_data[loop] & 0xff);
				}

				LOG_D("COMMAND_ROW address 0x%lx data length 0x%x\n\r", address, data_len);
#if defined(ANDROID_NODE)
				if ((address < APP_START_FLASH_ADDRESS)
					&& (address != APP_HANDLE_FLASH_ADDRESS)) {
					LOG_E("Bad address to Write to row\n\r");
#elif defined(ANDROID_BOOT)
				if (address < FLASH_FIRMWARE_START_ADDRESS) {
					LOG_E("Bad address to Write to row\n\r");
#endif
				} else {
					rc = flash_write_row(address, &byte_data[4]);
					if(rc != SUCCESS) {
						LOG_E("Failed to write row to address 0x%lx\n\r", address);
					}
					transmit_ready();
				}
			}
			break;

		case COMMAND_REFLASHED:
			LOG_D("COMMAND_REFLASHED\n\r");
#ifdef ANDROID_NODE
			CALL_APP_INIT();
			CALL_APP_MAIN();
			LOG_D("Application is valid\n\r");
			app_valid = TRUE;

			if (eeprom_write(EEPROM_APP_VALID_MAGIC_ADDR, APP_VALID_MAGIC_VALUE) != SUCCESS) {
				LOG_E("Bad EEPROM Write\n\r");
			}
			if (eeprom_write((EEPROM_APP_VALID_MAGIC_ADDR + 1), (u8) (~APP_VALID_MAGIC_VALUE)) != SUCCESS) {
				LOG_E("BAD EEPROM Write\n\r");
			}
#elif defined(ANDROID_BOOT)
			eeprom_write(EEPROM_FIRMWARE_VALID_MAGIC_ADDR, FIRMWARE_VALID_MAGIC_VALUE);
			eeprom_write((EEPROM_FIRMWARE_VALID_MAGIC_ADDR + 1), (u8) (~FIRMWARE_VALID_MAGIC_VALUE));
			firmware_valid = TRUE;
			break;
#endif //ANDROID_NODE / ANDROID_BOOT

		case ANDROID_APP_TYPE_REQ:
			LOG_D("ANDROID_APP_TYPE_REQ\n\r");
			transmit_app_type_info();
			break;

		case HARDWARE_INFO_REQ:
			LOG_D("HARDWARE_INFO_REQ\n\r");
			transmit_hardware_info();
			break;

		case BOOTCODE_INFO_REQ:
			LOG_D("BOOTCODE_INFO_REQ\n\r");
			transmit_bootcode_info();
			break;

		case FIRMWARE_INFO_REQ:
			LOG_D("FIRMWARE_INFO_REQ\n\r");
			transmit_firmware_info();
			break;

#ifdef ANDROID_NODE
		case APPLICATION_INFO_REQ:
			LOG_D("APPLICATION_INFO_REQ\n\r");
			transmit_application_info();
			break;
#endif //ANDROID_NODE

#ifdef ANDROID_NODE
		case NODE_CONFIG_INFO_REQ:
			LOG_D("NODE_CONFIG_INFO_REQ\n\r");
			transmit_node_config_info();
			break;
#endif //ANDROID_NODE

#ifdef ANDROID_NODE
		case NODE_CONFIG_UPDATE:
			byte_data = (BYTE *) data;
			LOG_D("NODE_CONFIG_UPDATE\n\r");
			LOG_D("data[0] = 0x%x\n\r", byte_data[0]);
			LOG_D("data[1] = 0x%x\n\r", byte_data[1]);
			LOG_D("data[2] = 0x%x\n\r", byte_data[2]);
			LOG_D("descrioption String is %s\n\r", &byte_data[3]);

			eeprom_write(EEPROM_L3_NODE_ADDRESS_ADDR, byte_data[0]);
			eeprom_write(EEPROM_CAN_BAUD_RATE_ADDR, byte_data[1]);
			eeprom_write(EEPROM_IO_ADDRESS_ADDR, byte_data[2]);

			rc = eeprom_str_write(EEPROM_NODE_DESCRIPTION_ADDR, &byte_data[3], &length);
			if(rc != SUCCESS) {
				LOG_E("Error writing node description to eeprom\n\r");
			}
			LOG_D("%d bytes written to EEPROM\n\r", length);
			break;
#endif //ANDROID_NODE

		default:
			LOG_W("Unprocessed message 0x%x\n\r", cmd);
			break;
	}
}

void app_connected_main()
{
}

void app_connected_process_usb_event(USB_EVENT event)
{
	switch (event) {
		case EVENT_ANDROID_ATTACH:
			break;

		case EVENT_ANDROID_DETACH:
			set_idle_state();
			break;

		default:
			break;
	}
}

static void transmit_ready(void)
{
	BYTE buffer[2];

	buffer[0] = 1;
	buffer[1] = COMMAND_READY;

	android_transmit(buffer, 2);
}

void transmit_app_type_info(void)
{
	char buffer[3];

	LOG_D("transmit_app_type_info()\n\r");

	buffer[0] = 2;
	buffer[1] = ANDROID_APP_TYPE_RESP;
#if defined(ANDROID_BOOT)
	buffer[2] = BOOTLOADER_APP;
#elif defined(ANDROID_NODE)
	buffer[2] = NODE_CONFIG_APP;
#endif
	android_transmit((BYTE *) buffer, 3);
}
void transmit_hardware_info(void)
{
	char buffer[160];
	BYTE index = 0;
	UINT16 length;

	LOG_D("transmit_hardware_info()\n\r");

	buffer[1] = HARDWARE_INFO_RESP;

	index = 2;

	length = 24;
	index += flash_strcpy(&buffer[index], hardware_manufacturer, &length) + 1;

	length = 24;
	index += flash_strcpy(&buffer[index], hardware_model, &length) + 1;

	length = 50;
	index += flash_strcpy(&buffer[index], hardware_description, &length) + 1;

	length = 10;
	index += flash_strcpy(&buffer[index], hardware_version, &length) + 1;

	length = 50;
	index += flash_strcpy(&buffer[index], hardware_uri, &length) + 1;

	LOG_D("Transmit boot info length %d\n\r", index);
	buffer[0] = index;

	android_transmit((BYTE *) buffer, index);
}

void transmit_bootcode_info(void)
{
	//    char string[50];
	char buffer[152];
	BYTE index = 0;
	UINT16 length;

	buffer[1] = BOOTCODE_INFO_RESP;

	index = 2;

	length = 40;
	index += flash_strcpy(&buffer[index], bootcode_author, &length) + 1;

	length = 50;
	index += flash_strcpy(&buffer[index], bootcode_description, &length) + 1;

	length = 10;
	index += flash_strcpy(&buffer[index], bootcode_version, &length) + 1;

	length = 50;
	index += flash_strcpy(&buffer[index], bootcode_uri, &length) + 1;

	LOG_D("Transmit boot info length %d\n\r", index);
	buffer[0] = index;

	android_transmit((BYTE *) buffer, index);
}

void transmit_firmware_info(void)
{
	char buffer[152];
	UINT16 index = 0;
	UINT16 length;

	buffer[1] = FIRMWARE_INFO_RESP;

	index = 2;

	length = 40;
	index += flash_strcpy(&buffer[index], firmware_author, &length) + 1;

	length = 50;
	index += flash_strcpy(&buffer[index], firmware_description, &length) + 1;

	length = 10;
	index += flash_strcpy(&buffer[index], firmware_version, &length) + 1;

	length = 50;
	index += flash_strcpy(&buffer[index], firmware_uri, &length) + 1;

	LOG_D("Transmit Firmware info length %d\n\r", index);
	buffer[0] = index;

	android_transmit((BYTE *) buffer, index);
}

#ifdef ANDROID_NODE
void transmit_application_info(void)
{
	char buffer[153];
	char test[50];
	UINT16 index = 0;
	UINT16 length;

	buffer[1] = APPLICATION_INFO_RESP;

	buffer[2] = app_valid;

	index = 3;

	length = 40;
	flash_strcpy(test, app_author, &length);
	LOG_D("App Author is:%s\n\r", test);

	length = 40;
	index += flash_strcpy(&buffer[index], app_author, &length) + 1;

	length = 50;
	flash_strcpy(test, app_software, &length);
	LOG_D("App Software is:%s\n\r", test);

	length = 50;
	index += flash_strcpy(&buffer[index], app_software, &length) + 1;

	length = 10;
	flash_strcpy(test, app_version, &length);
	LOG_D("App Version is:%s\n\r", test);

	length = 10;
	index += flash_strcpy(&buffer[index], app_version, &length) + 1;

	length = 50;
	flash_strcpy(test, app_uri, &length);
	LOG_D("App URI is:%s\n\r", test);

	length = 50;
	index += flash_strcpy(&buffer[index], app_uri, &length) + 1;

	LOG_D("Transmit Application info length %d\n\r", index);
	buffer[0] = index;

	android_transmit((BYTE *) buffer, index);
}
#endif //ANDROID_NODE

#ifdef ANDROID_NODE
void transmit_node_config_info(void)
{
	BYTE buffer[55];
	UINT16 index;
	UINT16 length;
	BYTE value;
	result_t rc;

	buffer[1] = NODE_CONFIG_INFO_RESP;

	index = 2;

	eeprom_read(EEPROM_L3_NODE_ADDRESS_ADDR, (BYTE *) & value);
	LOG_D("Layer 3 Node Address 0x%x\n\r", value);
	buffer[index++] = value;

	eeprom_read(EEPROM_CAN_BAUD_RATE_ADDR, (BYTE *) & value);
	LOG_D("CAN Baud Rate 0x%x\n\r", value);
	buffer[index++] = value;

	eeprom_read(EEPROM_IO_ADDRESS_ADDR, (BYTE *) & value);
	LOG_D("I/O Address 0x%x\n\r", value);
	buffer[index++] = value;

	length = 50;
	rc = eeprom_str_read(EEPROM_NODE_DESCRIPTION_ADDR, &buffer[index], &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to write the node description\n\r");
	}
	LOG_D("Description: String Length %d string '%s'\n\r", length, &buffer[index]);

	index = index + length;
	buffer[0] = index;

	android_transmit((BYTE *) buffer, index);
}
#endif //ANDROID_NODE

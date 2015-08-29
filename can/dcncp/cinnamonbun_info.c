/**
 *
 * \file es_lib/can/dcncp/cinnamonbun_info.c
 *
 * Functions for retrieving the CinnamonBun Info Strings
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
#define DEBUG_FILE
#include "es_lib/utils/flash.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/can/dcncp/cinnamonbun_info.h"
#include "es_lib/firmware/firmware.h"
#include "es_lib/utils/eeprom.h"

#define TAG "CB_INFO"

result_t cb_get_hardware_info(u8 *data, u16 *data_len)
{
	u8       buffer[200];
	u8      *data_ptr;
	u16      size;
	u16      length;
	u16      loop;
	result_t rc;

	LOG_D("cb_get_hardare_info()\n\r");

	data_ptr = data;
	size = 0;

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) HW_MANUFACTURER_24_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read HW Manufacturer\n\r");
		return (rc);
	}

	LOG_D("Hw Manufacturer read as %s length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_MODEL_24_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to ready HW Model\n\r");
		return (rc);
	}

	LOG_D("HW Model read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read HW Description\n\r");
		return (rc);
	}

	LOG_D("HW Description read as %s length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) HW_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the HW Verson\n\r");
		return(rc);
	}

	LOG_D("HW Version read as %s, lenght %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the HW URI\n\r");
		return(rc);
	}

	LOG_D("HW URI read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
	LOG_D("Hardware info length %d\n\r", size);
	return (SUCCESS);
}

result_t cb_get_boot_info(u8 *data, u16 *data_len)
{
	u8       buffer[200];
	u8      *data_ptr;
	u16      size;
	u16      length;
	u16      loop;
	result_t rc;

	LOG_D("cb_get_boot_info()\n\r");

	data_ptr = data;
	size = 0;

	length = 40;

	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read HW Manufacturer\n\r");
		return (rc);
	}

	LOG_D("Boot Author read as %s length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to ready HW Model\n\r");
		return (rc);
	}

	LOG_D("Boot Description read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) BOOT_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the HW Verson\n\r");
		return(rc);
	}

	LOG_D("Boot Version read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the HW URI\n\r");
		return(rc);
	}

	LOG_D("Boot URI read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
	LOG_D("Boot info length %d\n\r", size);
	return (SUCCESS);
}

result_t cb_get_firmware_info(u8 *data, u16 *data_len)
{
	u8       buffer[200];
	u8      *data_ptr;
	u16      size;
	u16      length;
	u16      loop;
	result_t rc;

	LOG_D("cb_get_firmware_info()\n\r");

	data_ptr = data;
	size = 0;

	length = 40;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read Firmware Author\n\r");
		return (rc);
	}

	LOG_D("Firmware Author read as %s length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to ready FIrmware Desc\n\r");
		return (rc);
	}

	LOG_D("Firmware Description read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) FIRMWARE_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the Firmware Verson\n\r");
		return(rc);
	}

	LOG_D("Firmware Version read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_URL_50_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the HW URI\n\r");
		return(rc);
	}

	LOG_D("Firmware URI read as %s, length %d\n\r", buffer, length);
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
	LOG_D("Boot info length %d\n\r", size);
	return (SUCCESS);
}

result_t cb_get_application_info(u8 *data, u16 *data_len)
{
	u8       buffer[154];
	u8      *data_ptr;
	u16      size;
	u16      length;
	u16      loop;
	result_t rc;

	LOG_D("cb_get_application_info()\n\r");

	data_ptr = data;
	size = 0;

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read the App Author\n\r");
		return (rc);
	}
	LOG_D("Application Author read as %s, length %d\n\r", buffer, length);

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_SOFTWARE_50_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read the App Software\n\r");
		return (rc);
	}
	LOG_D("App Software read as %s, length %d\n\r", buffer, length);

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_VERSION_10_ADDRESS, &length);
	if (rc != SUCCESS) {
		LOG_E("Failed to read the App Version\n\r");
		return (rc);
	}
	LOG_D("App Version read as %s, length %d\n\r", buffer, length);

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the App URI\n\r");
		return(rc);
	}
	LOG_D("App URI read as %s, length %d\n\r", buffer, length);

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
	LOG_D("Application info length %d\n\r", size);
	return (SUCCESS);
}

#ifdef CAN_NODE_OS
result_t cb_get_node_config_info(u8 *data, u16 *data_len)
{
	u8       buffer[200];
	u8      *data_ptr;
	u16      size;
	u16      length;
	u16      loop;
	u8       value;
	result_t rc;

	LOG_D("cb_get_node_config_info()\n\r");

	data_ptr = data;
	size = 0;

	eeprom_read(EEPROM_NODE_ADDRESS, (u8 *)&value);
	LOG_D("Layer 3 Node Address 0x%x\n\r", value);
	*data_ptr++ = (char)value;
	size++;

	eeprom_read(EEPROM_CAN_BAUD_RATE_ADDR, (u8 *)&value);
	LOG_D("CAN Baud Rate 0x%x\n\r", value);
	*data_ptr++ = (char)value;
	size++;

	eeprom_read(EEPROM_IO_ADDRESS_ADDR, (u8 *)&value);
	LOG_D("I/O Address 0x%x\n\r", value);
	*data_ptr++ = (char)value;
	size++;

	/*
	 * The Node Descrition is limited to being a 30 Byte string
	 * including the null terminator!
	 */
	length = 50;
	rc = eeprom_str_read(EEPROM_NODE_DESCRIPTION_ADDR, buffer, &length);
	if(rc != SUCCESS) {
		LOG_E("Failed to read the node description\n\r");
		return(rc);
	}
	LOG_D("Description: String Length %d string '%s'\n\r", length, buffer);

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
		LOG_E("Oops Out of space. Size %d\n\r", size);
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
	LOG_D("Node Config info length %d\n\r", size);
	return (SUCCESS);
}
#endif // CAN_NODE_OS

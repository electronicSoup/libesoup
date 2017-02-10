/**
 *
 * \file es_lib/can/dcncp/cinnamonbun_info.c
 *
 * Functions for retrieving the CinnamonBun Info Strings
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#include "system.h"
#define DEBUG_FILE
#include "es_lib/utils/flash.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/can/dcncp/cinnamonbun_info.h"
#include "es_lib/firmware/firmware.h"
#include "es_lib/utils/eeprom.h"

#define TAG "CB_INFO"

result_t cb_get_hardware_info(uint8_t *data, uint16_t *data_len)
{
	uint8_t       buffer[200];
	uint8_t      *data_ptr;
	uint16_t      size;
	uint16_t      length;
	uint16_t      loop;
	result_t rc;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "cb_get_hardare_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) HW_MANUFACTURER_24_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read HW Manufacturer\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Hw Manufacturer read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_MODEL_24_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to ready HW Model\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "HW Model read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read HW Description\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "HW Description read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) HW_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the HW Verson\n\r");
#endif
		return(rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "HW Version read as %s, lenght %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the HW URI\n\r");
#endif
		return(rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "HW URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Hardware info length %d\n\r", size);
#endif
	return (SUCCESS);
}

result_t cb_get_boot_info(uint8_t *data, uint16_t *data_len)
{
	uint8_t       buffer[200];
	uint8_t      *data_ptr;
	uint16_t      size;
	uint16_t      length;
	uint16_t      loop;
	result_t rc;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "cb_get_boot_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	length = 40;

	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read HW Manufacturer\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Boot Author read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to ready HW Model\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Boot Description read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) BOOT_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the HW Verson\n\r");
#endif
		return(rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Boot Version read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the HW URI\n\r");
#endif
		return(rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Boot URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Boot info length %d\n\r", size);
#endif
	return (SUCCESS);
}

result_t cb_get_firmware_info(uint8_t *data, uint16_t *data_len)
{
	uint8_t       buffer[200];
	uint8_t      *data_ptr;
	uint16_t      size;
	uint16_t      length;
	uint16_t      loop;
	result_t rc;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "cb_get_firmware_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	length = 40;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read Firmware Author\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Firmware Author read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to ready FIrmware Desc\n\r");
#endif
		return (rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Firmware Description read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) FIRMWARE_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the Firmware Verson\n\r");
#endif
		return(rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Firmware Version read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_URL_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the HW URI\n\r");
#endif
		return(rc);
	}

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Firmware URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Boot info length %d\n\r", size);
#endif
	return (SUCCESS);
}

result_t cb_get_application_info(uint8_t *data, uint16_t *data_len)
{
	uint8_t       buffer[154];
	uint8_t      *data_ptr;
	uint16_t      size;
	uint16_t      length;
	uint16_t      loop;
	result_t rc;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "cb_get_application_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the App Author\n\r");
#endif
		return (rc);
	}
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Application Author read as %s, length %d\n\r", buffer, length);
#endif

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_SOFTWARE_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the App Software\n\r");
#endif
		return (rc);
	}
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "App Software read as %s, length %d\n\r", buffer, length);
#endif

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_VERSION_10_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the App Version\n\r");
#endif
		return (rc);
	}
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "App Version read as %s, length %d\n\r", buffer, length);
#endif

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the App URI\n\r");
#endif
		return(rc);
	}
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "App URI read as %s, length %d\n\r", buffer, length);
#endif

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Application info length %d\n\r", size);
#endif
	return (SUCCESS);
}

#ifdef SYS_CAN_NODE_OS
result_t cb_get_node_config_info(uint8_t *data, uint16_t *data_len)
{
	uint8_t       buffer[200];
	uint8_t      *data_ptr;
	uint16_t      size;
	uint16_t      length;
	uint16_t      loop;
	uint8_t       value;
	result_t rc;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "cb_get_node_config_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	eeprom_read(EEPROM_NODE_ADDRESS, (uint8_t *)&value);
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Layer 3 Node Address 0x%x\n\r", value);
#endif
	*data_ptr++ = (char)value;
	size++;

	eeprom_read(EEPROM_CAN_BAUD_RATE_ADDR, (uint8_t *)&value);
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "CAN Baud Rate 0x%x\n\r", value);
#endif
	*data_ptr++ = (char)value;
	size++;

	eeprom_read(EEPROM_IO_ADDRESS_ADDR, (uint8_t *)&value);
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "I/O Address 0x%x\n\r", value);
#endif
	*data_ptr++ = (char)value;
	size++;

	/*
	 * The Node Descrition is limited to being a 30 Byte string
	 * including the null terminator!
	 */
	length = 50;
	rc = eeprom_str_read(EEPROM_NODE_DESCRIPTION_ADDR, buffer, &length);
	if(rc != SUCCESS) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Failed to read the node description\n\r");
#endif
		return(rc);
	}
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Description: String Length %d string '%s'\n\r", length, buffer);
#endif

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "Node Config info length %d\n\r", size);
#endif
	return (SUCCESS);
}
#endif // SYS_CAN_NODE_OS

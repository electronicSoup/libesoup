/**
 *
 * \file libesoup/board/cd-PIC24FJ256GB106/cinnamonbun_info.c
 *
 * Functions for retrieving the board's Info Strings
 *
 * Copyright 2017 2018 electronicSoup Limited
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

#if (defined(SYS_BOARD_INFO) && (defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)))

//
// Hardware Info
//
#define HARDWARE_INFO_BASE 0x200
__prog__ char hardware_manufacturer[24] __attribute__ ((space(prog),address(HARDWARE_INFO_BASE))) = "electronicSoup";
__prog__ char hardware_model[24]        __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24))) = "Cinnamon Bun";
__prog__ char hardware_description[50]  __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24))) = "CAN Bus Node dev Platform";
__prog__ char hardware_version[10]      __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50))) = "1.0.0";
__prog__ char hardware_uri[50]          __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10))) = "http://www.electronicsoup.com/products/";
//
// Bootloader Info
//
__prog__ char bootcode_author[40]       __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50))) = "electronicSoup";
__prog__ char bootcode_description[50]  __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40))) = "Android Bootloader";
__prog__ char bootcode_version[10]      __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40 + 50))) = "v0.8.0";
__prog__ char bootcode_uri[50]          __attribute__ ((space(prog),address(HARDWARE_INFO_BASE + 24 + 24 + 50 + 10 + 50 + 40 + 50 + 10))) = "http://www.electronicsoup.com/cb/bootloader";

/*
 * Firmware Info
 */
#define FIRMWARE_STRINGS_BASE 0x8800
__prog__ char firmware_author[40]       __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE))) = "";
__prog__ char firmware_description[50]  __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE + 40))) = "";
__prog__ char firmware_version[10]      __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE + 40 + 50))) = "";
__prog__ char firmware_uri[50]          __attribute__ ((space(prog),address(FIRMWARE_STRINGS_BASE + 40 + 50 + 10))) = "";


#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char * TAG = "CB_INFO";
#include "libesoup/logger/serial_log.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif  // SYS_SERIAL_LOGGING

#ifndef SYS_FLASH_RW
#error libesoup_config.h file should define SYS_FLASH_RW Required by Board Info (see libesoup/examples/libesoup_config.h)
#endif

#ifndef SYS_EEPROM
#error libesoup_config.h file should define SYS_EEPROM Required by Board Info (see libesoup/examples/libesoup_config.h)
#endif

#include "libesoup/hardware/flash.h"
#include "libesoup/boards/cb-PIC24FJ256GB106/cinnamonbun_info.h"
#include "libesoup/firmware/firmware.h"
#include "libesoup/hardware/eeprom.h"

result_t cb_get_hardware_info(uint8_t *data, uint16_t *data_len)
{
	uint8_t       buffer[200];
	uint8_t      *data_ptr;
	uint16_t      size;
	uint16_t      length;
	uint16_t      loop;
	result_t rc;

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("cb_get_hardare_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) HW_MANUFACTURER_24_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read HW Manufacturer\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Hw Manufacturer read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_MODEL_24_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to ready HW Model\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("HW Model read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read HW Description\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("HW Description read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) HW_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the HW Verson\n\r");
#endif
		return(rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("HW Version read as %s, lenght %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)HW_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the HW URI\n\r");
#endif
		return(rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("HW URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Hardware info length %d\n\r", size);
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("cb_get_boot_info()\n\r");
#endif
	data_ptr = data;
	size = 0;

	length = 40;

	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read HW Manufacturer\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Boot Author read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to ready HW Model\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_E("Boot Description read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) BOOT_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the HW Verson\n\r");
#endif
		return(rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Boot Version read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)BOOT_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the HW URI\n\r");
#endif
		return(rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Boot URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Boot info length %d\n\r", size);
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("cb_get_firmware_info()\n\r");
#endif
	data_ptr = data;
	size = 0;

	length = 40;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read Firmware Author\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Firmware Author read as %s length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_DESCRIPTION_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to ready FIrmware Desc\n\r");
#endif
		return (rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Firmware Description read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) FIRMWARE_VERSION_10_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the Firmware Verson\n\r");
#endif
		return(rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Firmware Version read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *)FIRMWARE_URL_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the HW URI\n\r");
#endif
		return(rc);
	}

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Firmware URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Boot info length %d\n\r", size);
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("cb_get_application_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_AUTHOR_40_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the App Author\n\r");
#endif
		return (rc);
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Application Author read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		log_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_SOFTWARE_50_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the App Software\n\r");
#endif
		return (rc);
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("App Software read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_VERSION_10_ADDRESS, &length);
	if (rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the App Version\n\r");
#endif
		return (rc);
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("App Version read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_URI_50_ADDRESS, &length);
	if(rc != SUCCESS) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the App URI\n\r");
#endif
		return(rc);
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("App URI read as %s, length %d\n\r", buffer, length);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Application info length %d\n\r", size);
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

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("cb_get_node_config_info()\n\r");
#endif

	data_ptr = data;
	size = 0;

	eeprom_read(EEPROM_NODE_ADDRESS, (uint8_t *)&value);
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Layer 3 Node Address 0x%x\n\r", value);
#endif
	*data_ptr++ = (char)value;
	size++;

	eeprom_read(EEPROM_CAN_BAUD_RATE_ADDR, (uint8_t *)&value);
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("CAN Baud Rate 0x%x\n\r", value);
#endif
	*data_ptr++ = (char)value;
	size++;

	eeprom_read(EEPROM_IO_ADDRESS_ADDR, (uint8_t *)&value);
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("I/O Address 0x%x\n\r", value);
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
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Failed to read the node description\n\r");
#endif
		return(rc);
	}
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Description: String Length %d string '%s'\n\r", length, buffer);
#endif
	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
		loop++;
	}

	if(size == *data_len) {
#if (defined(SYS_SERIAL_LOGGING) && (SYS_LOG_LEVEL <= LOG_ERROR))
		LOG_E("Oops Out of space. Size %d\n\r", size);
#endif
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("Node Config info length %d\n\r", size);
#endif
	return (SUCCESS);
}
#endif // SYS_CAN_NODE_OS

#endif // SYS_BOARD_INFO

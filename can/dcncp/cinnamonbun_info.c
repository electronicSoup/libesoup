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

#define TAG "CB_INFO"

result_t cb_get_hardware_info(char *data, u16 *data_len)
{
	char     buffer[200];
	char    *data_ptr;
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

result_t cb_get_application_info(char *data, u16 *data_len)
{
	char     buffer[154];
	char    *data_ptr;
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
		return (rc);
	}

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
	}

	if(size == *data_len) {
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_SOFTWARE_50_ADDRESS, &length);
	if (rc != SUCCESS) {
		return (rc);
	}

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
	}

	if(size == *data_len) {
		return(ERR_NO_RESOURCES);
	}

	length = 10;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_VERSION_10_ADDRESS, &length);
	if (rc != SUCCESS) {
		return (rc);
	}

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
	}

	if(size == *data_len) {
		return(ERR_NO_RESOURCES);
	}

	length = 50;
	rc = flash_strcpy(buffer, (__prog__ char *) APP_URI_50_ADDRESS, &length);

	loop = 0;
	while (loop <= length && size < *data_len) {
		*data_ptr++ = buffer[loop];
		size++;
	}

	if(size == *data_len) {
		return(ERR_NO_RESOURCES);
	}

	*data_len = size;
	LOG_D("Application info length %d\n\r", size);
	return (SUCCESS);
}

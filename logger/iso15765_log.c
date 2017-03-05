/**
 *
 * \file es_lib/logger/iso15765_log.c
 *
 * ISO15765 Logger Protocol Functionality
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
#include <stdio.h>
#include <string.h>
#include "system.h"
#include "es_lib/can/dcncp/dcncp_can.h"
#include "es_lib/can/es_can.h"

#define DEBUG_FILE TRUE
#include "es_lib/logger/serial_log.h"
#include "es_lib/logger/iso15765_log.h"

#define TAG "ISO15765_LOG"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL 
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif

/*
 * Network Logging
 */
#if defined(SYS_ISO15765)
#ifdef SYS_ISO15765_LOGGER
static void (*iso15765_logger_handler)(uint8_t, log_level_t, char*) = NULL;
#endif // SYS_ISO15765_LOGGER

static uint8_t iso15765_logger = FALSE;
static uint8_t iso15765_logger_address;
static log_level_t iso15765_logger_level = Error;
#endif

#if defined(SYS_ISO15765)
#ifdef SYS_ISO15765_LOGGER
static void iso15765_log_handler(iso15765_msg_t *message)
{
	log_level_t level;
	char        string[SYS_ISO15765_MAX_MSG];

	level = message->data[0];
	if ((level >= iso15765_logger_level) && iso15765_logger_handler) {
		strcpy(string, (char *) &(message->data[1]));
		iso15765_logger_handler(message->address, level, string);
	}
}
#endif // SYS_ISO15765_LOGGER
#endif // SYS_ISO15765

/*
 * Register this node on the Network as the logger
 */
#if defined(SYS_ISO15765)
#ifdef SYS_ISO15765_LOGGER
result_t iso15765_logger_register_as_logger(void (*handler)(uint8_t, log_level_t, char *), log_level_t level)
{
	iso15765_target_t target;

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso15765_log_reg_as_handler() level %x\n\r", level);
#endif
	if(iso15765_initialised()) {
		if(handler != NULL) {
			iso15765_logger_handler = handler;
			iso15765_logger_level = level;

			target.protocol = ISO15765_LOGGER_PROTOCOL_ID;
			target.handler = iso15765_log_handler;
			iso15765_dispatch_reg_handler(&target);

			return (dcncp_register_this_node_net_logger(level));
		} else {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "No handler given\n\r");
#endif
			return(ERR_BAD_INPUT_PARAMETER);
		}
	} else {
		return(ERR_NOT_READY);
	}
}
#endif // SYS_ISO15765_LOGGER
#endif // SYS_ISO15765

/*
 * Unregister this node as the Network Logger!
 */
#if defined(SYS_ISO15765)
#ifdef SYS_ISO15765_LOGGER
result_t iso15765_logger_unregister_as_logger(void)
{
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso15765_log_unreg_as_handler()\n\r");
#endif
	iso15765_logger = FALSE;
	iso15765_logger_handler = NULL;
	return(dcncp_unregister_this_node_net_logger());
}
#endif // SYS_ISO15765_LOGGER
#endif // SYS_ISO15765

#if defined(SYS_ISO15765)
void iso15765_log(log_level_t level, char *string)
{
	uint8_t loop;
//	uint8_t address;
	uint8_t data[SYS_ISO15765_MAX_MSG];

	iso15765_msg_t msg;

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "iso15765_log(0x%x, %s)\n\r", (uint16_t)level, string);
#endif

	if(iso15765_logger) {
		if(level <= iso15765_logger_level) {
			if(strlen((char *)string) < SYS_ISO15765_MAX_MSG - 2) {
				msg.address = iso15765_logger_address;
				msg.protocol = ISO15765_LOGGER_PROTOCOL_ID;
				data[0] = level;

				for(loop = 0; loop <= strlen(string); loop++) {
					data[loop + 1] = string[loop];
				}

				msg.size = loop + 1;
				msg.data = data;

				if(msg.size < SYS_ISO15765_MAX_MSG) {
					iso15765_tx_msg(&msg);
				} else {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "message size limit exceeded!\n\r");
#endif
				}
			}
		} else {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
			log_d(TAG, "ISO15765 logger not logging insifficient Level\n\r");
#endif
		}
	} else {
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		log_d(TAG, "no Logger Registered\n\r");
#endif
	}
}
#endif  // if defined(SYS_ISO15765)

/*
 * Another network node has registered as the system
 */
#if defined(SYS_ISO15765)
void iso15765_logger_register_remote(uint8_t address, log_level_t level)
{
	iso15765_logger = TRUE;
	iso15765_logger_address = address;
	iso15765_logger_level = level;
}
#endif // SYS_ISO15765

/*
 * Another network node has UN-registered as the system
 */
#if defined(SYS_ISO15765)
void iso15765_logger_unregister_remote(uint8_t address)
{
	if(iso15765_logger_address == address)
		iso15765_logger = FALSE;
}
#endif // SYS_ISO15765

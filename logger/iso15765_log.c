/**
 * @file libesoup/logger/iso15765_log.c
 *
 * @author John Whitmore
 *
 * @brief ISO15765 Logger Protocol Functionality
 *
 * Copyright 2017-2018 electronicSoup Limited
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
#include "libesoup_config.h"

#if defined(SYS_CAN_ISO15765_LOG)

#include "libesoup/comms/can/dcncp/dcncp_can.h"
#include "libesoup/comms/can/can.h"

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
#include "libesoup/logger/serial_log.h"

static const char *TAG = "ISO15765_LOG";

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL 
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif // SYS_SERIAL_LOGGING

#ifndef SYS_CAN_ISO15765
#error libesoup_config.h should define SYS_CAN_ISO15765 to utilise L3 Logging
#endif

/*
 * Network Logging
 */
#ifdef SYS_CAN_ISO15765_LOGGER
static void (*iso15765_logger_handler)(uint8_t, log_level_t, char*) = NULL;
#endif // SYS_ISO15765_LOGGER

static uint8_t iso15765_logger = FALSE;
static uint8_t iso15765_logger_address;
static uint8_t iso15765_logger_level = LOG_DEBUG;

#ifdef SYS_CAN_ISO15765_LOGGER
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
#endif // SYS_CAN_ISO15765_LOGGER

/*
 * Register this node on the Network as the logger
 */
#ifdef SYS_CAN_ISO15765_LOGGER
result_t iso15765_logger_register_as_logger(void (*handler)(uint8_t, log_level_t, char *), log_level_t level)
{
	iso15765_target_t target;

	LOG_D("iso15765_log_reg_as_handler() level %x\n\r", level);
	if(iso15765_initialised()) {
		if(handler != NULL) {
			iso15765_logger_handler = handler;
			iso15765_logger_level = level;

			target.protocol = ISO15765_LOGGER_PROTOCOL_ID;
			target.handler = iso15765_log_handler;
			iso15765_dispatch_reg_handler(&target);

			return (dcncp_register_this_node_net_logger(level));
		} else {
			LOG_E("No handler given\n\r");
			return(ERR_BAD_INPUT_PARAMETER);
		}
	} else {
		return(ERR_NOT_READY);
	}
}
#endif // SYS_CAN_ISO15765_LOGGER

/*
 * Unregister this node as the Network Logger!
 */
#ifdef SYS_CAN_ISO15765_LOGGER
result_t iso15765_logger_unregister_as_logger(void)
{
	LOG_D("iso15765_log_unreg_as_handler()\n\r");
	iso15765_logger = FALSE;
	iso15765_logger_handler = NULL;
	return(dcncp_unregister_this_node_net_logger());
}
#endif // SYS_CAN_ISO15765_LOGGER

void iso15765_log(uint8_t level, char *string)
{
	uint8_t loop;
//	uint8_t address;
	uint8_t data[SYS_CAN_ISO15765_MAX_MSG];

	iso15765_msg_t msg;

	LOG_D("iso15765_log(0x%x, %s)\n\r", (uint16_t)level, string);

	if(iso15765_logger) {
		if(level <= iso15765_logger_level) {
			if(strlen((char *)string) < SYS_CAN_ISO15765_MAX_MSG - 2) {
				msg.address = iso15765_logger_address;
				msg.protocol = CAN_ISO15765_LOGGER_PROTOCOL_ID;
				data[0] = level;

				for(loop = 0; loop <= strlen(string); loop++) {
					data[loop + 1] = string[loop];
				}

				msg.size = loop + 1;
				msg.data = data;

				if(msg.size < SYS_CAN_ISO15765_MAX_MSG) {
					iso15765_tx_msg(&msg);
				} else {
					LOG_E("message size limit exceeded!\n\r");
				}
			}
		} else {
			LOG_D("ISO15765 logger not logging insifficient Level\n\r");
		}
	} else {
		LOG_D("no Logger Registered\n\r");
	}
}

/*
 * Another network node has registered as the system
 */
void iso15765_logger_register_remote(uint8_t address, uint8_t level)
{
	iso15765_logger = TRUE;
	iso15765_logger_address = address;
	iso15765_logger_level = level;
}

/*
 * Another network node has UN-registered as the system
 */
void iso15765_logger_unregister_remote(uint8_t address)
{
	if(iso15765_logger_address == address)
		iso15765_logger = FALSE;
}

#endif // SYS_CAN_ISO15765_LOG

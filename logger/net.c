/**
 *
 * \file es_can/logger/net.h
 *
 * Network Logger Protocol Functionality
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
#include <string.h>
#include "system.h"
#include "es_lib/dcncp/l2_dcncp.h"
#include "es_lib/can/es_can.h"

#define DEBUG_FILE
#include "es_lib/logger/serial.h"
#include "es_lib/logger/net.h"

#if defined(MCP)
#include "es_lib/utils/utils.h"
#endif

#if DEBUG_LEVEL < NO_LOGGING
#define TAG "NET_LOG"
#endif

/*
 * Network Logging
 */
#if defined(CAN_LAYER_3)
static void (*net_logger_handler)(u8, log_level_t, char*) = NULL;

static bool net_logger = FALSE;
static bool net_logger_local = FALSE;
static u8 net_logger_address;
static log_level_t net_logger_level = Error;
#endif

#if defined(CAN_LAYER_3)
static void l3NetLogHandler(l3_can_msg_t *message)
{
	log_level_t level;
	char        string[L3_CAN_MAX_MSG];

	if(net_logger && net_logger_local) {
		level = message->data[0];
		if((level >= net_logger_level) && net_logger_handler) {
			strcpy(string, (char *) &(message->data[1]));
			net_logger_handler(message->address, level, string);
		}
	} else {
		DEBUG_D("No Handler\n\r");
	}
}
#endif

/*
 * Register this node on the Network as the logger
 */
#if defined(CAN_LAYER_3)
result_t net_logger_local_register(void (*handler)(u8, log_level_t, char *), log_level_t level)
{
	DEBUG_D("net_log_reg_as_handler() level %x\n\r", level);
	if(l3_initialised()) {
		net_logger = TRUE;
		net_logger_local = TRUE;
		net_logger_handler = handler;
		net_logger_level = level;

		l3_register_handler(NET_LOG_L3_ID, l3NetLogHandler);

		return(register_this_node_net_logger(level));
	} else {
		return(ERR_L3_UNINITIALISED);
	}
}
#endif

/*
 * Unregister this node as the Network Logger!
 */
#if defined(CAN_LAYER_3)
result_t net_logger_local_cancel(void)
{
	DEBUG_D("net_log_unreg_as_handler()\n\r");
	if(net_logger_local) {
		net_logger_local = FALSE;
		net_logger = FALSE;
		net_logger_handler = NULL;
	}
	return(unregister_this_node_net_logger());
}
#endif

#if defined(CAN_LAYER_3)
void net_log(log_level_t level, char *string)
{
	u8 loop;
	u8 address;
	u8 data[L3_CAN_MAX_MSG];

	l3_can_msg_t msg;

	DEBUG_D("net_log(0x%x, %s)\n\r", (u16)level, string);

	if(net_logger) {
		if(net_logger_local && net_logger_handler) {
			get_l3_node_address(&address);
			net_logger_handler(address, level, string);
			DEBUG_D("Local Net Logger\n\r");
		} else if(level <= net_logger_level) {
			if(strlen((char *)string) < L3_CAN_MAX_MSG - 2) {
				msg.address = net_logger_address;
				msg.protocol = NET_LOG_L3_ID;
				data[0] = level;

				for(loop = 0; loop <= strlen(string); loop++) {
					data[loop + 1] = string[loop];
				}

				msg.size = loop + 1;
				msg.data = data;

				if(msg.size < L3_CAN_MAX_MSG) {
					l3_tx_msg(&msg);
				} else {
					DEBUG_E("Layer 3 message size limit exceeded!\n\r");
				}
			}
		} else {
			DEBUG_D("Net logger not logging insifficient Level\n\r");
		}
	} else {
		DEBUG_D("no Logger Registered\n\r");
	}
}
#endif

/*
 * Another network node has registered as the system
 */
#if defined(CAN_LAYER_3)
void net_logger_foreign_register(u8 address, log_level_t level)
{
	net_logger = TRUE;
	net_logger_local = FALSE;
	net_logger_address = address;
	net_logger_level = level;
}
#endif

/*
 * Another network node has UN-registered as the system
 */
#if defined(CAN_LAYER_3)
void net_logger_foreign_cancel(u8 address)
{
	if(net_logger_address == address)
		net_logger = FALSE;
}
#endif

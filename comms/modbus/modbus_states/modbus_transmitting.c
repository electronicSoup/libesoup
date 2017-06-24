/**
 *
 * \file libesoup/modbus/modbus_transmitting.c
 *
 * This file contains code for Modbus transmission
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
#define TAG "MODBUS_TRANSMITTING"
#define DEBUG_FILE TRUE

#include "libesoup_config.h"

#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/modbus/modbus.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif

extern struct modbus_state modbus_state;

static void tx_finished(void *);

void set_modbus_transmitting_state(struct modbus_channel *channel)
{
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_modbus_transmitting_state()\n\r");
#endif
	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = NULL;
	channel->transmit = NULL;
	channel->modbus_tx_finished = tx_finished;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;
}

void tx_finished(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;

#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "tx_finished()\n\r");
#endif
	set_modbus_awaiting_response_state(channel);
}

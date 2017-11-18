/**
 *
 * \file libesoup/modbus/modbus_starting.c
 *
 * This file contains code for Modbus starting state
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
#define TAG "MODBUS_STARTING"
#define DEBUG_FILE TRUE

#include "libesoup_config.h"

#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void *);

void set_modbus_starting_state(struct modbus_channel *channel)
{
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_INFO))
	log_i(TAG, "set_modbus_starting_state(channel %d)\n\r", channel->uart->uart);
#endif
#endif // SYS_SERIAL_LOGGING
	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->modbus_tx_finished = NULL;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;

	start_35_timer(channel);
}

static void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;
        
        if(channel->uart) {
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                log_d(TAG, "process_timer_35_expiry(channel %d)\n\r", channel->uart->uart);
#endif
#endif // SYS_SERIAL_LOGGING
                set_modbus_idle_state(channel);
        } else {
#ifdef SYS_SERIAL_LOGGING
#if ((DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
                log_d(TAG, "process_timer_35_expiry() No Uart\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
        }
}

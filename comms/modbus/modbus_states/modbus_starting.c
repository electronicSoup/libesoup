/**
 *
 * @file libesoup/comms/modbus/modbus_states/modbus_starting.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus starting state
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
#include "libesoup_config.h"

#ifdef SYS_MODBUS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MODBUS_STARTING";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void *);

void set_modbus_starting_state(struct modbus_channel *channel)
{
	LOG_I("set_modbus_starting_state(channel %d)\n\r", channel->uart->uindex);
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
                LOG_D("process_timer_35_expiry(channel %d)\n\r", channel->uart->uindex);
                set_modbus_idle_state(channel);
        } else {
                LOG_D("process_timer_35_expiry() No Uart\n\r");
        }
}

#endif // SYS_MODBUS

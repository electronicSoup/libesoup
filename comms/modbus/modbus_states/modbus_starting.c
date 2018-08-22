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

#include "libesoup/comms/modbus/modbus_private.h"

static void process_timer_35_expiry(struct modbus_channel *chan)
{
        if(chan->app_data) {
//                LOG_D("process_timer_35_expiry(channel %d)\n\r", chan->uart->uindex);
                set_modbus_idle_state(chan);
        } else {
                LOG_D("process_timer_35_expiry() No Uart\n\r");
        }
}

result_t set_modbus_starting_state(struct modbus_channel *chan)
{
	chan->state                    = mb_starting;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = process_timer_35_expiry;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	if(chan->app_data->idle_state_callback) {
		chan->app_data->idle_state_callback(chan->app_data->channel_id, FALSE);
	}
	return(start_35_timer(chan));
}

#endif // SYS_MODBUS

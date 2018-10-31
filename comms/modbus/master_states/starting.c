/**
 * @file libesoup/comms/modbus/master_states/starting.c
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

#if defined(SYS_MODBUS) && defined(SYS_MODBUS_MASTER)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MB_M_STARTING";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void process_timer_35_expiry(struct modbus_channel *chan)
{
        if(chan->app_data) {
                set_master_idle_state(chan);
        } else {
                LOG_E("No App Data\n\r");
        }
}

result_t set_master_starting_state(struct modbus_channel *chan)
{
	chan->state                    = mb_m_starting;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = process_timer_35_expiry;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = NULL;
	chan->process_response         = NULL;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	if(chan->app_data->idle_state_callback) {
		chan->app_data->idle_state_callback(chan->app_data->channel_id, FALSE);
	}
	return(start_35_timer(chan));
}

#endif // SYS_MODBUS

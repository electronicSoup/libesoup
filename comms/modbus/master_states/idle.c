/**
 * @file libesoup/comms/modbus/master_states/idle.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus Master idle state
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
static const char *TAG = "MB_M_IDLE";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static result_t transmit(struct modbus_channel *chan, uint8_t *data, uint16_t len, modbus_response_function callback)
{
	/*
	 * The response timeout timer is started when the transmission is
	 * completed in the modbus_awaiting_response state.
	 */
	if (!callback || !data || len == 0) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}
	chan->tx_modbus_address      = data[0];
	chan->process_response       = callback;
	
	set_master_transmitting_state(chan);
	return(modbus_tx_data(chan, data, len));
}

result_t set_master_idle_state(struct modbus_channel *chan)
{
//	LOG_D("set_modbus_idle_state(channel %d)\n\r", chan->modbus_index);
	chan->state                    = mb_m_idle;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = transmit;
        chan->rx_write_index           = 0;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	if(chan->app_data->idle_state_callback) {
		chan->app_data->idle_state_callback(chan->app_data->channel_id, TRUE);
	}
	
	return(SUCCESS);
}

#endif // SYS_MODBUS

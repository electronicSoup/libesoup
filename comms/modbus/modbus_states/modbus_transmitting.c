/**
 *
 * @file libesoup/comms/modbus/modbus_states/modbus_transmitting.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus transmission
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
static const char *TAG = "MODBUS_Tx";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void tx_finished(struct modbus_channel *chan);

result_t set_modbus_transmitting_state(struct modbus_channel *chan)
{
	LOG_D("set_modbus_transmitting_state()\n\r");

	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = tx_finished;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;
	
	if(chan->idle_callback) {
		chan->idle_callback(chan->modbus_index, FALSE);
	}
	
	return(SUCCESS);
}

void tx_finished(struct modbus_channel *chan)
{
        LOG_D("tx_finished()\n\r");
	set_modbus_awaiting_response_state(chan);
}

#endif // SYS_MODBUS

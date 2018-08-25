/**
 * @file libesoup/comms/modbus/modbus_states/modbus_processing_request.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus processing request state. This state is used by
 * a slave channel
 *
 * Copyright 2018 electronicSoup Limited
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

#if defined(SYS_MODBUS) && defined(SYS_MODBUS_SLAVE)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MODBUS_PROCESSING";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static result_t transmit(struct modbus_channel *chan, uint8_t *data, uint16_t len, modbus_response_function callback)
{
	if (!data || len == 0) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	set_slave_transmitting_state(chan);
	return(modbus_tx_data(chan, data, len));
}

result_t set_slave_processing_request_state(struct modbus_channel *chan)
{
	chan->state                    = mb_s_processing_request;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = transmit;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	return(SUCCESS);
}

#endif // SYS_MODBUS

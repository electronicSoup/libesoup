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

#if defined(SYS_MODBUS) && defined(SYS_MODBUS_SLAVE)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MB_S_Tx";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void tx_finished(struct modbus_channel *chan)
{
	/*
	 * After transmission the Slave node returns to the idle state
	 * whereas the master awaits a response to it's request.
	 */
	set_slave_idle_state(chan);
}

result_t set_slave_transmitting_state(struct modbus_channel *chan)
{
	chan->state                    = mb_s_transmitting;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = NULL;
        chan->rx_write_index           = 0;
	chan->modbus_tx_finished       = tx_finished;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	return(SUCCESS);
}

#endif // SYS_MODBUS

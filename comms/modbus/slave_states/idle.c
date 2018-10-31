/**
 * @file libesoup/comms/modbus/slave_states/idle.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus idle state
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
static const char *TAG = "MODBUS_IDLE";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void process_rx_character(struct modbus_channel *chan, uint8_t ch)
{
	result_t  rc;
	
	rc = set_slave_receiving_state(chan);
	chan->process_rx_character(chan, ch);
}

result_t set_slave_idle_state(struct modbus_channel *chan)
{
	chan->state                    = mb_s_idle;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = NULL;
        chan->rx_write_index           = 0;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = process_rx_character;
	chan->process_response_timeout = NULL;

	return(SUCCESS);
}

#endif // SYS_MODBUS

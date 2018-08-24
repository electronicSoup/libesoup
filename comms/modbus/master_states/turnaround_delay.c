/**
 * @file libesoup/comms/modbus/master_states/modbus_turn_around.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus state after transmitting a broadcast message
 *        turn around delay before next transmission.
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

#if defined(SYS_MODBUS) && defined(SYS_MODBUS_MASTER)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MODBUS_TURNAROUND";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void turnaround_expiry_fn(timer_id timer, union sigval data)
{
	result_t rc;
	struct modbus_channel *chan = (struct modbus_channel *)data.sival_ptr;

	LOG_D("%s\n\r", __func__);

	chan->turnaround_timer = BAD_TIMER_ID;

	rc = set_master_starting_state(chan);
	if (rc < 0) {
		LOG_E("Failed to set idle state\n\r");
	}
}

static result_t start_turnaround_timer(struct modbus_channel *chan)
{
	result_t          rc;
	struct timer_req  request;

	LOG_D("%s\n\r", __func__);

	if (chan->turnaround_timer != BAD_TIMER_ID) {
		return(-ERR_GENERAL_ERROR);
	}
	request.period.units    = SYS_MODBUS_TURNAROUND_TIMEOUT_UNITS;
	request.period.duration = SYS_MODBUS_TURNAROUND_TIMEOUT_DURATION;
	request.type            = single_shot;
	request.exp_fn          = turnaround_expiry_fn;
	request.data.sival_ptr  = chan;

	rc = sw_timer_start(&request);
	RC_CHECK

	chan->resp_timer = rc;

	return(SUCCESS);
}

result_t set_modbus_turnaround_state(struct modbus_channel *chan)
{
	LOG_D("set_modbus_turnaround_state()\n\r");
	chan->state                    = mb_m_turnaround;
	chan->rx_write_index           = 0;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	if(chan->app_data->idle_state_callback) {
		chan->app_data->idle_state_callback(chan->app_data->channel_id, FALSE);
	}

	return(start_turnaround_timer(chan));
}

#endif // SYS_MODBUS

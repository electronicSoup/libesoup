/**
 * @file libesoup/comms/modbus/modbus_states/modbus_turn_around.c
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

#ifdef SYS_MODBUS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MODBUS_TURNAROUND";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

extern struct modbus_channel channels[SYS_MODBUS_NUM_CHANNELS];
extern struct modbus_state modbus_state;

static void turnaround_expiry_fn(timer_id timer, union sigval data)
{
	result_t rc;
	
	LOG_D("%s\n\r", __func__);

	channels[data.sival_int].turnaround_timer = BAD_TIMER_ID;

	rc = set_modbus_idle_state(&channels[data.sival_int]);
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
	request.data.sival_int  = chan->modbus_index;

	rc = sw_timer_start(&request);
	RC_CHECK

	chan->resp_timer = rc;

	return(SUCCESS);
}

result_t set_modbus_turnaround_state(struct modbus_channel *chan)
{
	LOG_D("set_modbus_turnaround_state()\n\r");
	chan->rx_write_index           = 0;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = NULL;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = NULL;
	chan->process_response_timeout = NULL;

	if(chan->idle_callback) {
		chan->idle_callback(chan->modbus_index, FALSE);
	}

	return(start_turnaround_timer(chan));
}

#endif // SYS_MODBUS

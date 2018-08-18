/**
 * @file libesoup/comms/modbus/modbus_states/modbus_awaiting_response.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus awaiting response state
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
static const char *TAG = "MODBUS_AWAITING_RESPONSE";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void process_timer_35_expiry(struct modbus_channel *chan);
static void process_rx_character(struct modbus_channel *chan, uint8_t ch);
static void process_response_timeout(struct modbus_channel *chan);

extern struct modbus_channel channels[SYS_MODBUS_NUM_CHANNELS];

static void resp_timeout_expiry_fn(timer_id timer, union sigval data)
{
	LOG_D("%s\n\r", __func__);
	channels[data.sival_int].resp_timer = BAD_TIMER_ID;

	if (channels[data.sival_int].process_response_timeout) {
		channels[data.sival_int].process_response_timeout(&channels[data.sival_int]);
	} else {
		LOG_E("Response Timout in unknown state\n\r");
	}
}

static result_t start_response_timer(struct modbus_channel *chan)
{
	result_t          rc;
	struct timer_req  request;

	if (chan->resp_timer != BAD_TIMER_ID) {
		return(-ERR_GENERAL_ERROR);
	}
	request.period.units    = SYS_MODBUS_RESPONSE_TIMEOUT_UNITS;
	request.period.duration = SYS_MODBUS_RESPONSE_TIMEOUT_DURATION;
	request.type            = single_shot;
	request.exp_fn          = resp_timeout_expiry_fn;
	request.data.sival_int  = chan->modbus_index;

//	if (channel->address == 0) {
//		ticks = SYS_MODBUS_RESPONSE_BROADCAST_TIMEOUT;
//	} else {
//		ticks = SYS_MODBUS_RESPONSE_TIMEOUT;
//	}

	rc = sw_timer_start(&request);
	RC_CHECK

	chan->resp_timer = rc;

	return(SUCCESS);
}

result_t set_modbus_awaiting_response_state(struct modbus_channel *chan)
{
	LOG_D("set_modbus_awaiting_response_state()\n\r");

	chan->rx_write_index           = 0;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = process_timer_35_expiry;
	chan->transmit                 = NULL;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = process_rx_character;
	chan->process_response_timeout = process_response_timeout;

	if (chan->idle_callback) {
		chan->idle_callback(chan->modbus_index, FALSE);
	}

	return(start_response_timer(chan));
}

static result_t cancel_response_timer(struct modbus_channel *chan)
{
	if (chan->resp_timer != BAD_TIMER_ID) {
		return(sw_timer_cancel(&(chan->resp_timer)));
	}
	return(SUCCESS);
}

void process_timer_35_expiry(struct modbus_channel *chan)
{
	uint8_t  start_index;

	LOG_D("process_timer_35_expiry() chan %d msg length %d\n\r", chan->modbus_index, chan->rx_write_index);
	if(chan->rx_write_index > 2) {
		if(chan->rx_buffer[0] == chan->address) {
			start_index = 0;
		} else if (chan->rx_buffer[1] == chan->address) {
			start_index = 1;
		} else {
			LOG_D("message from wrong address chan Address 0x%x\n\r", chan->address);
			LOG_D("chan->rx_buffer[0] = 0x%x\n\r", chan->rx_buffer[0]);
			LOG_D("chan->rx_buffer[1] = 0x%x\n\r", chan->rx_buffer[1]);
			set_modbus_idle_state(chan);
			return;
		}

		if (crc_check(&(chan->rx_buffer[start_index]), chan->rx_write_index - start_index)) {
			/*
			 * Response Good
			 * Subtract 2 for the CRC
			 */
			chan->process_response(chan->modbus_index, &(chan->rx_buffer[start_index]), chan->rx_write_index - (start_index + 2));
		} else {
			LOG_D("Bad CRC!\n\r");
		}
	} else {
		LOG_D("Resp short\n\r");
	}

	set_modbus_idle_state(chan);
}

void process_rx_character(struct modbus_channel *chan, uint8_t ch)
{
	if ((chan->rx_write_index == 0) && (ch == 0x00)) {
		LOG_E("0/00\n\r")
		return;
	}
	cancel_response_timer(chan);
	start_35_timer(chan);

	chan->rx_buffer[chan->rx_write_index++] = ch;

	if (chan->rx_write_index == SYS_MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

static void process_response_timeout(struct modbus_channel *chan)
{
	LOG_D("process_response_timeout()\n\r");
	set_modbus_starting_state(chan);
	if(chan->process_response) {
		chan->process_response(chan->modbus_index, NULL, 0);
	} else {
		LOG_E("No response Function\n\r");
	}
}

#endif // SYS_MODBUS

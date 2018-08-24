/**
 * @file libesoup/comms/modbus/modbus_states/modbus_receiving.c
 *
 * @author John Whitmore
 *
 * @brief Code for Modbus receiving state
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
static const char *TAG = "MODBUS_RX";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

static void process_rx_character(struct modbus_channel *chan, uint8_t ch)
{
	start_35_timer(chan);

	chan->rx_buffer[chan->rx_write_index++] = ch;

	if (chan->rx_write_index == SYS_MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

static void process_timer_35_expiry(struct modbus_channel *chan)
{
	uint8_t  start_index;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	uint16_t loop;
#endif

	LOG_D("process_timer_35_expiry() channel %d msg length %d\n\r", chan->app_data->channel_id, chan->rx_write_index);

	/*
	 * If there's no handler forget the frame
	 */
	if(!chan->app_data || !chan->app_data->unsolicited_frame_handler) {
		chan->rx_write_index = 0;
		LOG_W("No Handler\n\r");
		set_modbus_idle_state(chan);
		return;
	}

	/*
	 * Check if there's a valid Modbus frame starting on index 0
	 */
        if (crc_check(&(chan->rx_buffer[0]), chan->rx_write_index)) {
		start_index = 0;
        } else if (crc_check(&(chan->rx_buffer[1]), chan->rx_write_index - 1)) {
		start_index = 1;
        } else {
		/*
		 * No valid frame found
		 */
		chan->rx_write_index = 0;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
		LOG_D("Message bad!\n\r");
		for(loop = 0; loop < chan->rx_write_index; loop++) {
			LOG_D("Char %d - 0x%x\n\r", loop, chan->rx_buffer[loop]);
		}
#endif
		set_modbus_idle_state(chan);
		return;
	}
	if(chan->rx_buffer[start_index] == chan->app_data->address) {
		chan->app_data->unsolicited_frame_handler(chan->app_data->channel_id, &(chan->rx_buffer[start_index + 1]), chan->rx_write_index - (start_index + 3));
	} else {
		LOG_I("Not for this address\n\r");
	}
        chan->rx_write_index = 0;
	set_modbus_idle_state(chan);
}

result_t set_modbus_receiving_state(struct modbus_channel *chan)
{
	chan->state                    = mb_receiving;
	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = process_timer_35_expiry;
	chan->transmit                 = NULL;
        chan->rx_write_index           = 0;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = process_rx_character;
	chan->process_response_timeout = NULL;

	if(chan->app_data->idle_state_callback) {
		chan->app_data->idle_state_callback(chan->app_data->channel_id, FALSE);
	}
	
	return(SUCCESS);
}

#endif // SYS_MODBUS

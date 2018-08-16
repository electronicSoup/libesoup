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

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void *);
static void process_rx_character(struct modbus_channel *channel, uint8_t ch);
static void process_response_timeout(struct modbus_channel *channel);

void set_modbus_awaiting_response_state(struct modbus_channel *channel)
{
        result_t rc;
        
	LOG_D("set_modbus_awaiting_response_state()\n\r");
	channel->rx_write_index = 0;

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->modbus_tx_finished = NULL;
	channel->process_rx_character = process_rx_character;
	channel->process_response_timeout = process_response_timeout;

	rc = start_response_timer(channel);
        
        if(rc != 0) {
                LOG_E("Failed to start response timer\n\r");
        }
}

void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;
        
	uint8_t  start_index;
//	uint16_t loop;

	LOG_D("process_timer_35_expiry()\n\r");
	set_modbus_idle_state(channel);

	LOG_D("process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uindex, channel->rx_write_index);
	if(channel->rx_write_index > 2) {
		if(channel->rx_buffer[0] == channel->address) {
			start_index = 0;
		} else if (channel->rx_buffer[1] == channel->address) {
			start_index = 1;
		} else {
			LOG_D("message from wrong address channel Address 0x%x\n\r", channel->address);
			LOG_D("channel->rx_buffer[0] = 0x%x\n\r", channel->rx_buffer[0]);
			LOG_D("channel->rx_buffer[1] = 0x%x\n\r", channel->rx_buffer[1]);
			return;
		}

		if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
			/*
			 * Response Good
			 * Subtract 2 for the CRC
			 */
			channel->process_response(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
		} else {
			LOG_D("Message bad!\n\r");
			channel->process_response(NULL, 0, channel->response_callback_data);
		}
	} else {
		LOG_D("Message too short\n\r");
		channel->process_response(NULL, 0, channel->response_callback_data);
	}
}

void process_rx_character(struct modbus_channel *channel, uint8_t ch)
{
	if ((channel->rx_write_index == 0) && (ch == 0x00)) {
		return;
	}
	cancel_response_timer(channel);
	start_35_timer(channel);

	channel->rx_buffer[channel->rx_write_index++] = ch;

	if (channel->rx_write_index == SYS_MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

static void process_response_timeout(struct modbus_channel *channel)
{
	LOG_D("process_response_timeout()\n\r");
	set_modbus_starting_state(channel);
	if(channel->process_response) {
		channel->process_response(NULL, 0, channel->response_callback_data);
	} else {
		LOG_E("No response Function\n\r");
	}
}

#endif // SYS_MODBUS

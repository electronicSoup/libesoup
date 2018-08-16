/**
 *
 * @file libesoup/comms/modbus/modbus_states/modbus_idle.c
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

#ifdef SYS_MODBUS

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "MODBUS_IDLE";
#include "libesoup/logger/serial_log.h"
#endif

#include "libesoup/comms/modbus/modbus_private.h"

/*
 * Check required libesoup_config.h defines are found
 */

extern struct modbus_state modbus_state;

static void transmit(struct modbus_channel *channel, uint8_t *data, uint16_t len, modbus_response_function fn, void* callback_data);
static void process_timer_35_expiry(void *data);
static void process_rx_character(struct modbus_channel *channel, uint8_t ch);

result_t set_modbus_idle_state(struct modbus_channel *chan)
{
	LOG_D("set_modbus_idle_state(channel %d)\n\r", chan->uart->uindex);

	chan->process_timer_15_expiry  = NULL;
	chan->process_timer_35_expiry  = process_timer_35_expiry;
	chan->transmit                 = transmit;
        chan->rx_write_index           = 0;
	chan->modbus_tx_finished       = NULL;
	chan->process_rx_character     = process_rx_character;
	chan->process_response_timeout = NULL;

	if(chan->idle_callback) {
		chan->idle_callback(chan->modbus_index);
	}
	
	return(SUCCESS);
}

void transmit(struct modbus_channel *channel, uint8_t *data, uint16_t len, modbus_response_function fn, void *callback_data)
{
	LOG_D("Modbus Idle state Transmit(%d)\n\r", channel->uart->uindex);
	/*
	 * The response timeout timer is started when the transmission is
	 * completed in the modbus_awaiting_response state.
	 */
	channel->address = data[0];
	channel->process_response = fn;
	channel->response_callback_data = callback_data;
	set_modbus_transmitting_state(channel);
	modbus_tx_data(channel, data, len);
}

void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;

	uint8_t  start_index;
#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
	uint16_t loop;
#endif

	LOG_D("process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uindex, channel->rx_write_index);
        start_index = 0;
        if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
                /*
                 * Response Good
                 * Subtract 2 for the CRC
                 */
                LOG_D("Message Good! Start at 0\n\r");
                if(channel->process_unsolicited_msg) {
                        channel->process_unsolicited_msg(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
                }
                channel->rx_write_index = 0;
                return;
        }

        start_index = 1;
        if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
                /*
                 * Response Good
                 * Subtract 2 for the CRC
                 */
                LOG_D("Message Good! Start at 1\n\r");
                if(channel->process_unsolicited_msg) {
                        channel->process_unsolicited_msg(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2), channel->response_callback_data);
                }
                channel->rx_write_index = 0;
                return;
        }

#if (defined(SYS_SERIAL_LOGGING) && defined(DEBUG_FILE) && (SYS_LOG_LEVEL <= LOG_DEBUG))
        LOG_D("Message bad!\n\r");
	for(loop = 0; loop < channel->rx_write_index; loop++) {
		LOG_D("Char %d - 0x%x\n\r", loop, channel->rx_buffer[loop]);
	}
#endif
        if(channel->process_unsolicited_msg) {
                channel->process_unsolicited_msg(NULL, 0, channel->response_callback_data);
        }
        channel->rx_write_index = 0;
}

void process_rx_character(struct modbus_channel *channel, uint8_t ch)
{
	if ((channel->rx_write_index == 0) && (ch == 0x00)) {
		return;
	}

	start_35_timer(channel);

	channel->rx_buffer[channel->rx_write_index++] = ch;

	if (channel->rx_write_index == SYS_MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

#endif // SYS_MODBUS

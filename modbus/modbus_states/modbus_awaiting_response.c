#define TAG "MODBUS_AWAITING_RESPONSE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(struct modbus_channel *channel);
static void process_rx_character(struct modbus_channel *channel, u8 ch);
static void process_response_timeout(struct modbus_channel *channel);

void set_modbus_awaiting_response_state(struct modbus_channel *channel)
{
//	LOG_D("set_modbus_awaiting_response_state()\n\r");
	channel->rx_write_index = 0;

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->process_tx_finished = NULL;
	channel->process_rx_character = process_rx_character;
	channel->process_response_timeout = process_response_timeout;

	start_response_timer(channel);
}

void process_timer_35_expiry(struct modbus_channel *channel)
{
	u8  start_index;

//	RX_ISR_ENABLE = 0;
	LOG_D("process_timer_35_expiry() channel %d msg length %d\n\r", channel->uart->uart, channel->rx_write_index);
	set_modbus_idle_state(channel);

	if(channel->rx_write_index > 2) {
		if(channel->rx_buffer[0] == channel->address) {
			start_index = 0;
		} else if (channel->rx_buffer[1] == channel->address) {
			start_index = 1;
		} else {
			LOG_D("message from wrong address\n\r");
			return;
		}

		if (crc_check(&(channel->rx_buffer[start_index]), channel->rx_write_index - start_index)) {
			/*
			 * Response Good
			 * Subtract 2 for the CRC
			 */
			channel->process_response(&(channel->rx_buffer[start_index]), channel->rx_write_index - (start_index + 2));
		} else {
			LOG_D("Message bad!\n\r");
			channel->process_response(NULL, 0);
		}
	} else {
		LOG_D("Message too short\n\r");
		channel->process_response(NULL, 0);
	}
}

void process_rx_character(struct modbus_channel *channel, u8 ch)
{
	if ((channel->rx_write_index == 0) && (ch == 0x00)) {
		return;
	}
	cancel_response_timer(channel);
	start_35_timer(channel);

	channel->rx_buffer[channel->rx_write_index++] = ch;

	if (channel->rx_write_index == MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

static void process_response_timeout(struct modbus_channel *channel)
{
	set_modbus_starting_state(channel);
	if(channel->process_response) {
		channel->process_response(NULL, 0);
	} else {
		LOG_E("No response Function\n\r");
	}
}

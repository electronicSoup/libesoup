#define TAG "MODBUS_IDLE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void transmit(struct modbus_channel *channel, u8 *data, u16 len, modbus_response_function fn, void* callback_data);

void set_modbus_idle_state(struct modbus_channel *channel)
{
	LOG_D("set_modbus_idle_state(channel %d)\n\r", channel->uart->uart);

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = NULL;
	channel->transmit = transmit;
	channel->process_tx_finished = NULL;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;

	if(channel->idle_callback) {
		channel->idle_callback(channel->idle_callback_data);
	}
}

void transmit(struct modbus_channel *channel, u8 *data, u16 len, modbus_response_function fn, void *callback_data)
{
	LOG_D("Modbus Idle state Transmit()\n\r");

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

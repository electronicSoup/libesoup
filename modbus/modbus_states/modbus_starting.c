#define TAG "MODBUS_STARTING"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(struct modbus_channel *channel);

void set_modbus_starting_state(struct modbus_channel *channel)
{
	LOG_D("set_modbus_starting_state(channel %d)\n\r", channel->uart->uart);

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->process_tx_finished = NULL;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;

	start_35_timer(channel);
}

static void process_timer_35_expiry(struct modbus_channel *channel)
{
	LOG_D("process_timer_35_expiry(channel %d)\n\r", channel->uart->uart);

	set_modbus_idle_state(channel);
}

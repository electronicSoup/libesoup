#define TAG "MODBUS_TRANSMITTING"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_tx_finished(struct modbus_channel *channel);

void set_modbus_transmitting_state(struct modbus_channel *channel)
{
//	LOG_D("set_modbus_transmitting_state()\n\r");

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = NULL;
	channel->transmit = NULL;
	channel->process_tx_finished = process_tx_finished;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;
}

void process_tx_finished(struct modbus_channel *channel)
{
//	LOG_D("process_tx_finished()\n\r");
	set_modbus_awaiting_response_state(channel);
}

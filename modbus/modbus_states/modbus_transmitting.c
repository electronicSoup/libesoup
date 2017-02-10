#define TAG "MODBUS_TRANSMITTING"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void tx_finished(void *);

void set_modbus_transmitting_state(struct modbus_channel *channel)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_modbus_transmitting_state()\n\r");
#endif

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = NULL;
	channel->transmit = NULL;
	channel->modbus_tx_finished = tx_finished;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;
}

void tx_finished(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
        log_d(TAG, "tx_finished()\n\r");
#endif
	set_modbus_awaiting_response_state(channel);
}

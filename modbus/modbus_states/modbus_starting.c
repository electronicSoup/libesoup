#define TAG "MODBUS_STARTING"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void *);

void set_modbus_starting_state(struct modbus_channel *channel)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "set_modbus_starting_state(channel %d)\n\r", channel->uart->uart);
#endif

	channel->process_timer_15_expiry = NULL;
	channel->process_timer_35_expiry = process_timer_35_expiry;
	channel->transmit = NULL;
	channel->modbus_tx_finished = NULL;
	channel->process_rx_character = NULL;
	channel->process_response_timeout = NULL;

	start_35_timer(channel);
}

static void process_timer_35_expiry(void *data)
{
        struct modbus_channel *channel = (struct modbus_channel *)data;
        
        if(channel->uart) {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
                log_d(TAG, "process_timer_35_expiry(channel %d)\n\r", channel->uart->uart);
#endif

                set_modbus_idle_state(channel);
        } else {
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
                log_d(TAG, "process_timer_35_expiry() No Uart\n\r");
#endif
        }
}

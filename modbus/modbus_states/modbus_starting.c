#define TAG "MODBUS_STARTING"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void);

void set_modbus_starting_state(void)
{
	LOG_D("set_modbus_starting_state()\n\r");

	modbus_state.process_timer_15_expiry = NULL;
	modbus_state.process_timer_35_expiry = process_timer_35_expiry;
	modbus_state.transmit = NULL;
	modbus_state.process_tx_finished = NULL;
	modbus_state.process_rx_character = NULL;
	modbus_state.process_response_timeout = NULL;

	start_35_timer();
}

static void process_timer_35_expiry(void)
{
	LOG_D("process_timer_35_expiry()\n\r");

	set_modbus_idle_state();
}

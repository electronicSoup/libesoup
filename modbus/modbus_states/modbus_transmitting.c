#define TAG "MODBUS_TRANSMITTING"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_tx_finished(void);

static u8 address;
static modbus_response_function process_response;

void set_modbus_transmitting_state(u8 addr, modbus_response_function fn)
{
//	LOG_D("set_modbus_transmitting_state()\n\r");

	address = addr;
	process_response = fn;

	modbus_state.process_timer_15_expiry = NULL;
	modbus_state.process_timer_35_expiry = NULL;
	modbus_state.transmit = NULL;
	modbus_state.process_tx_finished = process_tx_finished;
	modbus_state.process_rx_character = NULL;
	modbus_state.process_response_timeout = NULL;
}

void process_tx_finished(void)
{
//	LOG_D("process_tx_finished()\n\r");
	set_modbus_awaiting_response_state(address, process_response);
}

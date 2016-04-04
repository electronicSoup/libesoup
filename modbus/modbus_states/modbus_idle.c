#define TAG "MODBUS_IDLE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void transmit(u8 *data, u16 len, modbus_response_function fn);

void set_modbus_idle_state(void)
{
//	LOG_D("set_modbus_idle_state()\n\r");

	modbus_state.process_timer_15_expiry = NULL;
	modbus_state.process_timer_35_expiry = NULL;
	modbus_state.transmit = transmit;
	modbus_state.process_tx_finished = NULL;
	modbus_state.process_rx_character = NULL;
	modbus_state.process_response_timeout = NULL;
}
void transmit(u8 *data, u16 len, modbus_response_function fn)
{
	/*
	 * The response timeout timer is started when the transmission is
	 * completed in the modbus_awaiting_response state.
	 */
	set_modbus_transmitting_state(data[0], fn);
	modbus_tx_data(data, len);
}

#define TAG "MODBUS_AWAITING_RESPONSE"
#define DEBUG_FILE

#include "system.h"

#include "es_lib/logger/serial_log.h"
#include "es_lib/modbus/modbus.h"

extern struct modbus_state modbus_state;

static void process_timer_35_expiry(void);
static void process_rx_character(u8 ch);

static u8  rx_buffer[MODBUS_RX_BUFFER_SIZE];
static u16 rx_write_index;
//static u8     rx_line_complete = 0;
//static UINT16 rx_read_index = 0;
//static UINT16 rx_buffer_count = 0;

//static u8 watch_trmt = FALSE;

static u8 address;
static modbus_response_function process_response;

static void process_response_timeout(void);

void set_modbus_awaiting_response_state(u8 addr, modbus_response_function fn)
{
//	LOG_D("set_modbus_awaiting_response_state()\n\r");

	address = addr;
	process_response = fn;

	modbus_state.process_timer_15_expiry = NULL;
	modbus_state.process_timer_35_expiry = process_timer_35_expiry;
	modbus_state.transmit = NULL;
	modbus_state.process_tx_finished = NULL;
	modbus_state.process_rx_character = process_rx_character;
	modbus_state.process_response_timeout = process_response_timeout;

	rx_write_index = 0;
	start_response_timer(address);
}

void process_timer_35_expiry(void)
{
	u8  start_index;
	u16 loop;

	RX_ISR_ENABLE = 0;
	LOG_D("process_timer_35_expiry() msg length %d\n\r", rx_write_index);
	set_modbus_idle_state();

	if(rx_write_index > 2) {
#ifdef DEBUG
		for (loop = 0; loop < rx_write_index; loop++) {
			asm ("CLRWDT");
			LOG_D("char %d - 0x%x - %c\n\r", loop, rx_buffer[loop], rx_buffer[loop]);
		}
#endif
		if(rx_buffer[0] == address) {
			start_index = 0;
		} else if (rx_buffer[1] == address) {
			start_index = 1;
		} else {
			LOG_D("message from wrong address\n\r");
			return;
		}
		LOG_D("start index is %d\n\r", start_index);

		if (crc_check(&rx_buffer[start_index], rx_write_index - start_index)) {
			LOG_D("Message Rx'd good\n\r");
			/*
			 * Subtract 2 for the CRC
			 */
			process_response(&rx_buffer[start_index], rx_write_index - (start_index + 2));
		} else {
			LOG_D("Message bad!\n\r");
			process_response(NULL, 0);
		}
	} else {
		LOG_D("Message too short\n\r");
		process_response(NULL, 0);
	}
}

void process_rx_character(u8 ch)
{
//	LOG_D("process_rx_character(0x%x)\n\r", ch);

	if ((rx_write_index == 0) && (ch == 0x00)) {
//		putchar('%');
		return;
	}
	cancel_response_timer();
	start_35_timer();

	rx_buffer[rx_write_index++] = ch;

	if (rx_write_index == MODBUS_RX_BUFFER_SIZE) {
		LOG_E("UART 2 Overflow: Line too long\n\r");
	}
}

static void process_response_timeout(void)
{
//	LOG_D("process_response_timout()\n\r");
	set_modbus_idle_state();
	process_response(NULL, 0);
}

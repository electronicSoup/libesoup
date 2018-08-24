/*
 * libesoup/examples/main_modbus_slave.c
 *
 * Copyright 2018 electronicSoup Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU Lesser General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */
#include "libesoup_config.h"

#include "libesoup/errno.h"
#include "libesoup/timers/delay.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/comms/modbus/modbus.h"

#define DEBUG_FILE
static const char *TAG = "SLAVE";
#include "libesoup/logger/serial_log.h"

#define SN65HVD72D_TX        RD0
#define SN65HVD72D_TX_ENABLE RD1
#define SN65HVD72D_RX        RD2

#define SN65HVD72D_RECEIVE   0b0
#define SN65HVD72D_SEND      0b1

static struct modbus_app_data  modbus_data;
static uint8_t                 modbus_chan_idle;

void callback(modbus_id chan, uint8_t *msg, uint8_t len)
{
	uint8_t i;
	
	LOG_D("%s\n\r", __func__);
	for (i = 0; i < len; i++) {
		LOG_D("0x%x\n\r", msg[i]);
	}
}

void tx_finished(struct uart_data *uart)
{
	result_t rc;
	
	rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_RECEIVE);
	RC_CHECK_STOP
}

void modbus_idle(modbus_id modbus, uint8_t idle)
{
	result_t rc;

	if (modbus != modbus_data.channel_id) {
		rc = -1;
		RC_CHECK_STOP
	}
	
	if(idle) {
		LOG_D("Idle\n\r");
	} else {
		LOG_D("Busy\n\r");
	}
	modbus_chan_idle = idle;
}

void modbus_process(modbus_id chan, uint8_t *frame, uint8_t len)
{
	result_t rc;
	uint8_t  i;
	uint8_t  response[5];

	/*
	 * If not other response is sent then the request is unsupported.
	 * Send an error response accordingly.
	 */
	response[0] = 0xff;
	response[1] = 0xff;
	rc = modbus_error_resp(chan, response, 2);
	RC_CHECK_PRINT_CONT("Response!\n\r");

	LOG_D("%s chan-%d, len-%d\n\r", __func__, chan, len);

	for (i = 0; i < len; i++) {
		serial_printf("0x%x,", frame[i]);
	}
	serial_printf("\n\r");
}

void modbus_broadcast_handler(modbus_id chan, uint8_t *frame, uint8_t len)
{
	uint8_t i;

	LOG_D("%s chan-%d, len-%d\n\r", __func__, chan, len);

	for (i = 0; i < len; i++) {
		serial_printf("0x%x,", frame[i]);
	}
	serial_printf("\n\r");
}

int main()
{
	result_t          rc;
	rc = libesoup_init();
	RC_CHECK_STOP

	modbus_chan_idle = FALSE;
	
	rc = gpio_set(SN65HVD72D_TX, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK_STOP
		
	rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_RECEIVE);
	RC_CHECK_STOP
		
	rc = gpio_set(SN65HVD72D_RX, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK_STOP

	/*
	 * Initialise the UART connected to the MAX3221E
	 */
	rc = uart_calculate_mode(&modbus_data.uart_data.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_TWO_STOP_BITS, UART_IDLE_HIGH);
	RC_CHECK_STOP

	modbus_data.address                   = 0x01;                // Modbus Slave Address
	modbus_data.idle_state_callback       = modbus_idle;
	modbus_data.unsolicited_frame_handler = modbus_process;
	modbus_data.broadcast_frame_handler   = modbus_broadcast_handler;
	modbus_data.uart_data.tx_pin          = SN65HVD72D_TX;
	modbus_data.uart_data.rx_pin          = SN65HVD72D_RX;
	modbus_data.uart_data.tx_finished     = tx_finished;
	modbus_data.uart_data.baud            = 9600;                // Nice relaxed baud rate
	
	/*
	 * Reserve a UART channel for our use
	 */
	rc = modbus_reserve(&modbus_data);
	RC_CHECK_STOP

	LOG_D("Entering main loop\n\r");
	
	while(1) {
		libesoup_tasks();
		Nop();
	}
}

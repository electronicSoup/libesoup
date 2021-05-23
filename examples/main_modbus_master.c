/*
 * libesoup/examples/main_modbus_master.c
 *
 * Copyright 2018-2019 electronicSoup Limited
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
#ifdef SYS_EXAMPLE_MODBUS_MASTER

#include "libesoup/errno.h"
#include "libesoup/timers/delay.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/comms/modbus/modbus.h"

#define DEBUG_FILE
static const char *TAG = "MASTER";
#include "libesoup/logger/serial_log.h"

#define SN65HVD72D_TX        RD0
#define SN65HVD72D_TX_ENABLE RD1
#define SN65HVD72D_RX        RD2

#define SN65HVD72D_RECEIVE   0b0
#define SN65HVD72D_SEND      0b1

static struct modbus_app_data   app_data;
static uint8_t                  modbus_chan_idle;

void coil_callback(modbus_id chan, uint8_t *msg, uint8_t len)
{
	uint8_t i;

	LOG_D("%s len-%d\n\r", __func__, len);
	for (i = 0; i < len; i++) {
		serial_printf("0x%x-", msg[i]);
	}
	serial_printf("\n\r");

	if (msg[1] & 0x80) {
		LOG_E("Error Resp\n\r");
	}
}

void regs_callback(modbus_id chan, uint8_t *msg, uint8_t len)
{
	uint8_t i;

	for (i = 0; i < len; i++) {
		serial_printf("0x%x-", msg[i]);
	}
	serial_printf("\n\r");

	if (msg[1] & 0x80) {
		LOG_E("Error Resp\n\r");
	}
}

void exp_fn(timer_id timer, union sigval data)
{
	result_t rc;

	if (modbus_chan_idle) {
		/*
		 * Enter Transmit mode on the transceiver
		 */
		rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
		RC_CHECK_STOP

//		rc = modbus_read_coils_req(app_data.channel_id, 0x01, 0x0000, 0x0001, coil_callback);
		rc = modbus_read_holding_regs_req(app_data.channel_id, 0x01, 0x0000, 0x0002, regs_callback);
		RC_CHECK_STOP
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
	if (modbus != app_data.channel_id) {
		while(1);
	}
	modbus_chan_idle = idle;
}

int main()
{
	result_t          rc;
	struct timer_req  request;
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
	rc = uart_calculate_mode(&app_data.uart_data.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	RC_CHECK_STOP

	app_data.idle_state_callback         = modbus_idle;
	app_data.address                     = 0;               // Master Node
	app_data.unsolicited_frame_handler   = NULL;            // As Master node only processing sollicitated responses
	app_data.uart_data.tx_pin            = SN65HVD72D_TX;
	app_data.uart_data.rx_pin            = SN65HVD72D_RX;
	app_data.uart_data.tx_finished       = tx_finished;
	app_data.uart_data.baud              = 9600;                // Nice relaxed baud rate

	/*
	 * Reserve a UART channel for our use
	 */
	rc = modbus_reserve(&app_data);
	RC_CHECK_STOP

	request.period.units    = Seconds;
	request.period.duration = 30;
	request.type            = repeat_expiry;
	request.exp_fn          = exp_fn;

	rc = sw_timer_start(&request);

	LOG_D("Entering main loop\n\r");

	while(1) {
		libesoup_tasks();
		Nop();
	}
}

#endif // SYS_EXAMPLE_MODBUS_MASTER

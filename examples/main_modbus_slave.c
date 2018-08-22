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

static struct uart_data   uart;

modbus_id         modbus_chan;
static uint8_t    modbus_chan_idle;

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

void rx_char(uint8_t uart_id, uint8_t ch)
{
	LOG_D("rx_char(%c)\n\r", ch);
}

void modbus_idle(modbus_id modbus, uint8_t idle)
{
	result_t rc;

	if (modbus != modbus_chan) {
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
	LOG_D("%s\n\r", __func__);
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
	rc = uart_calculate_mode(&uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_TWO_STOP_BITS, UART_IDLE_HIGH);
	RC_CHECK_STOP

	uart.tx_pin          = SN65HVD72D_TX;
	uart.rx_pin          = SN65HVD72D_RX;
	uart.tx_finished     = tx_finished;
	uart.baud            = 9600;                // Nice relaxed baud rate
	uart.process_rx_char = rx_char;
	
	/*
	 * Reserve a UART channel for our use
	 */
	rc = modbus_slave_reserve(&uart, modbus_idle, modbus_process);
	if (rc >= 0) {
		modbus_chan = rc;
	}
	RC_CHECK_STOP

	LOG_D("Entering main loop\n\r");
	
	while(1) {
		libesoup_tasks();
		Nop();
	}
}

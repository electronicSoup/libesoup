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

void tx_finished(struct uart_data *uart)
{
	result_t rc;
	
	rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_RECEIVE);
	RC_CHECK_STOP
}

static result_t process_read_coils(modbus_id chan, uint8_t *frame, uint8_t len)
{
	result_t  rc;
	uint16_t  coil_address;
	uint16_t  number_of_coils;
	uint8_t   buffer;

	if (len != 4) {
		rc = modbus_error_resp(chan, MODBUS_READ_COILS, MODBUS_ADDRESS_EXCEPTION);
		RC_CHECK_LINE_CONT
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	coil_address    = (frame[0] << 8) | frame[1];
	number_of_coils = (frame[2] << 8) | frame[3];

	LOG_D("Read %d coils from address 0x%x\n\r", coil_address, number_of_coils);
	
	/*
	 * Just for example let's pretend we have one coil at address zero
	 */
	if (coil_address != 0x00) {
		return(modbus_error_resp(chan, MODBUS_READ_COILS, MODBUS_ADDRESS_EXCEPTION));
	}

	/*
	 * Respond with a single bit for coil 0x000 value 1
	 */
	buffer = 0x01;
	return(modbus_read_coils_resp(chan, &buffer, 0x01));
}


void modbus_process(modbus_id chan, uint8_t *frame, uint8_t len)
{
	result_t rc;
	uint8_t  i;

	LOG_D("Process a received frame\n\r");
	/*
	 * A simple application which only processes the MODBUS_READ_COILS
	 * Function code. All other function requests will result in an error
	 * response to the master.
	 */
	switch(frame[0]) {
	case MODBUS_READ_COILS:
		rc = process_read_coils(chan, frame, len);
		RC_CHECK_PRINT_CONT("Read Coils!\n\r");
		break;

	default:
		/*
		 * Function code is not handled so return error
		 */
		rc = modbus_error_resp(chan, frame[0], MODBUS_FUNCTION_CODE_EXCEPTION);
		RC_CHECK_LINE_CONT
		break;
	}

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
	rc = uart_calculate_mode(&modbus_data.uart_data.uart_mode, UART_8_DATABITS, UART_PARITY_EVEN, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	RC_CHECK_STOP

	modbus_data.address                   = 0x01;                // Modbus Slave Address
	modbus_data.unsolicited_frame_handler = modbus_process;
	modbus_data.broadcast_frame_handler   = modbus_broadcast_handler;
	modbus_data.uart_data.tx_pin          = SN65HVD72D_TX;
	modbus_data.uart_data.rx_pin          = SN65HVD72D_RX;
	modbus_data.uart_data.tx_finished     = tx_finished;
	modbus_data.uart_data.baud            = 9600;	// Nice relaxed baud rate

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

/*
 * libesoup/comms/control_chain/test/main_control_chain.c
 *
 * Copyright 2020 electronicSoup Limited
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

#ifdef SYS_TEST_CONTROL_CHAIN

#include "libesoup/errno.h"
#include "libesoup/timers/delay.h"
#include "libesoup/timers/sw_timers.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/comms/uart/uart.h"

#define DEBUG_FILE
static const char *TAG = "CC_MAIN";
#include "libesoup/logger/serial_log.h"

#define SN65HVD72D_TX        RD0
#define SN65HVD72D_TX_ENABLE RD1
#define SN65HVD72D_RX        RD2

#define SN65HVD72D_RECEIVE   0b0
#define SN65HVD72D_SEND      0b1

#define CONTROL_CHAIN_BAUD   9600

static struct uart_data      cc_channel;

static void cc_rx(uint8_t uart_id, uint8_t ch)
{
	LOG_D("Rx 0x%x\n\r", ch);
}

#if 0
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
	uint8_t   i;
	result_t  rc;
	uint16_t  coil_address;
	uint16_t  number_of_coils;
	uint8_t   buffer;

	LOG_D("%s len %d\n\r", __func__, len);

	for (i = 0; i < len; i++) {
		serial_printf("0x%x,", frame[i]);
	}
	serial_printf("\n\r");

	if (len != 4) {
		rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
		RC_CHECK_STOP
		rc = modbus_error_resp(chan, MODBUS_READ_COILS, MODBUS_ADDRESS_EXCEPTION);
		RC_CHECK_LINE_CONT
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	coil_address    = frame[0];
	coil_address    = (coil_address << 8) | frame[1];
	number_of_coils = frame[2];
	number_of_coils = (number_of_coils << 8) | frame[3];

	/*
	 * Just for example let's pretend we have one coil at address zero
	 */
	if (coil_address != 0x00) {
		rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
		RC_CHECK_STOP
		return(modbus_error_resp(chan, MODBUS_READ_COILS, MODBUS_ADDRESS_EXCEPTION));
	}

	/*
	 * Respond with a single bit for coil 0x000 value 1
	 */
	buffer = 0x01;
	rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
	RC_CHECK_STOP
	return(modbus_read_coils_resp(chan, &buffer, 0x01));
}

static result_t process_read_registers(modbus_id chan, uint8_t *frame, uint8_t len)
{
	uint8_t   i;
	result_t  rc;
	uint16_t  reg_address;
	uint16_t  number_of_regs;
	uint8_t   buffer[4];

	LOG_D("%s len %d\n\r", __func__, len);

	for (i = 0; i < len; i++) {
		serial_printf("0x%x,", frame[i]);
	}
	serial_printf("\n\r");

	if (len != 4) {
		rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
		RC_CHECK_STOP
		rc = modbus_error_resp(chan, MODBUS_READ_COILS, MODBUS_ADDRESS_EXCEPTION);
		RC_CHECK_LINE_CONT
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	reg_address    = frame[0];
	reg_address    = (reg_address << 8) | frame[1];
	number_of_regs = frame[2];
	number_of_regs = (number_of_regs << 8) | frame[3];

	/*
	 * Just for example let's pretend we have one coil at address zero
	 */
	if (reg_address != 0x00) {
		rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
		RC_CHECK_STOP
		return(modbus_error_resp(chan, MODBUS_READ_COILS, MODBUS_ADDRESS_EXCEPTION));
	}

	/*
	 * Respond with a single bit for coil 0x000 value 1
	 */
	buffer[0] = 0x00;
	buffer[1] = 0xb7;
	buffer[2] = 0x02;
	buffer[3] = 0x79;
	rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
	RC_CHECK_STOP
	return(modbus_read_registers_resp(chan, (uint8_t *)&buffer[0], 0x04));
}

void modbus_process(modbus_id chan, uint8_t *frame, uint8_t len)
{
	result_t rc;

	/*
	 * A simple application which only processes the MODBUS_READ_COILS
	 * Function code. All other function requests will result in an error
	 * response to the master.
	 */
	switch(frame[0]) {
	case MODBUS_READ_COILS:
		/*
		 * Don't send on Function code for processing only data
		 */
		rc = process_read_coils(chan, &frame[1], len - 1);
		RC_CHECK_PRINT_CONT("Read Coils!\n\r");
		break;

	case MODBUS_READ_HOLDING_REGISTERS:
		/*
		 * Don't send on Function code for processing only data
		 */
		rc = process_read_registers(chan, &frame[1], len - 1);
		RC_CHECK_PRINT_CONT("Read Regs!\n\r");
		break;

	default:
		/*
		 * Function code is not handled so return error
		 */
		LOG_D("Unrecognised message type\n\r");
		rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_SEND);
		RC_CHECK_STOP
		rc = modbus_error_resp(chan, frame[0], MODBUS_FUNCTION_CODE_EXCEPTION);
		if (rc < 0) {
			LOG_E("Response failed\n\r");
		}
		break;
	}
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
#endif

int main()
{
	result_t          rc;
	rc = libesoup_init();
	RC_CHECK_STOP

	rc = gpio_set(SN65HVD72D_TX, GPIO_MODE_DIGITAL_OUTPUT, 0);
	RC_CHECK_STOP

	rc = gpio_set(SN65HVD72D_TX_ENABLE, GPIO_MODE_DIGITAL_OUTPUT, SN65HVD72D_RECEIVE);
	RC_CHECK_STOP

	rc = gpio_set(SN65HVD72D_RX, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK_STOP


	cc_channel.rx_pin          = SN65HVD72D_RX;
	cc_channel.tx_pin          = SN65HVD72D_TX;
	cc_channel.baud            = CONTROL_CHAIN_BAUD;
	cc_channel.process_rx_char = cc_rx;
	rc = uart_calculate_mode(&cc_channel.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);

	rc = uart_reserve(&cc_channel);
#if 0
	/*
	 * Initialise the UART connected to the MAX3221E
	 */
	rc = uart_calculate_mode(&modbus_data.uart_data.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
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
#endif
	LOG_D("Entering main loop\n\r");

	while(1) {
		libesoup_tasks();
		Nop();
	}
}

#endif  // SYS_TEST_CONTROL_CHAIN

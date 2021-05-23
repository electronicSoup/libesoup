/**
 * libesoup/examples/modbus_main.c
 *
 * An example main.c file for the MODBUS
 *
 * The code is used in the example MPLAB-X project:
 * libesoup/examples/projects/microchip/BareBones.X
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
#ifdef SYS_EXAMPLE_MODBUS

#define DEBUG_FILE
static const char *TAG = "MAIN";
#include "libesoup/logger/serial_log.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/comms/modbus/modbus.h"

/*
 * Global Data
 */
static struct uart_data uart;

/*
 * Local static funcitons
 */
static void modbus_rx_frame(uint8_t *msg, uint8_t size, void *data);

int main(void)
{
        result_t rc;

	rc = libesoup_init();

	LOG_D("*************************\n\r");
	LOG_D("***   ModbusMonitor   ***\n\r");
	LOG_D("*************************\n\r");
	LOG_D("RCON 0x%x\n\r", RCON);

	asm ("CLRWDT");

	LOG_D("Initialise RS485 Chip Select\n\r");

        uart.tx_pin = RD7;
	uart.rx_pin = RD6;
	uart.baud   = 9600;
	rc = uart_calculate_mode(&uart.uart_mode, UART_8_DATABITS, UART_PARITY_NONE, UART_ONE_STOP_BIT, UART_IDLE_HIGH);
	uart.tx_finished = NULL;

        rc = modbus_master_reserve(&uart, NULL);
        if (rc != 0) {
                LOG_E("Failed to reserve a Modbus Channel\n\r");
        }

        LOG_D("Entering the main loop\n\r");

	while(1){
		libesoup_tasks();
		asm ("CLRWDT");
//		jobs_execute();
	}
}

void modbus_rx_frame(uint8_t *msg, uint8_t size, void *data)
{
        char      ch[10];
        char      string[100];
        uint8_t  *ptr;
        uint16_t  loop;

        string[0] = '\0';
        ptr = msg;

        LOG_D("frame len %d\n\r", size);
#if 0
        for(loop = 0; loop < size; loop++) {
                sprintf(ch, "0x%x,", *ptr++);
                strcat(string, ch);
        }
        LOG_D("modbus_rx_frame(%s)\n\r", string);
#endif
}

#endif // SYS_EXAMPLE_MODBUS

#include "libesoup_config.h"

//#include <stdio.h>
//#include <string.h>

#include "libesoup/logger/serial_log.h"
//#include "es_lib/logger/serial_log.h"
//#include "es_lib/timers/hw_timers.h"
//#include "es_lib/timers/timers.h"
//#include "es_lib/comms/uart.h"
//#include "es_lib/modbus/modbus.h"
//#include "es_lib/jobs/jobs.h"

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

	jobs_init();
	hw_timer_init();
	timer_init();

	/*
	 * Univarsal UART Code
	 */
	uart_init();

	serial_init();

	LOG_D("*************************\n\r");
	LOG_D("***   ModbusMonitor   ***\n\r");
	LOG_D("*************************\n\r");
	LOG_D("RCON 0x%x\n\r", RCON);

	asm ("CLRWDT");


	LOG_D("Initialise RS485 Chip Select\n\r");
	/*
	 * Pins RB2 and RB3 select transmit and receive so both are outputs
	 */
	TRISBbits.TRISB2 = 0;
	TRISBbits.TRISB3 = 0;

	RS485_RX;

        uart.tx_pin = RP1;
	uart.rx_pin = RP0;
	uart.baud   = MM_BAUD;
	uart.uart_mode = uart_calculate_mode(MM_DATABITS, MM_PARITY, MM_STOPBITS, IDLE_LOW);
	uart.tx_finished = NULL;

        rc = modbus_reserve(&uart, NULL, modbus_rx_frame, NULL);
        if (rc != SUCCESS) {
                LOG_E("Failed to reserve a Modbus Channel\n\r");
        }

        LOG_D("Entering the main loop\n\r");

	while(1){
		asm ("CLRWDT");
		CHECK_TIMERS();
		jobs_execute();
	}
}

void modbus_rx_frame(u8 *msg, u8 size, void *data)
{
        char  ch[10];
        char  string[100];
        u8   *ptr;
        u16   loop;

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

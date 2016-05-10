/**
 *
 * \file es_lib/comms/uart.c
 *
 * UART functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the version 2 of the GNU General Public License
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *******************************************************************************
 *
 */

#define U1_RX_ISR_FLAG   IFS0bits.U1RXIF
#define U1_TX_ISR_FLAG   IFS0bits.U1TXIF
#define U1_RX_ISR_ENABLE IEC0bits.U1RXIE
#define U1_TX_ISR_ENABLE IEC0bits.U1TXIE

#define U2_RX_ISR_FLAG   IFS1bits.U2RXIF
#define U2_TX_ISR_FLAG   IFS1bits.U2TXIF
#define U2_RX_ISR_ENABLE IEC1bits.U2RXIE
#define U2_TX_ISR_ENABLE IEC1bits.U2TXIE

#define U3_RX_ISR_FLAG   IFS5bits.U3RXIF
#define U3_TX_ISR_FLAG   IFS5bits.U3TXIF
#define U3_RX_ISR_ENABLE IEC5bits.U3RXIE
#define U3_TX_ISR_ENABLE IEC5bits.U3TXIE

#define U4_RX_ISR_FLAG   IFS5bits.U4RXIF
#define U4_TX_ISR_FLAG   IFS5bits.U4TXIF
#define U4_RX_ISR_ENABLE IEC5bits.U4RXIE
#define U4_TX_ISR_ENABLE IEC5bits.U4TXIE

// WARNING #define NUM_UARTS        3 is in .h file
#define UART_2           0x00
#define UART_3           0x01
#define UART_4           0x02
#define UART_BAD         0xff

#define DEBUG_FILE
#define TAG "UART"

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/utils/rand.h"

#include "uart.h"


enum uart_status {
	UART_FREE,
	UART_BUSY
};

struct uart {
	enum uart_status status;
	u16              magic;
	uart_data       *data;
	u8               tx_buffer[UART_TX_BUFFER_SIZE];
	u16              tx_write_index;
	u16              tx_read_index;
	u16              tx_count;
} uart;

struct uart uarts[NUM_UARTS];

/*
 * Local static Function prototypes
 */
static void uart_tx_isr(u8);

static void uart_set_rx_pin(u8 uart, u8 pin);
static void uart_set_tx_pin(u8 uart, u8 pin);
static void uart_set_com_config(uart_data *com);

static void uart_putchar(u8 uart, u8 ch);

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static u16 load_tx_buffer(u8 uart);

/*
 * Interrupt Service Routines
 */
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
{
	while(U2_TX_ISR_FLAG) {
		uart_tx_isr(UART_2);
		U2_TX_ISR_FLAG = 0;
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U3TXInterrupt(void)
{
	while(U3_TX_ISR_FLAG) {
		uart_tx_isr(UART_3);
		U3_TX_ISR_FLAG = 0;
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U4TXInterrupt(void)
{
	while(U4_TX_ISR_FLAG) {
		uart_tx_isr(UART_4);
		U4_TX_ISR_FLAG = 0;
	}
}

static void uart_tx_isr(u8 uart)
{
	if ((uarts[uart].data == NULL) || (uarts[uart].status != UART_BUSY)) {
		LOG_E("UART Null in ISR!\n\r");
		return;
	}

	/*
	 * If the TX buffer is not full load it from the tx buffer
	 */
	if (!load_tx_buffer(uart)) {
		switch(uart) {
			case UART_2:
				/*
				 * Can't use U2STAbits.TRMT to detect when Tx Queue is empty
				 * as it ain't reliable at all.
				 */
				if (U2STAbits.UTXISEL0) {
					while (!U2STAbits.TRMT) {
						Nop();
					}

					/*
					 * Inform the higher layer we're finished
					 */
					uarts[uart].data->tx_finished(UART_2);

					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U2STAbits.UTXISEL1 = 1;
					U2STAbits.UTXISEL0 = 0;
				} else {
					/*
					 * Interrupt when the last character is shifted out of the Transmit
					 * Shift Register; all transmit operations are completed
					 */
					U2STAbits.UTXISEL1 = 0;
					U2STAbits.UTXISEL0 = 1;
				}
				break;

			case UART_3:
				/*
				 * Can't use U2STAbits.TRMT to detect when Tx Queue is empty
				 * as it ain't reliable at all.
				 */
				if (U3STAbits.UTXISEL0) {
					while (!U3STAbits.TRMT) {
						Nop();
					}

					/*
					 * Inform the higher layer we're finished
					 */
					uarts[uart].data->tx_finished(UART_3);

					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U3STAbits.UTXISEL1 = 1;
					U3STAbits.UTXISEL0 = 0;
				} else {
					/*
					 * Interrupt when the last character is shifted out of the Transmit
					 * Shift Register; all transmit operations are completed
					 */
					U3STAbits.UTXISEL1 = 0;
					U3STAbits.UTXISEL0 = 1;
				}
				break;

			case UART_4:
				/*
				 * Can't use U2STAbits.TRMT to detect when Tx Queue is empty
				 * as it ain't reliable at all.
				 */
				if (U4STAbits.UTXISEL0) {
					while (!U4STAbits.TRMT) {
						Nop();
					}

					/*
					 * Inform the higher layer we're finished
					 */
					uarts[uart].data->tx_finished(UART_4);

					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U4STAbits.UTXISEL1 = 1;
					U4STAbits.UTXISEL0 = 0;
				} else {
					/*
					 * Interrupt when the last character is shifted out of the Transmit
					 * Shift Register; all transmit operations are completed
					 */
					U4STAbits.UTXISEL1 = 0;
					U4STAbits.UTXISEL0 = 1;
				}
				break;

			default:
				LOG_E("Bad comm port given!\n\r");
				break;
		}
	}
}

/*
 * Receive Interrupt Service Routines
 */
void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
{
	u8 ch;

	U2_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U2STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U2STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U2STAbits.URXDA) {
		ch = U2RXREG;
		uarts[UART_2].data->process_rx_char(UART_2, ch);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U3RXInterrupt(void)
{
	u8 ch;

	U3_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U3STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U3STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U3STAbits.URXDA) {
		ch = U3RXREG;
		uarts[UART_3].data->process_rx_char(UART_3, ch);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U4RXInterrupt(void)
{
	u8 ch;

	U4_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U4STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U4STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U4STAbits.URXDA) {
		ch = U4RXREG;
		uarts[UART_4].data->process_rx_char(UART_4, ch);
	}
}


/*
 * Initialisation
 */
void uart_init(void)
{
	u8 loop;

	for(loop = 0; loop < NUM_UARTS; loop++) {
		uarts[loop].status = UART_FREE;
		uarts[loop].data = NULL;
	}
}

result_t uart_reserve(uart_data *data)
{
	/*
	 * Find a free uart to use
	 */
	u8  loop;

	for(loop = 0; loop < NUM_UARTS; loop++) {
		if(uarts[loop].status == UART_FREE) {

			uarts[loop].data = data;
			uarts[loop].status = UART_BUSY;

			data->uart = loop;

			uarts[loop].tx_write_index = 0;
			uarts[loop].tx_read_index = 0;
			uarts[loop].tx_count = 0;

			AD1PCFGL = 0xffff;

			/*
			 * Set up the Rx & Tx pins
			 */
			uart_set_rx_pin((u8) data->uart, data->rx_pin);
			uart_set_tx_pin((u8) data->uart, data->tx_pin);

			uart_set_com_config(data);

			return(SUCCESS);
		}
	}

	return(ERR_NO_RESOURCES);
}

result_t uart_release(uart_data *data)
{
	u8  uart;

	uart = data->uart;

	if(uarts[uart].data != data) {
		LOG_E("uart_tx called with bad data pointer\n\r");
		return(ERR_BAD_INPUT_PARAMETER);
	}

	uarts[uart].data = data;
	uarts[uart].status = UART_BUSY;

	data->uart = UART_BAD;

	return(SUCCESS);
}

result_t uart_tx(uart_data *data, u8 *buffer, u16 len)
{
	u8  uart;
	u8 *ptr;

	uart = data->uart;

	if(uarts[uart].data != data) {
		LOG_E("uart_tx called with bad data pointer\n\r");
		return(ERR_BAD_INPUT_PARAMETER);
	}

	ptr = buffer;

	while(len--) {
		uart_putchar(uart, *ptr++);
	}
	return(SUCCESS);
}

static void uart_putchar(u8 uart, u8 ch)
{
//	u8 loop;

	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */
	switch(uart) {
		case UART_2:
			/*
			 * If either the TX Buffer is full OR there are already characters in
			 * our SW Buffer then add to SW buffer
			 */
			if (uarts[uart].tx_count || U2STAbits.UTXBF) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U2STAbits.UTXISEL1 = 1;
				U2STAbits.UTXISEL0 = 0;

				if(uarts[uart].tx_count == UART_TX_BUFFER_SIZE) {
					LOG_E("Circular buffer full!");
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
				uarts[uart].tx_write_index = (++(uarts[uart].tx_write_index) % UART_TX_BUFFER_SIZE);
				uarts[uart].tx_count++;
			} else {
				U2TXREG = ch;
			}
			break;

		case UART_3:
			/*
			 * If either the TX Buffer is full OR there are already characters in
			 * our SW Buffer then add to SW buffer
			 */
			if (uarts[uart].tx_count || U3STAbits.UTXBF) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U3STAbits.UTXISEL1 = 1;
				U3STAbits.UTXISEL0 = 0;

				if(uarts[uart].tx_count == UART_TX_BUFFER_SIZE) {
					LOG_E("Circular buffer full!");
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
				uarts[uart].tx_write_index = (++(uarts[uart].tx_write_index) % UART_TX_BUFFER_SIZE);
				uarts[uart].tx_count++;
			} else {
				U3TXREG = ch;
			}
			break;

		case UART_4:
			/*
			 * If either the TX Buffer is full OR there are already characters in
			 * our SW Buffer then add to SW buffer
			 */
			if (uarts[uart].tx_count || U4STAbits.UTXBF) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U4STAbits.UTXISEL1 = 1;
				U4STAbits.UTXISEL0 = 0;

				if(uarts[uart].tx_count == UART_TX_BUFFER_SIZE) {
					LOG_E("Circular buffer full!");
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
				uarts[uart].tx_write_index = (++(uarts[uart].tx_write_index) % UART_TX_BUFFER_SIZE);
				uarts[uart].tx_count++;
			} else {
				U4TXREG = ch;
			}
			break;

		default:
			LOG_E("Unrecognised UART in putchar()\n\r");
			break;
	}
}

static void uart_set_rx_pin(u8 uart, u8 pin)
{
	switch (pin) {
		case RP0:
			TRISBbits.TRISB0 = 1;
			break;

		case RP1:
			TRISBbits.TRISB1 = 1;
			break;

		case RP13:
			TRISBbits.TRISB2 = 1;
			break;

		case RP25:
			TRISDbits.TRISD4 = 1;
			break;

		case RP28:
			TRISBbits.TRISB4 = 1;
			break;

		default:
			LOG_E("Unknow Peripheral Rx Pin\n\r");
			break;
	}

	switch (uart) {
//		case UART_1:
//			RPINR18bits.U1RXR = pin;
//			break;
//
		case UART_2:
			RPINR19bits.U2RXR = pin;
			break;

		case UART_3:
			RPINR17bits.U3RXR = pin;
			break;

		case UART_4:
			RPINR27bits.U4RXR = pin;
			break;
	}
}

static void uart_set_tx_pin(u8 uart, u8 pin)
{
	u8 tx_function;

	switch (uart) {
//		case UART_1:
//			tx_function = 3;
//			break;

		case UART_2:
			tx_function = 5;
			break;

		case UART_3:
			tx_function = 28;
			break;

		case UART_4:
			tx_function = 30;
			break;
	}

	switch (pin) {
		case RP0:
			TRISBbits.TRISB0 = 0;
			RPOR0bits.RP0R = tx_function;
			break;

		case RP1:
			TRISBbits.TRISB1 = 0;
			RPOR0bits.RP1R = tx_function;
			break;

		case RP13:
			TRISBbits.TRISB2 = 0;
			RPOR6bits.RP13R = tx_function;
			break;

		case RP25:
			TRISDbits.TRISD4 = 0;
			RPOR12bits.RP25R = tx_function;
			break;

		case RP28:
			TRISBbits.TRISB4 = 0;
			RPOR14bits.RP28R = tx_function;
			break;

		default:
			LOG_E("Unknow Peripheral Tx Pin\n\r");
			break;
	}
}

static void uart_set_com_config(uart_data *com)
{
	switch (com->uart) {
//		case UART_1:
//			U1MODE = com->uart_mode;
//
//			U1STA = 0x8410;
//
//			/*
//			 * Interrupt when a character is transferred to the Transmit Shift
//			 * Register (TSR), and as a result, the transmit buffer becomes empty
//			 */
//			U1STAbits.UTXISEL1 = 1;
//			U1STAbits.UTXISEL0 = 0;
//
//			/*
//			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
//			 *
//			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
//			 *
//			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
//			 *
//			 */
//			U1BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;
//
//			U1_RX_ISR_ENABLE = 0;
//			U1_TX_ISR_ENABLE = 1;
//			break;
//
		case UART_2:
			U2MODE = com->uart_mode;
			U2MODEbits.UARTEN = 1;

			U2STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U2STAbits.UTXISEL1 = 1;
			U2STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U2BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;

			U2_RX_ISR_ENABLE = 0;
			U2_TX_ISR_ENABLE = 1;
			break;

		case UART_3:
			U3MODE = com->uart_mode;
			U3MODEbits.UARTEN = 1;

			U3STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U3STAbits.UTXISEL1 = 1;
			U3STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U3BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;

			U3_RX_ISR_ENABLE = 0;
			U3_TX_ISR_ENABLE = 1;
			break;

		case UART_4:
			U4MODE = com->uart_mode;
			U4MODEbits.UARTEN = 1;

			U4STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U4STAbits.UTXISEL1 = 1;
			U4STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U4BRG = ((CLOCK_FREQ / com->baud) / 16) - 1;

			U4_RX_ISR_ENABLE = 0;
			U4_TX_ISR_ENABLE = 1;
			break;
	}
}

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static u16 load_tx_buffer(u8 uart)
{
	switch (uart) {
//		case UART_1:
//			while(!U1STAbits.UTXBF && (com->tx_buffer_read_index < com->tx_buffer_size)) {
//				U1TXREG = com->tx_buffer[com->tx_buffer_read_index++];
//			}
//
//			if(com->tx_buffer_read_index < com->tx_buffer_size) {
//				/*
//				 * Interrupt when a character is transferred to the Transmit Shift
//				 * Register (TSR), and as a result, the transmit buffer becomes empty
//				 */
//				U1STAbits.UTXISEL1 = 1;
//				U1STAbits.UTXISEL0 = 0;
//			} else {
//				/*
//				 * Interrupt when the last character is shifted out of the Transmit
//				 * Shift Register; all transmit operations are completed
//				 */
//				U1STAbits.UTXISEL1 = 0;
//				U1STAbits.UTXISEL0 = 1;
//			}
//			break;
//
		case UART_2:
			/*
			 * If the TX buffer is not full load it from the circular buffer
			 */
			while ((!U2STAbits.UTXBF) && (uarts[uart].tx_count)) {
				U2TXREG = uarts[uart].tx_buffer[uarts[uart].tx_read_index];
				uarts[uart].tx_read_index = (++(uarts[uart].tx_read_index) % UART_TX_BUFFER_SIZE);
				uarts[uart].tx_count--;
			}

			return(uarts[uart].tx_count);
			break;

		case UART_3:
			/*
			 * If the TX buffer is not full load it from the circular buffer
			 */
			while ((!U3STAbits.UTXBF) && (uarts[uart].tx_count)) {
				U3TXREG = uarts[uart].tx_buffer[uarts[uart].tx_read_index];
				uarts[uart].tx_read_index = (++(uarts[uart].tx_read_index) % UART_TX_BUFFER_SIZE);
				uarts[uart].tx_count--;
			}

			return(uarts[uart].tx_count);
			break;

		case UART_4:
			/*
			 * If the TX buffer is not full load it from the circular buffer
			 */
			while ((!U4STAbits.UTXBF) && (uarts[uart].tx_count)) {
				U4TXREG = uarts[uart].tx_buffer[uarts[uart].tx_read_index];
				uarts[uart].tx_read_index = (++(uarts[uart].tx_read_index) % UART_TX_BUFFER_SIZE);
				uarts[uart].tx_count--;
			}

			return(uarts[uart].tx_count);
			break;
	}

//	return(com->tx_buffer_size - com->tx_buffer_read_index);
	LOG_E("load_tx_buffer() Bad UART\n\r");
	return(0);
}

#if 0

	/*
	 * Serial Port pin configuration should be defined
	 * in include file system.h
	 */
	UxMODE = 0x8800;
//	UxMODEbits.LPBACK = 1;

	if (MODBUS_DATA_BITS == 8) {
		if (MODBUS_PARITY == PARITY_NONE) {
			UxMODEbits.PDSEL = 0x00;
		} else if (MODBUS_PARITY == PARITY_EVEN) {
			UxMODEbits.PDSEL = 0x01;
		} else if (MODBUS_PARITY == PARITY_ODD) {
			UxMODEbits.PDSEL = 0x10;
		}
	} else if (MODBUS_DATA_BITS == 9) {
		UxMODEbits.PDSEL = 0x11;
	} else {
		LOG_E("Unrecognised Data bit/Parity configuration\n\r");
	}

	if (MODBUS_STOP_BITS == ONE_STOP_BIT) {
		UxMODEbits.STSEL = 0;
	} else if (MODBUS_STOP_BITS == TWO_STOP_BITS) {
		UxMODEbits.STSEL = 1;
	} else {
		LOG_E("Unrecognised Stop bits configuration\n\r");
	}

	if (MODBUS_RX_IDLE_LEVEL == IDLE_LOW) {
		UxMODEbits.RXINV = 0;
	} else if (MODBUS_RX_IDLE_LEVEL == IDLE_HIGH) {
		UxMODEbits.RXINV = 1;
	}

	UxSTA  = 0x8410;

	/*
	 * Interrupt when a character is transferred to the Transmit Shift
	 * Register (TSR), and as a result, the transmit buffer becomes empty
	 */
	UxSTAbits.UTXISEL1 = 1;
	UxSTAbits.UTXISEL0 = 0;

	/*
	 * Interrupt when the last character is shifted out of the Transmit
	 * Shift Register; all transmit operations are completed
	 */
//	UxSTAbits.UTXISEL1 = 0;
//	UxSTAbits.UTXISEL0 = 1;

	/*
	 * Interrupt when a character is transferred to the Transmit Shift
	 * Register (this implies there is at least one character open in
	 * the transmit buffer)
	 */
//	UxSTAbits.UTXISEL1 = 0;
//	UxSTAbits.UTXISEL0 = 0;

	RX_ISR_ENABLE = 0;
	TX_ISR_ENABLE = 1;

	/*
	 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
	 *
	 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
	 *
	 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
	 *
	 */
	UxBRG = ((CLOCK_FREQ / MODBUS_BAUD) / 16) - 1;
#endif
/**
 *
 * \file es_lib/comms/uart.c
 *
 * UART functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2016 John Whitmore <jwhitmore@electronicsoup.com>
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
#define DEBUG_FILE TRUE
#define TAG "UART"

#include <stdio.h>

#include "system.h"
#include "es_lib/logger/serial_log.h"
#include "es_lib/utils/rand.h"

#include "es_lib/comms/uart/uart.h"

/*
 * Check required system.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error system.h file should define SYS_LOG_LEVEL (see es_lib/examples/system.h)
#endif

#ifndef SYS_CLOCK_FREQ
#error system.h file should define the SYS_CLOCK_FREQ
#endif

enum uart_status {
	UART_FREE,
	UART_BUSY
};

struct uart {
	enum uart_status       status;
	uint16_t               magic;
	struct uart_data      *data;
	uint8_t                tx_buffer[SYS_UART_TX_BUFFER_SIZE];
	uint16_t               tx_write_index;
	uint16_t               tx_read_index;
	uint16_t               tx_count;
} uart;

struct uart uarts[NUM_UARTS];

/*
 * Local static Function prototypes
 */
static void uart_tx_isr(uint8_t);

static void uart_set_rx_pin(uint8_t uart, uint8_t pin);
static void uart_set_tx_pin(uint8_t uart, uint8_t pin);
static void uart_set_com_config(struct uart_data *com);

static void uart_putchar(uint8_t uart, uint8_t ch);

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static uint16_t load_tx_buffer(uint8_t uart);

/*
 * Interrupt Service Routines
 */
void _ISR __attribute__((__no_auto_psv__)) _U1TXInterrupt(void)
{
	while(U1_TX_ISR_FLAG) {
		uart_tx_isr(UART_1);
		U1_TX_ISR_FLAG = 0;
	}
}

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

static void uart_tx_isr(uint8_t uart)
{
	if ((uarts[uart].data == NULL) || (uarts[uart].status != UART_BUSY)) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "UART Null in ISR!\n\r");
#endif
		return;
	}

	/*
	 * If the TX buffer is not full load it from the tx buffer
	 */
	if (!load_tx_buffer(uart)) {
		switch(uart) {
			case UART_1:
				/*
				 * Can't use U2STAbits.TRMT to detect when Tx Queue is empty
				 * as it ain't reliable at all.
				 */
				if (U1STAbits.UTXISEL0) {
					while (!U1STAbits.TRMT) {
						Nop();
					}

					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U1STAbits.UTXISEL1 = 1;
					U1STAbits.UTXISEL0 = 0;

                                        /*
                                         * Inform the higher layer we're finished
                                         */
                                        uarts[uart].data->tx_finished(uarts[uart].data);
				} else {
					/*
					 * Interrupt when the last character is shifted out of the Transmit
					 * Shift Register; all transmit operations are completed
					 */
					U1STAbits.UTXISEL1 = 0;
					U1STAbits.UTXISEL0 = 1;
				}
				break;

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
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U2STAbits.UTXISEL1 = 1;
					U2STAbits.UTXISEL0 = 0;

                                        /*
                                         * Inform the higher layer we're finished
                                         */
                                        uarts[uart].data->tx_finished(uarts[uart].data);
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
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U3STAbits.UTXISEL1 = 1;
					U3STAbits.UTXISEL0 = 0;

                                        /*
                                         * Inform the higher layer we're finished
                                         */
                                        uarts[uart].data->tx_finished(uarts[uart].data);
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
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U4STAbits.UTXISEL1 = 1;
					U4STAbits.UTXISEL0 = 0;

                                        /*
                                         * Inform the higher layer we're finished
                                         */
                                        uarts[uart].data->tx_finished(uarts[uart].data);
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
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				log_e(TAG, "Bad comm port given!\n\r");
#endif
				return;
				break;
		}
	}
}

/*
 * Receive Interrupt Service Routines
 */
void _ISR __attribute__((__no_auto_psv__)) _U1RXInterrupt(void)
{
	uint8_t ch;

	U1_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U1STAbits.OERR) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "RX Buffer overrun\n\r");
#endif
		U1STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U1STAbits.URXDA) {
		ch = U1RXREG;
		uarts[UART_1].data->process_rx_char(UART_1, ch);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
{
	uint8_t ch;

	U2_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U2STAbits.OERR) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "RX Buffer overrun\n\r");
#endif
		U2STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U2STAbits.URXDA) {
		ch = U2RXREG;
		uarts[UART_2].data->process_rx_char(UART_2, ch);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U3RXInterrupt(void)
{
	uint8_t ch;

	U3_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U3STAbits.OERR) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "RX Buffer overrun\n\r");
#endif
		U3STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U3STAbits.URXDA) {
		ch = U3RXREG;
		uarts[UART_3].data->process_rx_char(UART_3, ch);
	}
}

void _ISR __attribute__((__no_auto_psv__)) _U4RXInterrupt(void)
{
	uint8_t ch;

	U4_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U4STAbits.OERR) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "RX Buffer overrun\n\r");
#endif
		U4STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U4STAbits.URXDA) {
		ch = U4RXREG;
		uarts[UART_4].data->process_rx_char(UART_4, ch);
	}
}

result_t uart_calculate_mode(uint16_t *mode, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level)
{
	*mode = 0x00;

	*mode |= UARTEN_MASK;

	if (databits == 8) {
		if (parity == PARITY_EVEN) {
			*mode |= PDSEL0_MASK;
		} else if (parity == PARITY_ODD) {
			*mode |= PDSEL1_MASK;
		}
	} else if (databits == 9) {
		*mode |= PDSEL1_MASK;
		*mode |= PDSEL0_MASK;
	} else {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
                log_e(TAG, "Bad byte length\n\r");
#endif
		return(ERR_BAD_INPUT_PARAMETER);
	}

	if (stopbits == TWO_STOP_BITS) {
		*mode |= STSEL_MASK;
	}

	if (rx_idle_level == IDLE_HIGH) {
		*mode |= RXINV_MASK;
	}

	return(SUCCESS);
}

/*
 * Initialisation
 */
void uart_init(void)
{
	uint8_t loop;

	for(loop = 0; loop < NUM_UARTS; loop++) {
		uarts[loop].status = UART_FREE;
		uarts[loop].data = NULL;
	}
}

result_t uart_reserve(struct uart_data *data)
{
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "uart_reserve()\n\r");
#endif
	/*
	 * Find a free uart to use
	 */
	uint8_t  loop;

	for(loop = 0; loop < NUM_UARTS; loop++) {
		if(uarts[loop].status == UART_FREE) {

			uarts[loop].data = data;
			uarts[loop].status = UART_BUSY;

			data->uart = loop;

			uarts[loop].tx_write_index = 0;
			uarts[loop].tx_read_index = 0;
			uarts[loop].tx_count = 0;

			/*
			 * Set up the Rx & Tx pins
			 */
			uart_set_rx_pin((uint8_t) data->uart, data->rx_pin);
			uart_set_tx_pin((uint8_t) data->uart, data->tx_pin);

			uart_set_com_config(data);

			return(SUCCESS);
		}
	}

	return(ERR_NO_RESOURCES);
}

result_t uart_release(struct uart_data *data)
{
	uint8_t  uart;

	uart = data->uart;

#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	log_d(TAG, "uart_release()  %d\n\r", uart);
#endif
        
	if(uarts[uart].data != data) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		log_e(TAG, "uart_tx called with bad data pointer\n\r");
#endif
		return(ERR_BAD_INPUT_PARAMETER);
	}

	uarts[uart].data = NULL;
	uarts[uart].status = UART_FREE;

	switch (uart) {
		case UART_2:
			U2MODEbits.UARTEN = 0;
			U2_RX_ISR_ENABLE = 0;
			U2_TX_ISR_ENABLE = 0;
			break;

		case UART_3:
			U3MODEbits.UARTEN = 0;
			U3_RX_ISR_ENABLE = 0;
			U3_TX_ISR_ENABLE = 0;
			break;

		case UART_4:
			U4MODEbits.UARTEN = 0;
			U4_RX_ISR_ENABLE = 0;
			U4_TX_ISR_ENABLE = 0;
			break;
	}

	data->uart = UART_BAD;

	return(SUCCESS);
}

result_t uart_tx_buffer(struct uart_data *data, uint8_t *buffer, uint16_t len)
{
	uint8_t  uart;
	uint8_t *ptr;

	uart = data->uart;

	if(uarts[uart].data != data) {
		return(ERR_BAD_INPUT_PARAMETER);
	}

	ptr = buffer;

	while(len--) {
		uart_putchar(uart, *ptr++);
	}
	return(SUCCESS);
}

result_t uart_tx_char(struct uart_data *data, char ch)
{
	uint8_t  uart;

	uart = data->uart;

	if(uarts[uart].data != data) {
		return(ERR_BAD_INPUT_PARAMETER);
	}

        uart_putchar(uart, ch);
	return(SUCCESS);        
}

static void uart_putchar(uint8_t uart, uint8_t ch)
{
//	uint8_t loop;
        uint8_t tmp;

	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */
	switch(uart) {
		case UART_1:
			/*
			 * If either the TX Buffer is full OR there are already characters in
			 * our SW Buffer then add to SW buffer
			 */
			if(U1STAbits.UTXBF || uarts[uart].tx_count) {
				if (uarts[uart].tx_count == 0) {
					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U1STAbits.UTXISEL1 = 1;
					U1STAbits.UTXISEL0 = 0;
				}

				if(uarts[uart].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Circular buffer full!");
#endif
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_write_index = tmp;
				uarts[uart].tx_count++;
			} else {
				U1TXREG = ch;
			}
			break;

		case UART_2:
			/*
			 * If either the TX Buffer is full OR there are already characters in
			 * our SW Buffer then add to SW buffer
			 */
			if(U2STAbits.UTXBF || uarts[uart].tx_count) {
				if (uarts[uart].tx_count == 0) {
					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U2STAbits.UTXISEL1 = 1;
					U2STAbits.UTXISEL0 = 0;
				}

				if(uarts[uart].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Circular buffer full!");
#endif
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_write_index = tmp;
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
			if(U3STAbits.UTXBF || uarts[uart].tx_count) {
				if (uarts[uart].tx_count == 0) {
					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U3STAbits.UTXISEL1 = 1;
					U3STAbits.UTXISEL0 = 0;
				}

				if(uarts[uart].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Circular buffer full!");
#endif
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_write_index = tmp;
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
			if(U4STAbits.UTXBF || uarts[uart].tx_count) {
				if (uarts[uart].tx_count == 0) {
					/*
					 * Interrupt when a character is transferred to the Transmit Shift
					 * Register (TSR), and as a result, the transmit buffer becomes empty
					 */
					U4STAbits.UTXISEL1 = 1;
					U4STAbits.UTXISEL0 = 0;
				}

				if(uarts[uart].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#if (SYS_LOG_LEVEL <= LOG_ERROR)
					log_e(TAG, "Circular buffer full!");
#endif
					return;
				}

				uarts[uart].tx_buffer[uarts[uart].tx_write_index] = ch;
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_write_index = tmp;
				uarts[uart].tx_count++;
			} else {
				U4TXREG = ch;
			}
			break;

		default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Unrecognised UART in putchar()\n\r");
#endif
			break;
	}
}

static void uart_set_rx_pin(uint8_t uart, uint8_t pin)
{
//        AD1PCFGL = 0xffff;
	switch (pin) {
		case RP0:
                        AD1PCFGLbits.PCFG0 = 1;
			TRISBbits.TRISB0 = 1;
			break;

		case RP1:
                        AD1PCFGLbits.PCFG1 = 1;
			TRISBbits.TRISB1 = 1;
			break;

		case RP13:
                        AD1PCFGLbits.PCFG2 = 1;
			TRISBbits.TRISB2 = 1;
			break;

		case RP25:
			TRISDbits.TRISD4 = 1;
			break;

		case RP28:
                        AD1PCFGLbits.PCFG4 = 1;
			TRISBbits.TRISB4 = 1;
			break;

		default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Unknow Peripheral Rx Pin\n\r");
#endif
			break;
	}

	switch (uart) {
		case UART_1:
			RPINR18bits.U1RXR = pin;
			break;

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

static void uart_set_tx_pin(uint8_t uart, uint8_t pin)
{
	uint8_t tx_function;

	switch (uart) {
		case UART_1:
			tx_function = 3;
			break;

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
                        AD1PCFGLbits.PCFG0 = 1;
			TRISBbits.TRISB0 = 0;
			RPOR0bits.RP0R = tx_function;
			break;

		case RP1:
                        AD1PCFGLbits.PCFG1 = 1;
			TRISBbits.TRISB1 = 0;
			RPOR0bits.RP1R = tx_function;
			break;

		case RP13:
                        AD1PCFGLbits.PCFG2 = 1;
			TRISBbits.TRISB2 = 0;
			RPOR6bits.RP13R = tx_function;
			break;

		case RP25:
			TRISDbits.TRISD4 = 0;
			RPOR12bits.RP25R = tx_function;
			break;

		case RP28:
                        AD1PCFGLbits.PCFG4 = 1;
			TRISBbits.TRISB4 = 0;
			RPOR14bits.RP28R = tx_function;
			break;

		default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Unknow Peripheral Tx Pin\n\r");
#endif
			break;
	}
}

static void uart_set_com_config(struct uart_data *com)
{
	switch (com->uart) {
		case UART_1:
			U1MODE = com->uart_mode;
			U1MODEbits.UARTEN = 1;

			U1STA = 0x8410;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			U1STAbits.UTXISEL1 = 1;
			U1STAbits.UTXISEL0 = 0;

			/*
			 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
			 *
			 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
			 *
			 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
			 *
			 */
			U1BRG = ((SYS_CLOCK_FREQ / com->baud) / 16) - 1;
                        U1_RX_ISR_FLAG = 0;
                        U1_TX_ISR_FLAG = 0;
			U1_RX_ISR_ENABLE = 1;
			U1_TX_ISR_ENABLE = 1;
			break;

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
			U2BRG = ((SYS_CLOCK_FREQ / com->baud) / 16) - 1;
                        U2_RX_ISR_FLAG = 0;
                        U2_TX_ISR_FLAG = 0;
			U2_RX_ISR_ENABLE = 1;
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
			U3BRG = ((SYS_CLOCK_FREQ / com->baud) / 16) - 1;
                        U3_RX_ISR_FLAG = 0;
                        U3_TX_ISR_FLAG = 0;
			U3_RX_ISR_ENABLE = 1;
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
			U4BRG = ((SYS_CLOCK_FREQ / com->baud) / 16) - 1;
                        U4_RX_ISR_FLAG = 0;
                        U4_TX_ISR_FLAG = 0;
			U4_RX_ISR_ENABLE = 1;
			U4_TX_ISR_ENABLE = 1;
			break;

		default:
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			log_e(TAG, "Bad UART passed\n\r");
#endif
			break;
	}
}

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static uint16_t load_tx_buffer(uint8_t uart)
{
        uint8_t tmp;
        
	switch (uart) {
		case UART_1:
			/*
			 * If the TX buffer is not full load it from the circular buffer
			 */
			while ((!U1STAbits.UTXBF) && (uarts[uart].tx_count)) {
				U1TXREG = uarts[uart].tx_buffer[uarts[uart].tx_read_index];
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_read_index = tmp;
				uarts[uart].tx_count--;
			}

			return(uarts[uart].tx_count);
			break;

		case UART_2:
			/*
			 * If the TX buffer is not full load it from the circular buffer
			 */
			while ((!U2STAbits.UTXBF) && (uarts[uart].tx_count)) {
				U2TXREG = uarts[uart].tx_buffer[uarts[uart].tx_read_index];
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_read_index = tmp;
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
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_read_index = tmp;
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
                                /*
                                 * Compiler don't like following two lines in a oner
                                 */
                                tmp = ++(uarts[uart].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
				uarts[uart].tx_read_index = tmp;
				uarts[uart].tx_count--;
			}

			return(uarts[uart].tx_count);
			break;
	}

//	return(com->tx_buffer_size - com->tx_buffer_read_index);
#if (SYS_LOG_LEVEL <= LOG_ERROR)
	log_e(TAG, "load_tx_buffer() Bad UART\n\r");
#endif
	return(0);
}

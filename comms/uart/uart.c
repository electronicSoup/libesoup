/**
 *
 * \file libesoup/comms/uart.c
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
/*
 * XC8 Compiler warns about unused functions
 */
#if defined (__XC8)
#pragma warning disable 520
#endif

#define DEBUG_FILE TRUE

#include "libesoup_config.h"

#ifdef SYS_SERIAL_LOGGING
const char *TAG = "UART";
#endif

#include "libesoup/logger/serial_log.h"
#include "libesoup/utils/rand.h"

#include "libesoup/comms/uart/uart.h"

/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_CLOCK_FREQ
#error libesoup_config.h file should define the SYS_CLOCK_FREQ
#endif

#ifndef SYS_UART_TX_BUFFER_SIZE
#error libesoup_config.h file should define the SYS_UART_TX_BUFFER_SIZE
#endif

enum uart_status {
	UART_FREE,
	UART_RESERVED
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
static void uart_set_uart_config(struct uart_data *uart);
static void uart_putchar(uint8_t uart, uint8_t ch);

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static uint16_t load_tx_buffer(uint8_t uart);

/*
 * Interrupt Service Routines
 */
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U1TXInterrupt(void)
{
	while(U1_TX_ISR_FLAG) {
		uart_tx_isr(UART_1);
		U1_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
{
	while(U2_TX_ISR_FLAG) {
		uart_tx_isr(UART_2);
		U2_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U3TXInterrupt(void)
{
	while(U3_TX_ISR_FLAG) {
		uart_tx_isr(UART_3);
		U3_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U4TXInterrupt(void)
{
	while(U4_TX_ISR_FLAG) {
		uart_tx_isr(UART_4);
		U4_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

static void uart_tx_isr(uint8_t uart)
{
	if ((uarts[uart].data == NULL) || (uarts[uart].status != UART_RESERVED)) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("UART Null in ISR!\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		return;
	}

	/*
	 * If the TX buffer is not full load it from the tx buffer
	 */
	if (!load_tx_buffer(uart)) {
		switch(uart) {
#ifdef UART_1
		case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
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
				if(uarts[uart].data->tx_finished != NULL) {
					uarts[uart].data->tx_finished(uarts[uart].data);
				}
			} else {
				/*
				 * Interrupt when the last character is shifted out of the Transmit
				 * Shift Register; all transmit operations are completed
				 */
				U1STAbits.UTXISEL1 = 0;
				U1STAbits.UTXISEL0 = 1;
			}
#endif // MicroController Selection
			break;
#endif // UART_1
#ifdef UART_2
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
#endif // UART_2
#ifdef UART_3
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
#endif // UART_3
#ifdef UART_4
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
#endif // UART_4
		default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
			LOG_E("Bad comm port given!\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
			break;
		}
	}
}

/*
 * Receive Interrupt Service Routines
 */
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U1RXInterrupt(void)
{
	uint8_t ch;

	U1_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U1STAbits.OERR) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("RX Buffer overrun\n\r");
#endif
#endif // SYS_SERIAL_LOGGGING
		U1STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U1STAbits.URXDA) {
		ch = U1RXREG;
		uarts[UART_1].data->process_rx_char(UART_1, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
{
	uint8_t ch;

	U2_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U2STAbits.OERR) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("RX Buffer overrun\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		U2STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U2STAbits.URXDA) {
		ch = U2RXREG;
		uarts[UART_2].data->process_rx_char(UART_2, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U3RXInterrupt(void)
{
	uint8_t ch;

	U3_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U3STAbits.OERR) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("RX Buffer overrun\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		U3STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U3STAbits.URXDA) {
		ch = U3RXREG;
		uarts[UART_3].data->process_rx_char(UART_3, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
void _ISR __attribute__((__no_auto_psv__)) _U4RXInterrupt(void)
{
	uint8_t ch;

	U4_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U4STAbits.OERR) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("RX Buffer overrun\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		U4STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U4STAbits.URXDA) {
		ch = U4RXREG;
		uarts[UART_4].data->process_rx_char(UART_4, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

result_t uart_calculate_mode(uint16_t *mode, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level)
{
	*mode = 0x00;

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
	*mode |= UARTEN_MASK;

	if (databits == UART_8_DATABITS) {
		if (parity == UART_PARITY_EVEN) {
			*mode |= PDSEL0_MASK;
		} else if (parity == UART_PARITY_ODD) {
			*mode |= PDSEL1_MASK;
		}
	} else if (databits == UART_9_DATABITS) {
		*mode |= PDSEL1_MASK;
		*mode |= PDSEL0_MASK;
	} else {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
                LOG_E("Bad byte length\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		return(ERR_BAD_INPUT_PARAMETER);
	}

	if (stopbits == UART_TWO_STOP_BITS) {
		*mode |= STSEL_MASK;
	}

	if (rx_idle_level == UART_IDLE_LOW) {
		*mode |= RXINV_MASK;
	}
        
        /*
         * There's a different BAUD Rate formula for High or Low Baud rate.
         * Have to investigate and merge dsPIC33 and PIC24
         */
#if defined (__PIC24FJ256GB106__)
	*mode |= BRGH_MASK;  // High Speed Mode
#endif

#elif defined(__18F2680) || defined(__18F4585)
        /*
         * The PIC18 doesn't have a MODE Register but instead uses the TXSTA,
         * RCSTA and BAUDCON registers
         */
        TXSTAbits.SYNC = DISABLED;  // Asynchronous Mode
        
	if (databits == UART_8_DATABITS) {
		if (parity == UART_PARITY_EVEN) {
//			*mode |= PDSEL0_MASK;
		} else if (parity == UART_PARITY_ODD) {
//			*mode |= PDSEL1_MASK;
		}
	} else if (databits == UART_9_DATABITS) {
//		*mode |= PDSEL1_MASK;
//		*mode |= PDSEL0_MASK;
	} else {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
                LOG_E("Bad byte length\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		return(ERR_BAD_INPUT_PARAMETER);
	}

	if (stopbits == UART_TWO_STOP_BITS) {
                /*
                 * PIC18 only supports One Stop bit.
                 */
		return(ERR_BAD_INPUT_PARAMETER);
	}

	if (rx_idle_level == UART_IDLE_HIGH) {
//		*mode |= RXINV_MASK;
	}
#endif // MicroController Selection

	return(SUCCESS);
}

/*
 * Initialisation of the uart data structures
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
	/*
	 * Find a free uart to use
	 */
	uint8_t  loop;

	for(loop = UART_1; loop < NUM_UARTS; loop++) {
		if(uarts[loop].status == UART_FREE) {

			uarts[loop].data = data;
			uarts[loop].status = UART_RESERVED;

			data->uart = loop;

			uarts[loop].tx_write_index = 0;
			uarts[loop].tx_read_index = 0;
			uarts[loop].tx_count = 0;

			/*
			 * Set up the Rx & Tx pins
			 */
			if (data->rx_pin != NO_PIN) {
			    uart_set_rx_pin((uint8_t) data->uart, data->rx_pin);
			}

			if (data->tx_pin != NO_PIN) {
				uart_set_tx_pin((uint8_t) data->uart, data->tx_pin);
			}

			uart_set_uart_config(data);

			return(SUCCESS);
		}
	}

	return(ERR_NO_RESOURCES);
}

result_t uart_release(struct uart_data *data)
{
	uint8_t  uart_index;

	uart_index = data->uart;

#ifdef SYS_SERIAL_LOGGING
#if (DEBUG_FILE && (SYS_LOG_LEVEL <= LOG_DEBUG))
	LOG_D("uart_release()  %d\n\r", uart_index);
#endif
#endif // SYS_SERIAL_LOGGING
	if(uarts[uart_index].data != data) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("uart_tx called with bad data pointer\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		return(ERR_BAD_INPUT_PARAMETER);
	}

	uarts[uart_index].data = NULL;
	uarts[uart_index].status = UART_FREE;

	switch (uart_index) {
#ifdef UART_1
        case UART_1:
                U1_ENABLE = DISABLED;
                U1_RX_ISR_ENABLE = DISABLED;
                U1_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_1
#ifdef UART_2
        case UART_2:
                U2_ENABLE = DISABLED;
                U2_RX_ISR_ENABLE = DISABLED;
                U2_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_2
#ifdef UART_3
        case UART_3:
                U3_ENABLE = DISABLED;
                U3_RX_ISR_ENABLE = DISABLED;
                U3_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_3
#ifdef UART_4
        case UART_4:
                U4_ENABLE = DISABLED;
                U4_RX_ISR_ENABLE = DISABLED;
                U4_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_4
        default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Unrecognised UART\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		return(ERR_BAD_INPUT_PARAMETER);
//                break;
	}

	data->uart = UART_BAD;

	return(SUCCESS);
}

result_t uart_tx_buffer(struct uart_data *data, uint8_t *buffer, uint16_t len)
{
	uint8_t  uart_index;
	uint8_t *ptr;

	uart_index = data->uart;

	if(uarts[uart_index].data != data) {
		return(ERR_BAD_INPUT_PARAMETER);
	}

	ptr = buffer;

	while(len--) {
		uart_putchar(uart_index, *ptr++);
	}
	return(SUCCESS);
}

result_t uart_tx_char(struct uart_data *data, char ch)
{
	uint8_t  uart_index;

	uart_index = data->uart;

	if(uarts[uart_index].data != data) {
		return(ERR_BAD_INPUT_PARAMETER);
	}

        uart_putchar(uart_index, ch);
	return(SUCCESS);        
}

static void uart_putchar(uint8_t uart_index, uint8_t ch)
{
        uint8_t tmp;

	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */
	switch(uart_index) {
#ifdef UART_1
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U1STAbits.UTXBF || uarts[uart_index].tx_count) {
			if (uarts[uart_index].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U1STAbits.UTXISEL1 = 1;
				U1STAbits.UTXISEL0 = 0;
			}

			if(uarts[uart_index].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				LOG_E("Circular buffer full!");
#endif
#endif // SYS_SERIAL_LOGGING
				return;
			}

			uarts[uart_index].tx_buffer[uarts[uart_index].tx_write_index] = ch;
			/*
                         * Compiler don't like following two lines in a oner
                         */
			tmp = ++(uarts[uart_index].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
			uarts[uart_index].tx_write_index = tmp;
			uarts[uart_index].tx_count++;
		} else {
			U1TXREG = ch;
		}
#elif defined(__18F2680) || defined(__18F4585)
		if(uarts[uart_index].tx_count == SYS_UART_TX_BUFFER_SIZE) {
			return;
		}

		if((uarts[uart_index].tx_count == 0) && PIR1bits.TXIF) {
			TXREG = ch;
			return;
		}

		uarts[uart_index].tx_buffer[uarts[uart_index].tx_write_index] = ch;
		/*
                 * Compiler don't like following two lines in a oner
                 */
		tmp = ++(uarts[uart_index].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
		uarts[uart_index].tx_write_index = tmp;
		uarts[uart_index].tx_count++;
		PIE1bits.TXIE = ENABLED;
#endif // #if defined(__18F2680) || defined(__18F4585)
		break;
#endif // UART_1
#ifdef UART_2
	case UART_2:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U2STAbits.UTXBF || uarts[uart_index].tx_count) {
			if (uarts[uart_index].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U2STAbits.UTXISEL1 = 1;
				U2STAbits.UTXISEL0 = 0;
			}

			if(uarts[uart_index].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				LOG_E("Circular buffer full!");
#endif
#endif // SYS_SERIAL_LOGGING
				return;
			}

			uarts[uart_index].tx_buffer[uarts[uart_index].tx_write_index] = ch;
			/*
                         * Compiler don't like following two lines in a oner
                         */
			tmp = ++(uarts[uart_index].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
			uarts[uart_index].tx_write_index = tmp;
			uarts[uart_index].tx_count++;
		} else {
			U2TXREG = ch;
		}
		break;
#endif // UART_2
#ifdef UART_3
	case UART_3:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U3STAbits.UTXBF || uarts[uart_index].tx_count) {
			if (uarts[uart_index].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U3STAbits.UTXISEL1 = 1;
				U3STAbits.UTXISEL0 = 0;
			}

			if(uarts[uart_index].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				LOG_E("Circular buffer full!");
#endif
#endif // SYS_SERIAL_LOGGING
				return;
			}

			uarts[uart_index].tx_buffer[uarts[uart_index].tx_write_index] = ch;
			/*
                         * Compiler don't like following two lines in a oner
                         */
			tmp = ++(uarts[uart_index].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
			uarts[uart_index].tx_write_index = tmp;
			uarts[uart_index].tx_count++;
		} else {
			U3TXREG = ch;
		}
		break;
#endif // UART_3
#ifdef UART_4
	case UART_4:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U4STAbits.UTXBF || uarts[uart_index].tx_count) {
			if (uarts[uart_index].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U4STAbits.UTXISEL1 = 1;
				U4STAbits.UTXISEL0 = 0;
			}

			if(uarts[uart_index].tx_count == SYS_UART_TX_BUFFER_SIZE) {
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
				LOG_E("Circular buffer full!");
#endif
#endif // SYS_SERIAL_LOGGING
				return;
			}

			uarts[uart_index].tx_buffer[uarts[uart_index].tx_write_index] = ch;
			/*
                         * Compiler don't like following two lines in a oner
                         */
			tmp = ++(uarts[uart_index].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
			uarts[uart_index].tx_write_index = tmp;
			uarts[uart_index].tx_count++;
		} else {
			U4TXREG = ch;
		}
		break;
#endif // UART_4
	default:
//#if (SYS_LOG_LEVEL <= LOG_ERROR)
//			LOG_E("Unrecognised UART in putchar()\n\r");
//#endif
		break;
	}
}

#if defined(__dsPIC33EP256MU806__)
static void uart_set_rx_pin(uint8_t uart, uint8_t pin)
{
	switch (pin) {
	case RP120:
		ANSELGbits.ANSG8 = DIGITAL_PIN;
		TRISGbits.TRISG8 = INPUT_PIN;
		break;

	default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Unknow Peripheral Rx Pin\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		break;
        }

	switch (uart) {
	case UART_1:
		PPS_UART_1_RX = pin;
		break;

	case UART_2:
		PPS_UART_2_RX = pin;
		break;

	case UART_3:
		PPS_UART_3_RX = pin;
		break;

	case UART_4:
		PPS_UART_4_RX = pin;
		break;
	}
}
#elif defined (__PIC24FJ256GB106__)
static void uart_set_rx_pin(uint8_t uart, uint8_t pin)
{
	switch (pin) {
	case RP0:
		AD1PCFGLbits.PCFG0 = DIGITAL_PIN;
		TRISBbits.TRISB0 = INPUT_PIN;
		break;

	case RP1:
		AD1PCFGLbits.PCFG1 = DIGITAL_PIN;
		TRISBbits.TRISB1 = INPUT_PIN;
		break;

	case RP13:
		AD1PCFGLbits.PCFG2 = DIGITAL_PIN;
		TRISBbits.TRISB2 = INPUT_PIN;
		break;

	case RP25:
		TRISDbits.TRISD4 = INPUT_PIN;
		break;

	case RP28:
		AD1PCFGLbits.PCFG4 = DIGITAL_PIN;
		TRISBbits.TRISB4 = INPUT_PIN;
		break;

	default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Unknow Peripheral Rx Pin\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		break;
	}

	switch (uart) {
	case UART_1:
		PPS_UART_1_RX = pin;
		break;

	case UART_2:
		PPS_UART_2_RX = pin;
		break;

	case UART_3:
		PPS_UART_3_RX = pin;
		break;

	case UART_4:
		PPS_UART_4_RX = pin;
		break;
	}
}
#elif defined(__18F2680) || defined(__18F4585)
static void uart_set_rx_pin(uint8_t uart, uint8_t pin)
{
        TRISCbits.TRISC7 = INPUT_PIN;
}
#endif // MicroContoller Selection

#if defined(__dsPIC33EP256MU806__)
static void uart_set_tx_pin(uint8_t uart, uint8_t pin)
{
	uint8_t tx_function;

	switch (uart) {
	case UART_1:
		tx_function = PPS_UART_1_TX;
		break;

	case UART_2:
		tx_function = PPS_UART_2_TX;
		break;

	case UART_3:
		tx_function = PPS_UART_3_TX;
		break;

	case UART_4:
		tx_function = PPS_UART_4_TX;
		break;
	}

	switch (pin) {
	case RP64:
		TRISDbits.TRISD0 = OUTPUT_PIN;
		PPS_RP64 = tx_function;
		break;

	case RP118:
		ANSELGbits.ANSG6 = DIGITAL_PIN;
		TRISGbits.TRISG6 = OUTPUT_PIN;
		PPS_RP118 = tx_function;
		break;

	default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Unknow Peripheral Tx Pin\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		break;
	}
}
#elif defined (__PIC24FJ256GB106__)
static void uart_set_tx_pin(uint8_t uart, uint8_t pin)
{
	uint8_t tx_function;

	switch (uart) {
	case UART_1:
		tx_function = PPS_UART_1_TX;
		break;

	case UART_2:
		tx_function = PPS_UART_2_TX;
		break;

	case UART_3:
		tx_function = PPS_UART_3_TX;
		break;

	case UART_4:
		tx_function = PPS_UART_4_TX;
		break;
	}

	switch (pin) {
	case RP0:
		AD1PCFGLbits.PCFG0 = DIGITAL_PIN;
		TRISBbits.TRISB0 = OUTPUT_PIN;
		RPOR0bits.RP0R = tx_function;
		break;

	case RP1:
		AD1PCFGLbits.PCFG1 = DIGITAL_PIN;
		TRISBbits.TRISB1 = OUTPUT_PIN;
		RPOR0bits.RP1R = tx_function;
		break;

	case RP13:
		AD1PCFGLbits.PCFG2 = DIGITAL_PIN;
		TRISBbits.TRISB2 = OUTPUT_PIN;
		RPOR6bits.RP13R = tx_function;
		break;

	case RP20:
		TRISDbits.TRISD5 = OUTPUT_PIN;
		RPOR10bits.RP20R = tx_function;
		break;

	case RP25:
		TRISDbits.TRISD4 = OUTPUT_PIN;
		RPOR12bits.RP25R = tx_function;
		break;

	case RP28:
		AD1PCFGLbits.PCFG4 = DIGITAL_PIN;
		TRISBbits.TRISB4 = OUTPUT_PIN;
		RPOR14bits.RP28R = tx_function;
		break;
			
	default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Unknow Peripheral Tx Pin\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
		break;
	}
}
#elif defined(__18F2680) || defined(__18F4585)
static void uart_set_tx_pin(uint8_t uart, uint8_t pin)
{
        TRISCbits.TRISC6 = OUTPUT_PIN;
}
#endif // MicroContoller Selection

static void uart_set_uart_config(struct uart_data *uart)
{
	switch (uart->uart) {
#ifdef UART_1
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
		U1MODE = uart->uart_mode;

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
		U1BRG = ((SYS_CLOCK_FREQ / uart->baud) / 16) - 1;
		U1_RX_ISR_FLAG = 0;
		U1_TX_ISR_FLAG = 0;

		if (uart->rx_pin != NO_PIN) {
			U1_RX_ISR_ENABLE = ENABLED;
		} else {
			U1_RX_ISR_ENABLE = DISABLED;
		}
		
		if (uart->tx_pin != NO_PIN) {
			U1_TX_ISR_PRIOTITY = 0x07;
			U1_TX_ISR_ENABLE = ENABLED;
			U1STAbits.UTXEN = ENABLED;
			
		} else {
			U1_TX_ISR_ENABLE = DISABLED;
			U1STAbits.UTXEN = DISABLED;
		}

		U1_ENABLE = ENABLED;
#elif defined(__18F2680) || defined(__18F4585)
		TXSTAbits.TXEN = 1;    // Transmitter enabled
		TXSTAbits.SYNC = 0;    // Asynchronous mode
		TXSTAbits.BRGH = 0;    // High Baud Rate Select bit

#if defined(ENABLE_USART_RX)
		RCSTAbits.CREN = 1;    // Enable the Receiver
#else
		RCSTAbits.CREN = 0;    // Disable the Receiver
#endif
		BAUDCONbits.BRG16 = 0; // 16-bit Baud Rate Register Enable bit

		SPBRG = (unsigned char)(((SYS_CLOCK_FREQ / uart->baud) / 64 ) - 1);

		PIE1bits.TXIE = 1;
#if defined(ENABLE_USART_RX)
		PIR1bits.RCIF = 0;
		PIE1bits.RCIE = 1;
#endif // ENABLE_EUSART_RX
		RCSTAbits.SPEN = 1;
		TXREG = 'J';
#endif // MicroController Selection
		break;
#endif // UART_1
#ifdef UART_2
	case UART_2:
		U2MODE = uart->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U2STAbits.URXISEL = 0b10;

		/*
		 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
		 *
		 */
		U2BRG = ((SYS_CLOCK_FREQ / uart->baud) / 16) - 1;

		if (uart->rx_pin != NO_PIN) {
			U2_RX_ISR_ENABLE = ENABLED;
		} else {
			U2_RX_ISR_ENABLE = DISABLED;
		}
		
		if (uart->tx_pin != NO_PIN) {
			U2_TX_ISR_PRIOTITY = 0x07;
			U2_TX_ISR_ENABLE = ENABLED;
			U2STAbits.UTXEN = ENABLED;
			
		} else {
			U2_TX_ISR_ENABLE = DISABLED;
			U2STAbits.UTXEN = DISABLED;
		}

		U2_ENABLE = ENABLED;
		break;
#endif // UART_2
#ifdef UART_3
	case UART_3:
		U3MODE = uart->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U3STAbits.URXISEL = 0b10;

		/*
		 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
		 *
		 */
		U3BRG = ((SYS_CLOCK_FREQ / uart->baud) / 16) - 1;

		if (uart->rx_pin != NO_PIN) {
			U3_RX_ISR_ENABLE = ENABLED;
		} else {
			U3_RX_ISR_ENABLE = DISABLED;
		}
		
		if (uart->tx_pin != NO_PIN) {
			U3_TX_ISR_PRIOTITY = 0x07;
			U3_TX_ISR_ENABLE = ENABLED;
			U3STAbits.UTXEN = ENABLED;
			
		} else {
			U3_TX_ISR_ENABLE = DISABLED;
			U3STAbits.UTXEN = DISABLED;
		}

		U3_ENABLE = ENABLED;
		break;
#endif // UART_3
#ifdef UART_4
	case UART_4:
		U4MODE = uart->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U4STAbits.URXISEL = 0b10;

		/*
		 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 * UxBRG = ((CLOCK/MODBUS_BAUD)/16) -1
		 *
		 */
		U4BRG = ((SYS_CLOCK_FREQ / uart->baud) / 16) - 1;

		if (uart->rx_pin != NO_PIN) {
			U4_RX_ISR_ENABLE = ENABLED;
		} else {
			U4_RX_ISR_ENABLE = DISABLED;
		}
		
		if (uart->tx_pin != NO_PIN) {
			U4_TX_ISR_PRIOTITY = 0x07;
			U4_TX_ISR_ENABLE = ENABLED;
			U3STAbits.UTXEN = ENABLED;
			
		} else {
			U4_TX_ISR_ENABLE = DISABLED;
			U4STAbits.UTXEN = DISABLED;
		}

		U4_ENABLE = ENABLED;
		break;
#endif // UART_4
	default:
#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
		LOG_E("Bad UART passed\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
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
#ifdef UART_1
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
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
#elif defined(__18F2680) || defined(__18F4585)
		/*
                 * The TXIF Interrupt is cleared by writing to TXREG it
                 * cannot be cleared by SW directly.
                 */
		if(uarts[uart].tx_count > 0) {
			TXREG = uarts[uart].tx_buffer[uarts[uart].tx_read_index];
			/*
                         * Compiler don't like following two lines in a oner
                         */
			tmp = ++(uarts[uart].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
			uarts[uart].tx_read_index = tmp;
			uarts[uart].tx_count--;
		} else {
			PIE1bits.TXIE = DISABLED;
		}
		return(uarts[uart].tx_count);
#endif // MicroController selection
#ifndef __XC8
		break;
#endif
#endif // UART_1
#ifdef UART_2
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
#endif // UART_2
#ifdef UART_3
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
#endif // UART_3
#ifdef UART_4
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
#endif // UART_4
	}

#ifdef SYS_SERIAL_LOGGING
#if (SYS_LOG_LEVEL <= LOG_ERROR)
	LOG_E("load_tx_buffer() Bad UART\n\r");
#endif
#endif // SYS_SERIAL_LOGGING
	return(0);
}

#if defined(__18F2680) || defined(__18F4585)
void pic18f_uart_isr(void)
{
#if defined(ENABLE_USART_RX)
	uint8_t data;
#endif // (ENABLE_UASAT_RX)

	if(PIR1bits.TXIF) {
		uart_tx_isr(UART_1);
	}
#if defined(ENABLE_USART_RX)
	if(PIR1bits.RCIF) {
		data = RCREG;
	}
#endif
}
#endif // (__18F2680) || (__18F4585)

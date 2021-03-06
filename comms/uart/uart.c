/**
 * @file libesoup/comms/uart/uart.c
 *
 * @author John Whitmore
 *
 * @brief UART functionality for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017-2020 electronicSoup Limited
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

#include "libesoup_config.h"

#if defined(SYS_UART1) || defined(SYS_UART2)

#ifdef SYS_SERIAL_LOGGING
#define DEBUG_FILE
static const char *TAG = "UART";
#include "libesoup/logger/serial_log.h"
/*
 * Check required libesoup_config.h defines are found
 */
#ifndef SYS_LOG_LEVEL
#error libesoup_config.h file should define SYS_LOG_LEVEL (see libesoup/examples/libesoup_config.h)
#endif
#endif

#include "libesoup/errno.h"
#include "libesoup/gpio/gpio.h"
#include "libesoup/utils/rand.h"
#include "libesoup/comms/uart/uart.h"
#include "libesoup/timers/time.h"
#ifdef SYS_UART_TEST_RESPONSE
#include "libesoup/timers/sw_timers.h"
#endif

/*
 * Check required libesoup_config.h defines are found
 */
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
	struct uart_data      *udata;
	uint8_t                tx_buffer[SYS_UART_TX_BUFFER_SIZE];
	uint16_t               tx_write_index;
	uint16_t               tx_read_index;
	uint16_t               tx_count;
} uart;

struct uart uarts[NUM_UART_CHANNELS];

/*
 * Local static Function prototypes
 */
static void     uart_tx_isr(enum uart_channel channel);

static result_t uart_set_rx_pin(enum uart_channel channel, enum gpio_pin pin);
static result_t uart_set_tx_pin(enum uart_channel channel, enum gpio_pin pin);
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)
static result_t uart_set_uart_config(struct uart_data *udata);
#else
extern result_t uart_set_uart_config(struct uart_data *udata);
#endif
static result_t uart_putchar(enum uart_channel channel, uint8_t ch);

static result_t tx_buffer_write(enum uart_channel channel, char ch);
static char     tx_buffer_read (enum uart_channel channel);

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static uint16_t load_tx_buffer(enum uart_channel channel);

/*
 * Interrupt Service Routines
 */
#if defined(SYS_UART1)
#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)) || defined(__dsPIC33EP256GP502__) && defined(SYS_UART1)
void _ISR __attribute__((__no_auto_psv__)) _U1TXInterrupt(void)
{
#ifdef SYS_TEST_BUILD
//	gpio_toggle_output(RA0);
#endif
	while(U1_TX_ISR_FLAG) {
		uart_tx_isr(UART_1);
		U1_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)
#endif // defined(SYS_UART1)

#if defined(SYS_UART2)
#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)) && defined(SYS_UART2) || defined(__dsPIC33EP256GP502__)
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
{
	while(U2_TX_ISR_FLAG) {
		uart_tx_isr(UART_2);
		U2_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)
#endif // SYS_UART_2

#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)) && defined(SYS_UART3)
void _ISR __attribute__((__no_auto_psv__)) _U3TXInterrupt(void)
{
	while(U3_TX_ISR_FLAG) {
		uart_tx_isr(UART_3);
		U3_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)) && defined(SYS_UART4)
void _ISR __attribute__((__no_auto_psv__)) _U4TXInterrupt(void)
{
	while(U4_TX_ISR_FLAG) {
		uart_tx_isr(UART_4);
		U4_TX_ISR_FLAG = 0;
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

static void uart_tx_isr(enum uart_channel channel)
{
 	result_t rc;

 	if((channel >= NUM_UART_CHANNELS) || (uarts[channel].status != UART_RESERVED) || (uarts[channel].udata == NULL)) {
 		// Todo - Possibly call global status handler with error?
		LOG_E("UART Null in ISR!\n\r");
		return;
	}

	/*
	 * If the uC TX buffer is not full load it from the SW tx buffer
	 */
 	rc = load_tx_buffer(channel);
 	if(rc < 0) {
 		LOG_E("load_tx_buffer()!\n\r");
 		return;
 	}
 	if (rc == 0) {
		switch(channel) {
#if defined(SYS_UART1)
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
				if(uarts[channel].udata->tx_finished != NULL)
					uarts[channel].udata->tx_finished(uarts[channel].udata);
			} else {
				/*
				 * Interrupt when the last character is shifted out of the Transmit
				 * Shift Register; all transmit operations are completed
				 */
				U1STAbits.UTXISEL1 = 0;
				U1STAbits.UTXISEL0 = 1;
			}
			break;
#endif // SYS_UART1
#if defined(SYS_UART2)
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
 				if(uarts[channel].udata->tx_finished != NULL)
                                        uarts[channel].udata->tx_finished(uarts[channel].udata);
			} else {
				/*
				 * Interrupt when the last character is shifted out of the Transmit
				 * Shift Register; all transmit operations are completed
				 */
				U2STAbits.UTXISEL1 = 0;
				U2STAbits.UTXISEL0 = 1;
			}
			break;
#endif // SYS_UART2
#if defined(SYS_UART3)
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
 				if(uarts[channel].udata->tx_finished != NULL)
                                        uarts[channel].udata->tx_finished(uarts[channel].udata);
			} else {
				/*
				 * Interrupt when the last character is shifted out of the Transmit
				 * Shift Register; all transmit operations are completed
				 */
				U3STAbits.UTXISEL1 = 0;
				U3STAbits.UTXISEL0 = 1;
			}
			break;
#endif // SYS_UART3
#if defined(SYS_UART4)
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
 				if(uarts[channel].udata->tx_finished != NULL)
                                        uarts[channel].udata->tx_finished(uarts[channel].udata);
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
 			LOG_E("Comm port?\n\r");
			break;
		}
	}
}

/*
 * Receive Interrupt Service Routines
 */
//#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)) &&
#if defined(SYS_UART1)
void _ISR __attribute__((__no_auto_psv__)) _U1RXInterrupt(void)
{
	uint8_t ch;

	U1_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U1STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U1STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U1STAbits.URXDA) {
		ch = U1RXREG;
		uarts[UART_1].udata->process_rx_char(UART_1, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)

//#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)) &&
#if defined(SYS_UART2)
void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
{
	uint8_t ch;

	U2_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (U2STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U2STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U2STAbits.URXDA) {
		ch = U2RXREG;
		if (uarts[UART_2].udata->process_rx_char) {
			uarts[UART_2].udata->process_rx_char(UART_2, ch);
		}
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)

//#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)) &&
#if defined(SYS_UART3)
void _ISR __attribute__((__no_auto_psv__)) _U3RXInterrupt(void)
{
	uint8_t ch;

	U3_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

        LOG_D("U3RX\n\r");
	if (U3STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U3STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U3STAbits.URXDA) {
		ch = U3RXREG;
		uarts[UART_3].udata->process_rx_char(UART_3, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

//#if (defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)) &&
#if defined(SYS_UART4)
void _ISR __attribute__((__no_auto_psv__)) _U4RXInterrupt(void)
{
	uint8_t ch;

	U4_RX_ISR_FLAG = 0;

	asm ("CLRWDT");

        LOG_D("U4RX\n\r");
	if (U4STAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		U4STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U4STAbits.URXDA) {
		ch = U4RXREG;
		uarts[UART_4].udata->process_rx_char(UART_4, ch);
	}
}
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

result_t uart_calculate_mode(uint16_t *mode, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level)
{
	*mode = 0x00;

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__) || defined(__dsPIC33EP256GP502__)
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
                LOG_E("Bad byte length\n\r");
		return(-ERR_BAD_INPUT_PARAMETER);
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
//	*mode |= BRGH_MASK;  // High Speed Mode

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
                LOG_E("Bad byte length\n\r");
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	if (stopbits == UART_TWO_STOP_BITS) {
                /*
                 * PIC18 only supports One Stop bit.
                 */
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	if (rx_idle_level == UART_IDLE_HIGH) {
//		*mode |= RXINV_MASK;
	}
#endif // MicroController Selection

	return(0);
}

/*
 * Initialisation of the uart data structures
 */
result_t uart_init(void)
{
	enum uart_channel channel;

	for(channel = 0; channel < NUM_UART_CHANNELS; channel++) {
		uarts[channel].status = UART_FREE;
		uarts[channel].udata  = NULL;
	}

        return(0);
}

#ifdef SYS_TEST_BUILD
result_t uart_tx_buffer_count(struct uart_data *udata)
{
	enum uart_channel channel;

	channel = udata->channel;

 	if(channel >= NUM_UART_CHANNELS)
 		return(-ERR_BAD_INPUT_PARAMETER);

 	if(uarts[channel].status != UART_RESERVED)
 		return(-ERR_BAD_INPUT_PARAMETER);

 	if(uarts[channel].udata != udata)
 		return(-ERR_BAD_INPUT_PARAMETER);

	return(uarts[channel].tx_count);
}
#endif // SYS_TEST_BUILD

/*
 * uart_reserve - Reserve a UART Channel for future use by the caller.
 */
result_t uart_reserve(struct uart_data *udata)
{
	/*
	 * Find a free uart to use
	 */
	result_t  rc;
	enum uart_channel channel;

	for(channel = 0; channel < NUM_UART_CHANNELS; channel++) {
		if(uarts[channel].status == UART_FREE) {
 			uarts[channel].udata          = udata;           // Keep track of calling structure
 			uarts[channel].status         = UART_RESERVED;   // Mark the UART as reserved

 			uarts[channel].tx_write_index = 0;               // Reset the counters
			uarts[channel].tx_read_index  = 0;
			uarts[channel].tx_count       = 0;

 			udata->channel = channel;                         // Store the uart channel being used, for future reference.

			/*
			 * Set up the Rx & Tx pins
> 			 * Bit strange to have neither Rx or Tx pin return error
			 */
 			if ((udata->rx_pin == INVALID_GPIO_PIN) && (udata->tx_pin == INVALID_GPIO_PIN)) {
 				return(-ERR_BAD_INPUT_PARAMETER);
			}
			if (udata->rx_pin != INVALID_GPIO_PIN) {
				rc = uart_set_rx_pin(channel, udata->rx_pin);
				if (rc < 0) {
					return(rc);
				}
			}

			if (udata->tx_pin != INVALID_GPIO_PIN) {
				rc = uart_set_tx_pin(channel, udata->tx_pin);
				RC_CHECK
			}

			rc = uart_set_uart_config(udata);
                        if (rc < 0) {
				return(rc);
			}

			return(channel);
		}
	}

	return(-ERR_NO_RESOURCES);
}

result_t uart_release(struct uart_data *udata)
{
	enum uart_channel channel;

	channel = udata->channel;

 	if (  (channel >= NUM_UART_CHANNELS)
 	    ||(uarts[channel].status != UART_RESERVED)
	    ||(uarts[channel].udata != udata)) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

 	udata->channel            = UART_BAD;
	uarts[channel].udata      = NULL;
	uarts[channel].status     = UART_FREE;

	switch (channel) {
#if defined(SYS_UART1)
        case UART_1:
                U1_ENABLE        = DISABLED;
                U1_RX_ISR_ENABLE = DISABLED;
                U1_TX_ISR_ENABLE = DISABLED;
                break;
#endif // SYS_UART1
#if defined(SYS_UART2)
        case UART_2:
                U2_ENABLE        = DISABLED;
                U2_RX_ISR_ENABLE = DISABLED;
                U2_TX_ISR_ENABLE = DISABLED;
                break;
#endif // SYS_UART2
#if defined(UART_3)
        case UART_3:
                U3_ENABLE        = DISABLED;
                U3_RX_ISR_ENABLE = DISABLED;
                U3_TX_ISR_ENABLE = DISABLED;
                break;
#endif // SYS_UART3
#if defined(UART_4)
        case UART_4:
                U4_ENABLE        = DISABLED;
                U4_RX_ISR_ENABLE = DISABLED;
                U4_TX_ISR_ENABLE = DISABLED;
                break;
#endif // SYS_UART4
        default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	udata->channel = UART_BAD;

	return(SUCCESS);
}

#ifdef SYS_TEST_BUILD
result_t uart_test_rx_buffer(struct uart_data *udata, uint8_t *buffer, uint16_t len)
{
	enum uart_channel channel;
	uint8_t          *ptr;
        int16_t           count = 0;

	channel = udata->channel;

 	if(  (channel >= NUM_UART_CHANNELS)
	   ||(uarts[channel].status != UART_RESERVED)
	   ||(uarts[channel].udata != udata)
	   ||(udata->rx_pin == INVALID_GPIO_PIN)) {
 		return(-ERR_BAD_INPUT_PARAMETER);
	}

	ptr = buffer;

	while(len--) {
		uarts[channel].udata->process_rx_char(channel, *ptr++);
                count++;
	}

	return(count);
}
#endif // SYS_TEST_BUILD

result_t uart_tx_buffer(struct uart_data *udata, uint8_t *buffer, uint16_t len)
{
	enum uart_channel channel;
	uint8_t          *ptr;
	result_t          rc = 0;
        int16_t           count = 0;

	channel = udata->channel;

 	if(  (channel >= NUM_UART_CHANNELS)
	   ||(uarts[channel].status != UART_RESERVED)
	   ||(uarts[channel].udata != udata)
	   ||(udata->tx_pin == INVALID_GPIO_PIN)) {
 		return(-ERR_BAD_INPUT_PARAMETER);
	}

	ptr = buffer;

	while(len--) {
		rc = uart_putchar(channel, *ptr++);
                RC_CHECK
                count++;
	}

	return(count);
}

result_t uart_tx_char(struct uart_data *udata, char ch)
{
	enum uart_channel channel;

	channel = udata->channel;

 	if(channel >= NUM_UART_CHANNELS)
 		return(-ERR_BAD_INPUT_PARAMETER);
        if(uarts[channel].udata != udata) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

        return(uart_putchar(channel, ch));
}

static result_t uart_putchar(enum uart_channel channel, uint8_t ch)
{
 	if(channel >= NUM_UART_CHANNELS)
 		return(-ERR_BAD_INPUT_PARAMETER);
 	if(uarts[channel].status != UART_RESERVED)
 		return(-ERR_BAD_INPUT_PARAMETER);
 	if(uarts[channel].udata->tx_pin == INVALID_GPIO_PIN)
 		return(-ERR_BAD_INPUT_PARAMETER);
	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */
	switch(channel) {
#if defined(SYS_UART1)
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__) || defined(__dsPIC33EP256GP502__)
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U1STAbits.UTXBF || uarts[channel].tx_count) {
			if (uarts[channel].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U1STAbits.UTXISEL1 = 1;
				U1STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(channel, ch));
		} else {
			U1TXREG = ch;
		}
#elif defined(__18F2680) || defined(__18F4585)
		if((uarts[channel].tx_count == 0) && PIR1bits.TXIF) {
			TXREG = ch;
			return;
		}

		PIE1bits.TXIE = ENABLED;

		return(tx_buffer_write(channel, ch));
#endif // #if defined(__18F2680) || defined(__18F4585)
#if !defined(__XC8)
		break;
#endif
#endif // SYS_UART1
#if defined(SYS_UART2)
	case UART_2:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U2STAbits.UTXBF || uarts[channel].tx_count) {
			if (uarts[channel].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U2STAbits.UTXISEL1 = 1;
				U2STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(channel, ch));
		} else {
			U2TXREG = ch;
		}
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U3STAbits.UTXBF || uarts[channel].tx_count) {
			if (uarts[channel].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U3STAbits.UTXISEL1 = 1;
				U3STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(channel, ch));
		} else {
			U3TXREG = ch;
		}
		break;
#endif // SYS_UART3
#if defined(UART4)
	case UART_4:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U4STAbits.UTXBF || uarts[channel].tx_count) {
			if (uarts[channel].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U4STAbits.UTXISEL1 = 1;
				U4STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(channel, ch));
		} else {
			U4TXREG = ch;
		}
		break;
#endif // UART_4
	default:
#ifndef __XC8 // XC8 compiler don't like recursion
			LOG_E("Unrecognised UART in putchar()\n\r");
#endif // __XC8
		break;
	}

	return(0);
}

static result_t tx_buffer_write(enum uart_channel channel, char ch)
{
	uint16_t tmp;

	/*
	 * Todo - This potential buffer overflow should be handled
	 *        Might change the function to return an error code
	 */
	if(uarts[channel].tx_count < SYS_UART_TX_BUFFER_SIZE) {
		uarts[channel].tx_buffer[uarts[channel].tx_write_index] = ch;

		/*
                 * Compiler don't like following two lines in a oner
                 */
		tmp = ++(uarts[channel].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
		uarts[channel].tx_write_index = tmp;
		uarts[channel].tx_count++;
		return(0);
	}

	return(-ERR_BUFFER_OVERFLOW);
}

static char tx_buffer_read(enum uart_channel channel)
{
	uint16_t tmp;
	char     ch = 0x00;

	if(uarts[channel].tx_count > 0) {
		ch = uarts[channel].tx_buffer[uarts[channel].tx_read_index];
		/*
	         * Compiler don't like following two lines in a oner
	         */
		tmp = ++(uarts[channel].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
		uarts[channel].tx_read_index = tmp;
		uarts[channel].tx_count--;
	}
	return(ch);
}

#if defined(__dsPIC33EP256MU806__)
static result_t uart_set_rx_pin(enum uart_channel channel, enum gpio_pin pin)
{
	int16_t rc;

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK

	rc = set_peripheral_input(pin);
	RC_CHECK

	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		PPS_I_UART_1_RX = rc;
		break;
#endif // SYS_UART1
#if defined(SYS_UART2)
	case UART_2:
		PPS_I_UART_2_RX = rc;
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		PPS_I_UART_3_RX = rc;
		break;
#endif // SYS_UART3
#if defined(SYS_UART4)
	case UART_4:
		PPS_I_UART_4_RX = rc;
		break;
#endif // SYS_UART4
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(0);
}
#elif defined(__dsPIC33EP128GS702__) || defined(__dsPIC33EP256GP502__)
static result_t uart_set_rx_pin(enum uart_channel channel, enum gpio_pin pin)
{
	int16_t rc;

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK

	rc = set_peripheral_input(pin);
	RC_CHECK

	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		PPS_I_UART_1_RX = rc;
		break;
#endif // SYS_UART1
#if defined(SYS_UART2)
	case UART_2:
		PPS_I_UART_2_RX = rc;
		break;
#endif // SYS_UART2
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}
	return(SUCCESS);
}
#elif defined (__PIC24FJ256GB106__)
static result_t uart_set_rx_pin(enum uart_channel channel, enum gpio_pin pin)
{
	result_t rc;

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK

	rc = set_peripheral_input(pin);
	RC_CHECK

	rc = SUCCESS;
	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		PPS_I_UART_1_RX = rc;
		break;
#endif
#if defined(SYS_UART2)
	case UART_2:
		PPS_I_UART_2_RX = rc;
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		PPS_I_UART_3_RX = rc;
		break;
#endif // SYS_UART3
#if defined(SYS_UART4)
	case UART_4:
		PPS_I_UART_4_RX = rc;
		break;
#endif // SYS_UART4
	default:
		rc = -ERR_BAD_INPUT_PARAMETER;
		break;
	}

	return(rc);
}
#elif defined(__18F2680) || defined(__18F4585)
static result_t uart_set_rx_pin(enum uart_channel channel, enum gpio_pin pin)
{
//        TRISCbits.TRISC7 = INPUT_PIN;
}
#endif // MicroContoller Selection

#if defined(__dsPIC33EP256MU806__)
static result_t uart_set_tx_pin(enum uart_channel channel, enum gpio_pin pin)
{
	result_t rc;
	uint16_t tx_function;

	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		tx_function = PPS_O_UART_1_TX;
		break;
#endif // SYS_UART1
#if defined(SYS_UART2)
	case UART_2:
		tx_function = PPS_O_UART_2_TX;
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		tx_function = PPS_O_UART_3_TX;
		break;
#endif // SYS_UART3
#if defined(SYS_UART4)
	case UART_4:
		tx_function = PPS_O_UART_4_TX;
		break;
#endif // SYS_UART4
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = set_peripheral_output(pin, tx_function);
	RC_CHECK;

	return(SUCCESS);
}
#elif defined(__dsPIC33EP128GS702__) || defined(__dsPIC33EP256GP502__)
static result_t uart_set_tx_pin(enum uart_channel channel, enum gpio_pin pin)
{
	result_t rc;
	uint16_t tx_function;

	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		tx_function = PPS_O_UART_1_TX;
		break;
#endif // SYS_UART2
#if defined(SYS_UART2)
	case UART_2:
		tx_function = PPS_O_UART_2_TX;
		break;
#endif // SYS_UART2
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = set_peripheral_output(pin, tx_function);
	RC_CHECK;

	return(SUCCESS);
}
#elif defined (__PIC24FJ256GB106__)
static result_t uart_set_tx_pin(enum uart_channel channel, enum gpio_pin pin)
{
	result_t   rc;
	uint8_t    tx_function = 0x00;

	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		tx_function = PPS_O_UART_1_TX;
		break;
#endif // SYS_UART 1
#if defined(SYS_UART2)
	case UART_2:
		tx_function = PPS_O_UART_2_TX;
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		tx_function = PPS_O_UART_3_TX;
		break;
#endif // SYS_UART3
#if defined(SYS_UART4)
	case UART_4:
		tx_function = PPS_O_UART_4_TX;
		break;
#endif // SYS_UART4
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = set_peripheral_output(pin, tx_function);
	RC_CHECK;

	return(SUCCESS);
}
#elif defined(__18F2680) || defined(__18F4585)
static result_t uart_set_tx_pin(enum uart_channel channel, enum gpio_pin pin)
{
        TRISCbits.TRISC6 = GPIO_OUTPUT_PIN;
	return(SUCCESS);
}
#endif // MicroContoller Selection

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__)
static result_t uart_set_uart_config(struct uart_data *udata)
{
	enum uart_channel channel;

        channel = udata->channel;
	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
		U1MODE = udata->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U1STAbits.UTXISEL1 = 1;
		U1STAbits.UTXISEL0 = 0;

		/*
		 * with BRGH = 0 Slow Speed Mode:
		 *        Baudrate - between  sys_clock_freq /(16 * 65536) and sys_clock_freq / 16
		 *                   For sys_clock_freq = 16,000,000 Min 15bps Max 1Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 3bps Max 250Kbps
		 *
		 *                   Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 *                   UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 *                   UxBRG = ((CLOCK / BAUD)/16) -1
		 *
		 *
		 * with BRGH = 1 : sys_clock_freq /(4 * 65536) <= Baudrate <= sys_clock_freq / 4
		 *                   For sys_clock_freq = 16,000,000 Min 61bps Max 4Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 15bps Max 1Mbps
		 */
		if(udata->uart_mode & BRGH_MASK) {
			/*
			 * Hight Speed Mode:
			 */
			U1BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 4) - 1;
		} else {
			/*
			 * Standard Mode:
			 */
			U1BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 16) - 1;
		}

		U1_RX_ISR_FLAG             = 0;
		U1_TX_ISR_FLAG             = 0;

		if (udata->rx_pin != INVALID_GPIO_PIN) {
                        U1STAbits.URXISEL  = 0b00;
			U1_RX_ISR_ENABLE   = ENABLED;
		} else {
			U1_RX_ISR_ENABLE   = DISABLED;
		}

		if (udata->tx_pin != INVALID_GPIO_PIN) {
			U1_TX_ISR_PRIOTITY = 0x07;
			U1_TX_ISR_ENABLE   = ENABLED;
			U1STAbits.UTXEN    = ENABLED;

		} else {
			U1_TX_ISR_ENABLE   = DISABLED;
			U1STAbits.UTXEN    = DISABLED;
		}

		U1_ENABLE = ENABLED;
		break;
#endif // SYS_UART1

#if defined(SYS_UART2)
	case UART_2:
		U2MODE = udata->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U2STAbits.URXISEL = 0b10;

		/*
		 * with BRGH = 0 Slow Speed Mode:
		 *        Baudrate - between  sys_clock_freq /(16 * 65536) and sys_clock_freq / 16
		 *                   For sys_clock_freq = 16,000,000 Min 15bps Max 1Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 3bps Max 250Kbps
		 *
		 *                   Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 *                   UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 *                   UxBRG = ((CLOCK / BAUD)/16) -1
		 *
		 *
		 * with BRGH = 1 : sys_clock_freq /(4 * 65536) <= Baudrate <= sys_clock_freq / 4
		 *                   For sys_clock_freq = 16,000,000 Min 61bps Max 4Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 15bps Max 1Mbps
		 */
		if(udata->uart_mode & BRGH_MASK) {
			/*
			 * Hight Speed Mode:
			 */
			U2BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 4) - 1;
		} else {
			/*
			 * Standard Mode:
			 */
			U2BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 16) - 1;
		}


		if (udata->rx_pin != INVALID_GPIO_PIN) {
                        U2STAbits.URXISEL  = 0b00;
			U2_RX_ISR_ENABLE   = ENABLED;
		} else {
			U2_RX_ISR_ENABLE   = DISABLED;
		}

		if (udata->tx_pin != INVALID_GPIO_PIN) {
			U2_TX_ISR_PRIOTITY = 0x07;
			U2_TX_ISR_ENABLE   = ENABLED;
			U2STAbits.UTXEN    = ENABLED;

		} else {
			U2_TX_ISR_ENABLE   = DISABLED;
			U2STAbits.UTXEN    = DISABLED;
		}

		U2_ENABLE                  = ENABLED;
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		U3MODE = udata->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U3STAbits.URXISEL = 0b10;

		/*
		 * with BRGH = 0 Slow Speed Mode:
		 *        Baudrate - between  sys_clock_freq /(16 * 65536) and sys_clock_freq / 16
		 *                   For sys_clock_freq = 16,000,000 Min 15bps Max 1Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 3bps Max 250Kbps
		 *
		 *                   Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 *                   UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 *                   UxBRG = ((CLOCK / BAUD)/16) -1
		 *
		 *
		 * with BRGH = 1 : sys_clock_freq /(4 * 65536) <= Baudrate <= sys_clock_freq / 4
		 *                   For sys_clock_freq = 16,000,000 Min 61bps Max 4Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 15bps Max 1Mbps
		 */
		if(udata->uart_mode & BRGH_MASK) {
			/*
			 * Hight Speed Mode:
			 */
			U3BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 4) - 1;
		} else {
			/*
			 * Standard Mode:
			 */
			U3BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 16) - 1;
		}


		if (udata->rx_pin != INVALID_GPIO_PIN) {
                        U3STAbits.URXISEL  = 0b00;
			U3_RX_ISR_ENABLE   = ENABLED;
		} else {
			U3_RX_ISR_ENABLE   = DISABLED;
		}

		if (udata->tx_pin != INVALID_GPIO_PIN) {
			U3_TX_ISR_PRIOTITY = 0x07;
			U3_TX_ISR_ENABLE   = ENABLED;
			U3STAbits.UTXEN    = ENABLED;

		} else {
			U3_TX_ISR_ENABLE   = DISABLED;
			U3STAbits.UTXEN    = DISABLED;
		}

		U3_ENABLE                  = ENABLED;
		break;
#endif // SYS_UART3
#if defined(SYS_UART4)
	case UART_4:
		U4MODE = udata->uart_mode;

		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		U4STAbits.URXISEL = 0b10;

		/*
		 * with BRGH = 0 Slow Speed Mode:
		 *        Baudrate - between  sys_clock_freq /(16 * 65536) and sys_clock_freq / 16
		 *                   For sys_clock_freq = 16,000,000 Min 15bps Max 1Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 3bps Max 250Kbps
		 *
		 *                   Desired Baud Rate = FCY/(16 (UxBRG + 1))
		 *
		 *                   UxBRG = ((FCY/Desired Baud Rate)/16) - 1
		 *
		 *                   UxBRG = ((CLOCK / BAUD)/16) -1
		 *
		 *
		 * with BRGH = 1 : sys_clock_freq /(4 * 65536) <= Baudrate <= sys_clock_freq / 4
		 *                   For sys_clock_freq = 16,000,000 Min 61bps Max 4Mbps
		 *                   For sys_clock_freq =  4,000,000 Min 15bps Max 1Mbps
		 */
		if(udata->uart_mode & BRGH_MASK) {
			/*
			 * Hight Speed Mode:
			 */
			U4BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 4) - 1;
		} else {
			/*
			 * Standard Mode:
			 */
			U4BRG = (uint16_t)((uint32_t)((uint32_t)sys_clock_freq / (uint32_t)udata->baud) / 16) - 1;
		}


		if (udata->rx_pin != INVALID_GPIO_PIN) {
                        U4STAbits.URXISEL  = 0b00;
			U4_RX_ISR_ENABLE   = ENABLED;
		} else {
			U4_RX_ISR_ENABLE   = DISABLED;
		}

		if (udata->tx_pin != INVALID_GPIO_PIN) {
			U4_TX_ISR_PRIOTITY = 0x07;
			U4_TX_ISR_ENABLE   = ENABLED;
			U3STAbits.UTXEN    = ENABLED;

		} else {
			U4_TX_ISR_ENABLE   = DISABLED;
			U4STAbits.UTXEN    = DISABLED;
		}

		U4_ENABLE                  = ENABLED;
		break;
#endif // SYS_UART4
	default:
 		return(-ERR_BAD_INPUT_PARAMETER);
	}
        return(SUCCESS);
}
#endif  // defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__18F2680) || defined(__18F4585)
static void uart_set_uart_config(struct uart_data *udata)
{
	switch (udata->uindex) {
#ifdef UART_1
	case UART_1:
		TXSTAbits.TXEN = 1;    // Transmitter enabled
		TXSTAbits.SYNC = 0;    // Asynchronous mode
		TXSTAbits.BRGH = 0;    // High Baud Rate Select bit

#if defined(SYS_ENABLE_USART_RX)
		RCSTAbits.CREN = 1;    // Enable the Receiver
#else
		RCSTAbits.CREN = 0;    // Disable the Receiver
#endif
		BAUDCONbits.BRG16 = 0; // 16-bit Baud Rate Register Enable bit

		SPBRG = (unsigned char)(((sys_clock_freq / udata->baud) / 64 ) - 1);

		PIE1bits.TXIE = 1;
#if defined(SYS_ENABLE_USART_RX)
		PIR1bits.RCIF = 0;
		PIE1bits.RCIE = 1;
#endif // ENABLE_EUSART_RX
		RCSTAbits.SPEN = 1;
		break;
#endif // UART_1

	default:
		LOG_E("Bad UART passed\n\r");
		break;
	}
}
#endif // defined(__18F2680) || defined(__18F4585)

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static uint16_t load_tx_buffer(enum uart_channel channel)
{
	switch (channel) {
#if defined(SYS_UART1)
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__) || defined(__dsPIC33EP128GS702__) || defined(__dsPIC33EP256GP502__)
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U1STAbits.UTXBF) && (uarts[channel].tx_count)) {
			U1TXREG = tx_buffer_read(channel);
		}

		return(uarts[channel].tx_count);
#elif defined(__18F2680) || defined(__18F4585)
		/*
                 * The TXIF Interrupt is cleared by writing to TXREG it
                 * cannot be cleared by SW directly.
                 */
		if(uarts[channel].tx_count > 0) {
			TXREG = tx_buffer_read(channel);
		} else {
			PIE1bits.TXIE = DISABLED;
		}
		return(uarts[channel].tx_count);
#endif // MicroController selection
#ifndef __XC8
		break;
#endif // __XC8
#endif // SYS_UART1
#if defined(SYS_UART2)
	case UART_2:
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U2STAbits.UTXBF) && (uarts[channel].tx_count)) {
			U2TXREG = tx_buffer_read(channel);
		}

		return(uarts[channel].tx_count);
		break;
#endif // SYS_UART2
#if defined(SYS_UART3)
	case UART_3:
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U3STAbits.UTXBF) && (uarts[channel].tx_count)) {
			U3TXREG = tx_buffer_read(channel);
		}

		return(uarts[channel].tx_count);
		break;
#endif // SYS_UART3
#if defined(SYS_UART4)
	case UART_4:
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U4STAbits.UTXBF) && (uarts[channel].tx_count)) {
			U4TXREG = tx_buffer_read(channel);
		}

		return(uarts[channel].tx_count);
		break;
#endif // SYS_UART4
	default:
 		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}
	return(SUCCESS);
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

#endif // defined(SYS_UART1) || defined(SYS_UART2)

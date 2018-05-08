/**
 * @file libesoup/comms/uart/uart.c
 *
 * @author John Whitmore
 * 
 * @brief UART functionalty for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017-2018 electronicSoup Limited
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

#ifdef SYS_UART

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

struct uart uarts[NUM_UARTS];

/*
 * Local static Function prototypes
 */
static void     uart_tx_isr(uint8_t uindex);

static result_t uart_set_rx_pin(uint8_t uindex, enum pin_t pin);
static result_t uart_set_tx_pin(uint8_t uindex, enum pin_t pin);
static result_t uart_set_uart_config(struct uart_data *udata);
static result_t uart_putchar(uint8_t uindex, uint8_t ch);

static result_t tx_buffer_write(uint8_t uindex, char ch);
static char     tx_buffer_read (uint8_t uindex);

/*
 * Returns the number of bytes still waiting to be loaded in HW TX Buffer.
 */
static uint16_t load_tx_buffer(uint8_t uindex);

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

static void uart_tx_isr(uint8_t uindex)
{
 	result_t rc;
 	
 	if((uindex >= NUM_UARTS) || (uarts[uindex].status != UART_RESERVED) || (uarts[uindex].udata == NULL)) {
 		// Todo - Possibly call global status handler with error?
		LOG_E("UART Null in ISR!\n\r");
		return;
	}

	/*
	 * If the uC TX buffer is not full load it from the SW tx buffer
	 */
 	rc = load_tx_buffer(uindex);
 	if(rc < 0) {
 		LOG_E("load_tx_buffer()!\n\r");
 		return;
 	}
 	if (rc == 0) {
		switch(uindex) {
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
				if(uarts[uindex].udata->tx_finished != NULL)
					uarts[uindex].udata->tx_finished(uarts[uindex].udata);
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
 				if(uarts[uindex].udata->tx_finished != NULL) 
                                        uarts[uindex].udata->tx_finished(uarts[uindex].udata);
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
 				if(uarts[uindex].udata->tx_finished != NULL) 
                                        uarts[uindex].udata->tx_finished(uarts[uindex].udata);
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
 				if(uarts[uindex].udata->tx_finished != NULL) 
                                        uarts[uindex].udata->tx_finished(uarts[uindex].udata);
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
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
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
#endif // #if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
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
		uarts[UART_2].udata->process_rx_char(UART_2, ch);
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
		LOG_E("RX Buffer overrun\n\r");
		U3STAbits.OERR = 0;   /* Clear the error flag */
	}

	while (U3STAbits.URXDA) {
		ch = U3RXREG;
		uarts[UART_3].udata->process_rx_char(UART_3, ch);
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
	uint8_t uindex;

	for(uindex = UART_1; uindex < NUM_UARTS; uindex++) {
		uarts[uindex].status = UART_FREE;
		uarts[uindex].udata = NULL;
	}
        
        return(0);
}

#ifdef SYS_DEBUG_BUILD
result_t uart_tx_buffer_count(struct uart_data *udata)
{
	uint8_t  uindex;

	uindex = udata->uindex;

 	if(uindex >= NUM_UARTS)
 		return(-ERR_BAD_INPUT_PARAMETER);
 
 	if(uarts[uindex].status != UART_RESERVED)
 		return(-ERR_BAD_INPUT_PARAMETER);
 
 	if(uarts[uindex].udata != udata) 
 		return(-ERR_BAD_INPUT_PARAMETER);
        
	return(uarts[uindex].tx_count);
}
#endif // SYS_DEBUG_BUILD

/*
 * uart_reserve - Reserve a UART Channel for future use by the caller.
 */
result_t uart_reserve(struct uart_data *udata)
{
	/*
	 * Find a free uart to use
	 */
	result_t  rc;
	uint8_t   uindex;

	for(uindex = UART_1; uindex < NUM_UARTS; uindex++) {
		if(uarts[uindex].status == UART_FREE) {

 			uarts[uindex].udata          = udata;           // Keep track of calling structure
 			uarts[uindex].status         = UART_RESERVED;   // Mark the UART as reserved

 			uarts[uindex].tx_write_index = 0;               // Reset the counters
			uarts[uindex].tx_read_index  = 0;
			uarts[uindex].tx_count       = 0;

 			udata->uindex = uindex;                         // Store the uart index for future reference.

			/*
			 * Set up the Rx & Tx pins
> 			 * Bit strange to have neither Rx or Tx pin return error
			 */
 			if ((udata->rx_pin == INVALID_PIN) && (udata->tx_pin == INVALID_PIN))
 				return(-ERR_BAD_INPUT_PARAMETER);
			if (udata->rx_pin != INVALID_PIN) {
				rc = uart_set_rx_pin(uindex, udata->rx_pin);
				RC_CHECK
			}

			if (udata->tx_pin != INVALID_PIN) {
				rc = uart_set_tx_pin(uindex, udata->tx_pin);
				RC_CHECK
			}

			rc = uart_set_uart_config(udata);
                        RC_CHECK

			return(uindex);
		}
	}

	return(-ERR_NO_RESOURCES);
}

result_t uart_release(struct uart_data *udata)
{
	uint8_t  uindex;

	uindex = udata->uindex;

 	if(uindex >= NUM_UARTS)
 		return(-ERR_BAD_INPUT_PARAMETER);
 
 	if(uarts[uindex].status != UART_RESERVED)
 		return(-ERR_BAD_INPUT_PARAMETER);
 
	if(uarts[uindex].udata != udata) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

 	udata->uindex            = UART_BAD; 
	uarts[uindex].udata      = NULL;
	uarts[uindex].status     = UART_FREE;

	switch (uindex) {
#ifdef UART_1
        case UART_1:
                U1_ENABLE        = DISABLED;
                U1_RX_ISR_ENABLE = DISABLED;
                U1_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_1
#ifdef UART_2
        case UART_2:
                U2_ENABLE        = DISABLED;
                U2_RX_ISR_ENABLE = DISABLED;
                U2_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_2
#ifdef UART_3
        case UART_3:
                U3_ENABLE        = DISABLED;
                U3_RX_ISR_ENABLE = DISABLED;
                U3_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_3
#ifdef UART_4
        case UART_4:
                U4_ENABLE        = DISABLED;
                U4_RX_ISR_ENABLE = DISABLED;
                U4_TX_ISR_ENABLE = DISABLED;
                break;
#endif // UART_4
        default:
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	udata->uindex = UART_BAD;

	return(0);
}

result_t uart_tx_buffer(struct uart_data *udata, uint8_t *buffer, uint16_t len)
{
	uint8_t   uindex;
	uint8_t  *ptr;
	result_t  rc = 0;
        int16_t   count = 0;

	uindex = udata->uindex;

 	if(uindex >= NUM_UARTS)
 		return(-ERR_BAD_INPUT_PARAMETER);
 
 	if(uarts[uindex].status != UART_RESERVED)
 		return(-ERR_BAD_INPUT_PARAMETER);
 
 	if(uarts[uindex].udata != udata) 
 		return(-ERR_BAD_INPUT_PARAMETER);
 
 	if(udata->tx_pin == INVALID_PIN)
 		return(-ERR_BAD_INPUT_PARAMETER);

	if(uarts[uindex].udata != udata) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

	ptr = buffer;

	while(len--) {
		rc = uart_putchar(uindex, *ptr++);
                RC_CHECK
                count++;
	}
	return(count);
}

result_t uart_tx_char(struct uart_data *udata, char ch)
{
	uint8_t  uindex;

	uindex = udata->uindex;

 	if(uindex >= NUM_UARTS)
 		return(-ERR_BAD_INPUT_PARAMETER);
        if(uarts[uindex].udata != udata) {
		return(-ERR_BAD_INPUT_PARAMETER);
	}

        return(uart_putchar(uindex, ch));
}

static result_t uart_putchar(uint8_t uindex, uint8_t ch)
{
 	if(uindex >= NUM_UARTS)
 		return(-ERR_BAD_INPUT_PARAMETER);
 	if(uarts[uindex].status != UART_RESERVED)
 		return(-ERR_BAD_INPUT_PARAMETER); 
 	if(uarts[uindex].udata->tx_pin == INVALID_PIN)
 		return(-ERR_BAD_INPUT_PARAMETER);
	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */
	switch(uindex) {
#ifdef UART_1
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U1STAbits.UTXBF || uarts[uindex].tx_count) {
			if (uarts[uindex].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U1STAbits.UTXISEL1 = 1;
				U1STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(uindex, ch));
		} else {
			U1TXREG = ch;
		}
#elif defined(__18F2680) || defined(__18F4585)
		if((uarts[uindex].tx_count == 0) && PIR1bits.TXIF) {
			TXREG = ch;
			return;
		}

		PIE1bits.TXIE = ENABLED;

		return(tx_buffer_write(uindex, ch));
#endif // #if defined(__18F2680) || defined(__18F4585)
#if !defined(__XC8)
		break;
#endif
#endif // UART_1
#ifdef UART_2
	case UART_2:
		/*
		 * If either the TX Buffer is full OR there are already characters in
		 * our SW Buffer then add to SW buffer
		 */
		if(U2STAbits.UTXBF || uarts[uindex].tx_count) {
			if (uarts[uindex].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U2STAbits.UTXISEL1 = 1;
				U2STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(uindex, ch));
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
		if(U3STAbits.UTXBF || uarts[uindex].tx_count) {
			if (uarts[uindex].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U3STAbits.UTXISEL1 = 1;
				U3STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(uindex, ch));
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
		if(U4STAbits.UTXBF || uarts[uindex].tx_count) {
			if (uarts[uindex].tx_count == 0) {
				/*
				 * Interrupt when a character is transferred to the Transmit Shift
				 * Register (TSR), and as a result, the transmit buffer becomes empty
				 */
				U4STAbits.UTXISEL1 = 1;
				U4STAbits.UTXISEL0 = 0;
			}

			return(tx_buffer_write(uindex, ch));
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

static result_t tx_buffer_write(uint8_t uindex, char ch)
{
	uint16_t tmp;

	/*
	 * Todo - This potential buffer overflow should be handled
	 *        Might change the function to return an error code
	 */	
	if(uarts[uindex].tx_count < SYS_UART_TX_BUFFER_SIZE) {
		uarts[uindex].tx_buffer[uarts[uindex].tx_write_index] = ch;

		/*
                 * Compiler don't like following two lines in a oner
                 */
		tmp = ++(uarts[uindex].tx_write_index) % SYS_UART_TX_BUFFER_SIZE;
		uarts[uindex].tx_write_index = tmp;
		uarts[uindex].tx_count++;
		return(0);
	}

	return(-ERR_BUFFER_OVERFLOW);
}

static char tx_buffer_read(uint8_t uindex)
{
	uint16_t tmp;
	char     ch = 0x00;
	
	if(uarts[uindex].tx_count > 0) {
		ch = uarts[uindex].tx_buffer[uarts[uindex].tx_read_index];
		/*
	         * Compiler don't like following two lines in a oner
	         */
		tmp = ++(uarts[uindex].tx_read_index) % SYS_UART_TX_BUFFER_SIZE;
		uarts[uindex].tx_read_index = tmp;
		uarts[uindex].tx_count--;
	}
	return(ch);
}

#if defined(__dsPIC33EP256MU806__)
static result_t uart_set_rx_pin(uint8_t uindex, enum pin_t pin)
{
	int16_t rc;
	
	rc = gpio_set(pin, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK

	rc = set_peripheral_input(pin);
	RC_CHECK

	switch (uindex) {
	case UART_1:
		PPS_I_UART_1_RX = rc;
		break;

	case UART_2:
		PPS_I_UART_2_RX = rc;
		break;

	case UART_3:
		PPS_I_UART_3_RX = rc;
		break;

	case UART_4:
		PPS_I_UART_4_RX = rc;
		break;
	}
	return(0);
}
#elif defined (__PIC24FJ256GB106__)
static result_t uart_set_rx_pin(uint8_t uindex, enum pin_t pin)
{
	result_t rc;
	
	rc = gpio_set(pin, GPIO_MODE_DIGITAL_INPUT, 0);
	RC_CHECK

	rc = set_peripheral_input(pin);
	RC_CHECK

	switch (uindex) {
	case UART_1:
		PPS_I_UART_1_RX = rc;
		break;

	case UART_2:
		PPS_I_UART_2_RX = rc;
		break;

	case UART_3:
		PPS_I_UART_3_RX = rc;
		break;

	case UART_4:
		PPS_I_UART_4_RX = rc;
		break;
	}
	
	return(0);
}
#elif defined(__18F2680) || defined(__18F4585)
static result_t uart_set_rx_pin(uint8_t uindex, enum pin_t pin)
{
//        TRISCbits.TRISC7 = INPUT_PIN;
}
#endif // MicroContoller Selection

#if defined(__dsPIC33EP256MU806__)
static result_t uart_set_tx_pin(uint8_t uindex, enum pin_t pin)
{
	result_t rc;
	uint16_t tx_function;

	switch (uindex) {
	case UART_1:
		tx_function = PPS_O_UART_1_TX;
		break;

	case UART_2:
		tx_function = PPS_O_UART_2_TX;
		break;

	case UART_3:
		tx_function = PPS_O_UART_3_TX;
		break;

	case UART_4:
		tx_function = PPS_O_UART_4_TX;
		break;
	default:
		return(-ERR_BAD_INPUT_PARAMETER);
		break;
	}

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = set_peripheral_output(pin, tx_function);
	RC_CHECK;

	return(0);
}
#elif defined (__PIC24FJ256GB106__)
static result_t uart_set_tx_pin(uint8_t uindex, enum pin_t pin)
{
	result_t   rc;
	uint8_t    tx_function = 0x00;

	switch (uindex) {
	case UART_1:
		tx_function = PPS_O_UART_1_TX;
		break;

	case UART_2:
		tx_function = PPS_O_UART_2_TX;
		break;

	case UART_3:
		tx_function = PPS_O_UART_3_TX;
		break;

	case UART_4:
		tx_function = PPS_O_UART_4_TX;
		break;
		
	default:
		LOG_E("Bad input parameters\n\r");
		break;
	}

	rc = gpio_set(pin, GPIO_MODE_DIGITAL_OUTPUT, 1);
	RC_CHECK;

	rc = set_peripheral_output(pin, tx_function);
	RC_CHECK;
	
	return(0);
}
#elif defined(__18F2680) || defined(__18F4585)
static result_t uart_set_tx_pin(uint8_t uindex, enum pin_t pin)
{
        TRISCbits.TRISC6 = GPIO_OUTPUT_PIN;
	return(0);
}
#endif // MicroContoller Selection

#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
static result_t uart_set_uart_config(struct uart_data *udata)
{
        uint8_t uindex;
        
        uindex = udata->uindex; 
	switch (uindex) {
#ifdef UART_1
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

		if (udata->rx_pin != INVALID_PIN) {
			U1_RX_ISR_ENABLE   = ENABLED;
		} else {
			U1_RX_ISR_ENABLE   = DISABLED;
		}
		
		if (udata->tx_pin != INVALID_PIN) {
			U1_TX_ISR_PRIOTITY = 0x07;
			U1_TX_ISR_ENABLE   = ENABLED;
			U1STAbits.UTXEN    = ENABLED;
			
		} else {
			U1_TX_ISR_ENABLE   = DISABLED;
			U1STAbits.UTXEN    = DISABLED;
		}

		U1_ENABLE = ENABLED;
		break;
#endif // UART_1

#ifdef UART_2
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


		if (udata->rx_pin != INVALID_PIN) {
			U2_RX_ISR_ENABLE   = ENABLED;
		} else {
			U2_RX_ISR_ENABLE   = DISABLED;
		}
		
		if (udata->tx_pin != INVALID_PIN) {
			U2_TX_ISR_PRIOTITY = 0x07;
			U2_TX_ISR_ENABLE   = ENABLED;
			U2STAbits.UTXEN    = ENABLED;
			
		} else {
			U2_TX_ISR_ENABLE   = DISABLED;
			U2STAbits.UTXEN    = DISABLED;
		}

		U2_ENABLE                  = ENABLED;
		break;
#endif // UART_2
#ifdef UART_3
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


		if (udata->rx_pin != INVALID_PIN) {
			U3_RX_ISR_ENABLE   = ENABLED;
		} else {
			U3_RX_ISR_ENABLE   = DISABLED;
		}
		
		if (udata->tx_pin != INVALID_PIN) {
			U3_TX_ISR_PRIOTITY = 0x07;
			U3_TX_ISR_ENABLE   = ENABLED;
			U3STAbits.UTXEN    = ENABLED;
			
		} else {
			U3_TX_ISR_ENABLE   = DISABLED;
			U3STAbits.UTXEN    = DISABLED;
		}

		U3_ENABLE                  = ENABLED;
		break;
#endif // UART_3
#ifdef UART_4
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


		if (udata->rx_pin != INVALID_PIN) {
			U4_RX_ISR_ENABLE   = ENABLED;
		} else {
			U4_RX_ISR_ENABLE   = DISABLED;
		}
		
		if (udata->tx_pin != INVALID_PIN) {
			U4_TX_ISR_PRIOTITY = 0x07;
			U4_TX_ISR_ENABLE   = ENABLED;
			U3STAbits.UTXEN    = ENABLED;
			
		} else {
			U4_TX_ISR_ENABLE   = DISABLED;
			U4STAbits.UTXEN    = DISABLED;
		}

		U4_ENABLE                  = ENABLED;
		break;
#endif // UART_4
	default:
		LOG_E("Bad UART passed\n\r");
 		return(-ERR_BAD_INPUT_PARAMETER);
	}
        return(0);
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
static uint16_t load_tx_buffer(uint8_t uindex)
{
	switch (uindex) {
#ifdef UART_1
	case UART_1:
#if defined(__dsPIC33EP256MU806__) || defined (__PIC24FJ256GB106__)
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U1STAbits.UTXBF) && (uarts[uindex].tx_count)) {
			U1TXREG = tx_buffer_read(uindex);
		}

		return(uarts[uindex].tx_count);
#elif defined(__18F2680) || defined(__18F4585)
		/*
                 * The TXIF Interrupt is cleared by writing to TXREG it
                 * cannot be cleared by SW directly.
                 */
		if(uarts[uindex].tx_count > 0) {
			TXREG = tx_buffer_read(uindex);
		} else {
			PIE1bits.TXIE = DISABLED;
		}
		return(uarts[uindex].tx_count);
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
		while ((!U2STAbits.UTXBF) && (uarts[uindex].tx_count)) {
			U2TXREG = tx_buffer_read(uindex);
		}

		return(uarts[uindex].tx_count);
		break;
#endif // UART_2
#ifdef UART_3
	case UART_3:
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U3STAbits.UTXBF) && (uarts[uindex].tx_count)) {
			U3TXREG = tx_buffer_read(uindex);
		}

		return(uarts[uindex].tx_count);
		break;
#endif // UART_3
#ifdef UART_4
	case UART_4:
		/*
		 * If the TX buffer is not full load it from the circular buffer
		 */
		while ((!U4STAbits.UTXBF) && (uarts[uindex].tx_count)) {
			U4TXREG = tx_buffer_read(uindex);
		}

		return(uarts[uindex].tx_count);
		break;
#endif // UART_4
	}

	LOG_E("load_tx_buffer() Bad UART\n\r");
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

#endif // SYS_UART

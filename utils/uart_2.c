/**
 *
 * \file es_lib/utils/uart_2.c
 *
 * Functions for using a second uart port.
 *
 * The first uart port is used by the logger. See es_lib/logger
 *
 * Copyright 2015 John Whitmore <jwhitmore@electronicsoup.com>
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
 */
#include "system.h"

#define DEBUG_FILE
#include "es_lib/logger/serial_log.h"
#include "es_lib/timers/hw_timers.h"

#define TAG "UART-2"
#define UART_2

#ifdef UART_2_RX
static u8     rx_buffer[UART_2_RX_BUFFER_SIZE];
static UINT16 rx_write_index = 0;
//static u8     rx_line_complete = 0;
//static UINT16 rx_read_index = 0;
//static UINT16 rx_buffer_count = 0;

static void (*line_process)(u8 *line, u16 len) = NULL;

#endif // UART_2_RX
//static u8 watch_trmt = FALSE;

/*
 * Tx Buffer is a circular buffer
 */
#ifdef UART_2_TX
static u8     tx_buffer[UART_2_TX_BUFFER_SIZE];
static UINT16 tx_write_index = 0;
static UINT16 tx_read_index = 0;
static UINT16 tx_count = 0;
#endif // UART_2_TX

#ifdef UART_2
#define UxSTA         U2STA
#define UxSTAbits     U2STAbits
#define UxMODE        U2MODE
#define UxMODEbits    U2MODEbits
#define UxBRG         U2BRG
#define RX_ISR_FLAG   IFS1bits.U2RXIF
#define TX_ISR_FLAG   IFS1bits.U2TXIF
#define RX_ISR_ENABLE IEC1bits.U2RXIE
#define TX_ISR_ENABLE IEC1bits.U2TXIE
#define UxTXREG       U2TXREG
#define UxRXREG       U2RXREG
#elif UART_3
#define UxSTAbits     U3STAbits
#define UxMODE        U3MODE
#define UxMODEbits    U3MODEbits
#define UxSTA         U3STA
#define UxBRG         U3BRG
#define RX_ISR_FLAG   IFS5bits.U3RXIF
#define TX_ISR_FLAG   IFS5bits.U3TXIF
#define RX_ISR_ENABLE IEC5bits.U3RXIE
#define TX_ISR_ENABLE IEC5bits.U3TXIE
#define UxTXREG       U3TXREG
#define UxRXREG       U3RXREG
#elif defined(UART_4)
#define UxSTAbits     U4STAbits
#define UxMODE        U4MODE
#define UxMODEbits    U4MODEbits
#define UxSTA         U4STA
#define UxBRG         U4BRG
#define RX_ISR_FLAG   IFS5bits.U4RXIF
#define TX_ISR_FLAG   IFS5bits.U4TXIF
#define RX_ISR_ENABLE IEC5bits.U4RXIE
#define TX_ISR_ENABLE IEC5bits.U4TXIE
#define UxTXREG       U4TXREG
#define UxRXREG       U4RXREG
#endif

//#define TEST
#ifdef TEST
static u8 test_line[] = "DE,Ensen 4250,ID,1263897715,MO,4250,TI,42467.611111,BV,12.9,LE,0.005600,VE,0.02134,VSI,0,VSP,0,FL,0.000,VO,16402000,FV,16402000,RV,0.000000,SV,16402000,SS,1,B0,42440.643461,B0,42440.643414,B0,42440.640694,CS,11438";

u16 strlen(u8 *str) {
	u8 *ptr = str;
	u16 count = 0;

	while(*ptr++) count++;
	return(count);
}
#endif

#ifdef UART_2_TIME_END_OF_LINE
static void hw_eol_expiry_function(void)
{
	LOG_D("EOL Timer expired\n\r");

	if (line_process) {
#ifdef TEST
		line_process(test_line, strlen(test_line));
#else
		line_process(rx_buffer, rx_write_index);
#endif
	} else {
		LOG_D("No line processor function defined\n\r");
	}
	rx_write_index = 0;
}

static void eol_start_timer()
{
	u8  hw_timer;

//	LOG_D("eol_start_timer()\n\r");

	hw_timer = hw_timer_start(mSeconds, UART_2_EOL_TIMER_MS, FALSE, hw_eol_expiry_function);
}
#endif // UART_2_TIME_END_OF_LINE

#ifdef UART_2_RX
void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
{
	u8 ch;

//	putchar('R');
	RX_ISR_FLAG = 0;
#ifdef UART_2_TIME_END_OF_LINE
	eol_start_timer();
#endif // UART_2_TIME_END_OF_LINE

	if (UxSTAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
	}

	while (UxSTAbits.URXDA) {
		ch = UxRXREG;
#ifdef UART_2_CR_END_OF_LINE
		if(ch == '\r') {
			rx_buffer[rx_write_index++] = '\0';
			if (line_process) {
				line_process(rx_buffer, rx_write_index);
			} else {
				LOG_D("No line processor function defined\n\r");
			}
			rx_write_index = 0;
		} else {
			rx_buffer[rx_write_index++] = ch;
		}
#endif
#ifdef UART_2_TIME_END_OF_LINE
//		LOG_D("Rx*0x%x*\n\r", ch);
		rx_buffer[rx_write_index++] = ch;
#endif
		if(rx_write_index == UART_2_RX_BUFFER_SIZE) {
			LOG_E("UART 2 Overflow: Line too long\n\r");
		}
	}
}
#endif //UART_2_RX

#ifdef UART_2_TX
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
{
//	putchar('T');
	
	/*
	 * If the TX buffer is not full load it from the circular buffer
	 */
	while ((!UxSTAbits.UTXBF) && (tx_count)) {
		UxTXREG = tx_buffer[tx_read_index];
		tx_read_index = (++tx_read_index % UART_2_TX_BUFFER_SIZE);
		tx_count--;
	}

	if (!tx_count) {
		/*
		 */
		if (UxSTAbits.UTXISEL0) {
			while (!UxSTAbits.TRMT) {
				Nop();
			}

			/*
			 * Transmission of message finished.
			 */
//			putchar('*');
//			TX_ISR_ENABLE = 0;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (TSR), and as a result, the transmit buffer becomes empty
			 */
			UxSTAbits.UTXISEL1 = 1;
			UxSTAbits.UTXISEL0 = 0;

			/*
			 * Interrupt when a character is transferred to the Transmit Shift
			 * Register (this implies there is at least one character open in
			 * the transmit buffer)
			 */
//			UxSTAbits.UTXISEL1 = 0;
//			UxSTAbits.UTXISEL0 = 0;
		} else {
//			putchar('-');
			/*
			 * Interrupt when the last character is shifted out of the Transmit
			 * Shift Register; all transmit operations are completed
			 */
			UxSTAbits.UTXISEL1 = 0;
			UxSTAbits.UTXISEL0 = 1;
		}
	}
	TX_ISR_FLAG = 0;
}
#endif //UART_2_TX

void uart_2_init(void (*line_fn)(u8 *line, u16 len))
{
	/*
	 * CinnamonBun is running a PIC24FJ256GB106 processor
	 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)
#ifdef UART_2_RX
	line_process = line_fn;

	rx_write_index  = 0;

	switch (UART_2_RX_PIN) {
		case RP0:
			TRISBbits.TRISB0 = 1;
			RPINR19bits.U2RXR = 0;
			break;

		case RP1:
			TRISBbits.TRISB1 = 1;
			RPINR19bits.U2RXR = 1;
			break;
	}
#endif

#ifdef UART_2_TX
	tx_write_index = 0;
	tx_read_index = 0;
	tx_count = 0;

	switch (UART_2_TX_PIN) {
		case RP0:
			TRISBbits.TRISB0 = 0;
			RPOR0bits.RP0R = 5;
			break;

		case RP1:
			TRISBbits.TRISB1 = 0;
			RPOR0bits.RP1R = 5;
			break;
	}
#endif
	/*
	 * Serial Port pin configuration should be defined
	 * in include file system.h
	 */
	UxMODE = 0x8800;
//	UxMODEbits.LPBACK = 1;

	if (UART_2_DATA_BITS == 8) {
		if (UART_2_PARITY == PARITY_NONE) {
			UxMODEbits.PDSEL = 0b00;
		} else if (UART_2_PARITY == PARITY_EVEN) {
			UxMODEbits.PDSEL = 0b01;
		} else if (UART_2_PARITY == PARITY_ODD) {
			UxMODEbits.PDSEL = 0b10;
		}
	} else if (UART_2_DATA_BITS == 9) {
		UxMODEbits.PDSEL = 0b11;
	} else {
		LOG_E("Unrecognised Data bit/Parity configuration\n\r");
	}

	if (UART_2_STOP_BITS == ONE_STOP_BIT) {
		UxMODEbits.STSEL = 0;
	} else if (UART_2_STOP_BITS == TWO_STOP_BITS) {
		UxMODEbits.STSEL = 1;
	} else {
		LOG_E("Unrecognised Stop bits configuration\n\r");
	}

	if (UART_2_RX_IDLE_LEVEL == IDLE_LOW) {
		UxMODEbits.RXINV = 0;
	} else if (UART_2_RX_IDLE_LEVEL == IDLE_HIGH) {
		UxMODEbits.RXINV = 1;
	}

	UxSTA  = 0x8410;
#ifdef UART_2_TX
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
#endif
	/*
	 * Desired Baud Rate = FCY/(16 (UxBRG + 1))
	 *
	 * UxBRG = ((FCY/Desired Baud Rate)/16) - 1
	 *
	 * UxBRG = ((CLOCK/UART_2_BAUD)/16) -1
	 *
	 */
	UxBRG = ((CLOCK_FREQ / UART_2_BAUD) / 16) - 1;

#ifdef UART_2_RX
	RX_ISR_FLAG = 0;
	RX_ISR_ENABLE = 1;
#endif
#ifdef UART_2_TX
	TX_ISR_FLAG = 0;
	TX_ISR_ENABLE = 1;
#endif

#endif // __PIC24FJ256GB106__ || __PIC24FJ64GB106__
}

#ifdef UART_2_TX
void uart_2_putchar(u8 ch)
{
//	u8 loop;
//	LOG_D("(0x%x)", ch);
	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */
	if (tx_count || UxSTAbits.UTXBF) {
		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		UxSTAbits.UTXISEL1 = 1;
		UxSTAbits.UTXISEL0 = 0;
		
//		LOG_D("Transmit buffer full add to circular\n\r");
		if (tx_count == UART_2_TX_BUFFER_SIZE) {
			LOG_E("Circular buffer full!");
			return;
		}

		tx_buffer[tx_write_index] = ch;
		tx_write_index = (++tx_write_index % UART_2_TX_BUFFER_SIZE);
		tx_count++;
	} else {
		UxTXREG = ch;
	}
	TX_ISR_ENABLE = 1;
}
#endif // UART_2_TX

#ifdef UART_2_TX
void uart_2_printf(char *string)
{
	char *ptr;

	LOG_D("uart_2_printf(%s)\n\r", string);

	ptr = string;

	while (*ptr) {
		uart_2_putchar(*ptr++);
	}

#ifdef TEST
#ifdef UART_2_TIME_END_OF_LINE
	eol_start_timer();
#endif // UART_2_TIME_END_OF_LINE
#endif // TEST
}
#endif // UART_2_TX

#ifdef UART_2_TX
void uart_2_tx_data(u8 *data, u16 len)
{
	u8 *ptr;
//	u32 loop;

	LOG_D("uart_2_tx_data()\n\r");

	ptr = data;

	while(len--) {
		uart_2_putchar(*ptr++);
	}
	
#if POLLING
	RS485_TX
	while (len) {
		if (!UxSTAbits.UTXBF) {
			UxTXREG = *ptr++;
			len--;
		}
	}

	while(!UxSTAbits.TRMT) {
		Nop();
	}

	RS485_RX
#endif
}
#endif // UART_2_TX

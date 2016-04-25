/**
 *
 * \file es_lib/utils/modbus.c
 *
 * Functions for using a MODBUS Comms.
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
#include "es_lib/timers/timers.h"
#include "es_lib/modbus/modbus.h"

#define TAG "MODBUS"

/*
 * Tx Buffer is a circular buffer
 */
static u8     tx_buffer[MODBUS_TX_BUFFER_SIZE];
static UINT16 tx_write_index = 0;
static UINT16 tx_read_index = 0;
static UINT16 tx_count = 0;

/*
 * Have to keep the 35 timer global in file as it'll be canceled.
 */
static u8 hw_35_timer = BAD_TIMER;
static es_timer resp_timer;


/* Table of CRC values for high?order byte
*/
static u8 crc_high_bytes[] = {
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
} ;

/* Table of CRC values for low?order byte
*/
static u8 crc_low_bytes[] = {
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2,
	0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
	0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
	0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
	0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
	0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6,
	0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
	0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32,
	0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE,
	0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
	0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA,
	0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
	0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
	0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62,
	0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
	0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE,
	0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA,
	0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
	0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76,
	0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
	0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
	0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E,
	0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
	0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A,
	0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86,
	0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
} ;

void start_15_timer(void);
void start_35_timer(void);
static void resp_timeout_expiry_fn(timer_t timer_id, union sigval data);

struct modbus_state modbus_state;

u16 crc_calculate(u8 *data, u16 len)
{
	u8 *ptr = data;
	u8  crc_high = 0xFF; /* high byte of CRC initialized */
	u8  crc_low  = 0xFF; /* low byte of CRC initialized */
	u16 index;           /* will index into CRC lookup table */

	while (len--) {      /* pass through message buffer */
		             /* calculate the CRC */
		asm ("CLRWDT");
		index = crc_high ^ *ptr++;
		crc_high = crc_low ^ crc_high_bytes[index];
		crc_low  = crc_low_bytes[index];
	}
	return (crc_high << 8 | crc_low);
}

u8 crc_check(u8 *data, u16 len)
{
	u16 crc;

	crc = crc_calculate(data, len - 2);

	if (  (((crc >> 8) & 0xff) == data[len - 2])
	    &&((crc & 0xff) == data[len -1]) ) {
		return (TRUE);
	} else {
		return (FALSE);
	}
}

static void hw_35_expiry_function(void)
{
	hw_35_timer = BAD_TIMER;

	if (modbus_state.process_timer_35_expiry) {
		modbus_state.process_timer_35_expiry();
	} else {
		LOG_E("T35 in unknown state\n\r");
	}
}

static void hw_15_expiry_function(void)
{
	if (modbus_state.process_timer_15_expiry) {
		modbus_state.process_timer_15_expiry();
	} else {
		LOG_E("T15 in unknown state\n\r");
	}
}

#if defined(MODBUS_UART_2)
void _ISR __attribute__((__no_auto_psv__)) _U2RXInterrupt(void)
#elif defined(MODBUS_UART_3)
void _ISR __attribute__((__no_auto_psv__)) _U3RXInterrupt(void)
#elif defined(MODBUS_UART_4)
void _ISR __attribute__((__no_auto_psv__)) _U4RXInterrupt(void)
#endif
{
	u8 ch;
	RX_ISR_FLAG = 0;

	asm ("CLRWDT");

	if (UxSTAbits.OERR) {
		LOG_E("RX Buffer overrun\n\r");
		UxSTAbits.OERR = 0;   /* Clear the error flag */
	}

	if (modbus_state.process_rx_character) {
		while (UxSTAbits.URXDA) {
			ch = UxRXREG;
			modbus_state.process_rx_character(ch);
		}
	} else {
		while (UxSTAbits.URXDA) {
			ch = UxRXREG;
			LOG_E("No Rx State! 0x%x\n\r", ch);
		}
	}
}

#if defined(MODBUS_UART_2)
void _ISR __attribute__((__no_auto_psv__)) _U2TXInterrupt(void)
#elif defined(MODBUS_UART_3)
void _ISR __attribute__((__no_auto_psv__)) _U3TXInterrupt(void)
#elif defined(MODBUS_UART_4)
void _ISR __attribute__((__no_auto_psv__)) _U4TXInterrupt(void)
#endif
{
	/*
	 * If the TX buffer is not full load it from the circular buffer
	 */
	while ((!UxSTAbits.UTXBF) && (tx_count)) {
		UxTXREG = tx_buffer[tx_read_index];
		tx_read_index = (++tx_read_index % MODBUS_TX_BUFFER_SIZE);
		tx_count--;
	}

	if (!tx_count) {
		/*
		 * Can't use UxSTAbits.TRMT to detect when Tx Queue is empty
		 * as it ain't reliable at all.
		 */
		if (UxSTAbits.UTXISEL0) {
			while (!UxSTAbits.TRMT) {
				Nop();
			}

			RS485_RX

			if (modbus_state.process_tx_finished) {
				modbus_state.process_tx_finished();
			} else {
				LOG_E("Tx Finished in unknown state\n\r");
			}

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

void start_15_timer()
{
	u8 hw_timer;

	//	PR5 = ((u16)((CLOCK_FREQ / MODBUS_BAUD) * 17) - 1) ;

	hw_timer = hw_timer_start(uSeconds, ((1000000 * 17)/MODBUS_BAUD), FALSE, hw_15_expiry_function);
}

void start_35_timer()
{
	if(hw_35_timer != BAD_TIMER) {
		hw_timer_cancel(hw_35_timer);
	}

//	PR4 = ((u16)((CLOCK_FREQ / MODBUS_BAUD) * 39) - 1) ;
	hw_35_timer = hw_timer_start(uSeconds, ((1000000 * 39)/MODBUS_BAUD), FALSE, hw_35_expiry_function);
}

void modbus_init()
{
	/*
	 * Initialise the 35 timer so it can't be canceled by mistake
	 */
	hw_35_timer = BAD_TIMER;

	/*
	 * Initialise the SW Timer used for response timeout
	 */
	TIMER_INIT(resp_timer);

	/*
	 * Set the starting state.
	 */
	set_modbus_starting_state();

	/*
	 * CinnamonBun is running a PIC24FJ256GB106 processor
	 */
#if defined(__PIC24FJ256GB106__) || defined(__PIC24FJ64GB106__)

	AD1PCFGL = 0xffff;

	switch (MODBUS_RX_PIN) {
		case RP0:
			TRISBbits.TRISB0 = 1;
#if defined(MODBUS_UART_2)
			RPINR19bits.U2RXR = 0;
#elif defined(MODBUS_UART_3)
			RPINR17bits.U3RXR = 0;
#elif defined(MODBUS_UART_4)
			RPINR27bits.U4RXR = 0;
#endif
			break;

		case RP1:
			TRISBbits.TRISB1 = 1;
#if defined(MODBUS_UART_2)
			RPINR19bits.U2RXR = 1;
#elif defined(MODBUS_UART_3)
			RPINR17bits.U3RXR = 1;
#elif defined(MODBUS_UART_4)
			RPINR27bits.U4RXR = 1;
#endif
			break;

		case RP13:
			TRISBbits.TRISB2 = 1;
#if defined(MODBUS_UART_2)
			RPINR19bits.U2RXR = 13;
#elif defined(MODBUS_UART_3)
			RPINR17bits.U3RXR = 13;
#elif defined(MODBUS_UART_4)
			RPINR27bits.U4RXR = 13;
#endif
			break;

		case RP25:
			TRISDbits.TRISD4 = 1;
#if defined(MODBUS_UART_2)
			RPINR19bits.U2RXR = 25;
#elif defined(MODBUS_UART_3)
			RPINR17bits.U3RXR = 25;
#elif defined(MODBUS_UART_4)
			RPINR27bits.U4RXR = 25;
#endif
			break;

		case RP28:
			TRISBbits.TRISB4 = 1;
#if defined(MODBUS_UART_2)
			RPINR19bits.U2RXR = 28;
#elif defined(MODBUS_UART_3)
			RPINR17bits.U3RXR = 28;
#elif defined(MODBUS_UART_4)
			RPINR27bits.U4RXR = 28;
#endif
			break;
	}

	tx_write_index = 0;
	tx_read_index = 0;
	tx_count = 0;

	switch (MODBUS_TX_PIN) {
		case RP0:
			TRISBbits.TRISB0 = 0;
#if defined(MODBUS_UART_2)
			RPOR0bits.RP0R = 5;
#elif defined(MODBUS_UART_3)
			RPOR0bits.RP0R = 28;
#elif defined(MODBUS_UART_4)
			RPOR0bits.RP0R = 30;
#endif
			break;

		case RP1:
			TRISBbits.TRISB1 = 0;
#if defined(MODBUS_UART_2)
			RPOR0bits.RP1R = 5;
#elif defined(MODBUS_UART_3)
			RPOR0bits.RP1R = 28;
#elif defined(MODBUS_UART_4)
			RPOR0bits.RP1R = 30;
#endif
			break;

		case RP13:
			TRISBbits.TRISB2 = 0;
#if defined(MODBUS_UART_2)
			RPOR6bits.RP13R = 5;
#elif defined(MODBUS_UART_3)
			RPOR6bits.RP13R = 28;
#elif defined(MODBUS_UART_4)
			RPOR6bits.RP13R = 30;
#endif
			break;

		case RP28:
			TRISBbits.TRISB4 = 0;
#if defined(MODBUS_UART_2)
			RPOR14bits.RP28R = 5;
#elif defined(MODBUS_UART_3)
			RPOR14bits.RP28R = 28;
#elif defined(MODBUS_UART_4)
			RPOR14bits.RP28R = 30;
#endif
			break;
	}

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
}

void modbus_putchar(u8 ch)
{
	u8 loop;

	/*
	 * If the Transmitter queue is currently empty turn on chip select.
	 */

	if (UxSTAbits.TRMT) {
		RS485_TX
		for(loop = 0; loop < 100; loop++) {
			Nop();
		}
	}

	/*
	 * If either the TX Buffer is full OR there are already characters in
	 * our SW Buffer then add to SW buffer
	 */
	if (tx_count || UxSTAbits.UTXBF) {
		/*
		 * Interrupt when a character is transferred to the Transmit Shift
		 * Register (TSR), and as a result, the transmit buffer becomes empty
		 */
		UxSTAbits.UTXISEL1 = 1;
		UxSTAbits.UTXISEL0 = 0;
		
		if (tx_count == MODBUS_TX_BUFFER_SIZE) {
			LOG_E("Circular buffer full!");
			return;
		}

		tx_buffer[tx_write_index] = ch;
		tx_write_index = (++tx_write_index % MODBUS_TX_BUFFER_SIZE);
		tx_count++;
	} else {
		UxTXREG = ch;
	}
}

void modbus_tx_data(u8 *data, u16 len)
{
	u16      crc;
	u8      *ptr;

	crc = crc_calculate(data, len);
	LOG_D("tx_data crc %x\n\r", crc);

	ptr = data;

	while(len--) {
		modbus_putchar(*ptr++);
	}

	if(MODBUS_ENDIAN == LITTLE_ENDIAN) {
		modbus_putchar((crc & 0xff));
		modbus_putchar((crc >> 8) & 0xff);
	} else {
		modbus_putchar((crc >> 8) & 0xff);
		modbus_putchar((crc & 0xff));
	}
}

result_t modbus_attempt_transmission(u8 *data, u16 len, modbus_response_function fn)
{
	if (modbus_state.transmit) {
		modbus_state.transmit(data, len, fn);
		return(SUCCESS);
	} else {
		LOG_E("Tx Attempted in unknown state\n\r");
		return(ERR_NOT_READY);
	}
}

result_t start_response_timer(u8 address)
{
	u8           ch;
	u16          ticks;
	union sigval timer_data;

	/*
	 * Clear the RX ISR conditions and enable ISR
	 */
	while (UxSTAbits.URXDA) {
		ch = UxRXREG;
	}
	RX_ISR_ENABLE = 1;

	timer_data.sival_int = 0;

	if (address == 0) {
		ticks = MODBUS_RESPONSE_BROADCAST_TIMEOUT;
	} else {
		ticks = MODBUS_RESPONSE_TIMEOUT;
	}
	return(timer_start(ticks,
		           resp_timeout_expiry_fn,
		           timer_data,
		           &resp_timer));

}

result_t cancel_response_timer(void)
{
	return(timer_cancel(&resp_timer));
}

static void resp_timeout_expiry_fn(timer_t timer_id, union sigval data)
{
//	LOG_D("resp_timeout_expiry_fn()\n\r");

	TIMER_INIT(resp_timer);

	if (modbus_state.process_response_timeout) {
		modbus_state.process_response_timeout();
	} else {
		LOG_E("Response Timout in unknown state\n\r");
	}
}

/**
 *
 * \file es_lib/utils/modbus.h
 *
 * Function prototypes for using modbus Comms.
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

#ifdef MODBUS_UART_2
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
#elif defined(MODBUS_UART_3)
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
#elif defined(MODBUS_UART_4)
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


#define MODBUS_READ_CONFIG 0x03
#define MODBUS_READ_DATA   0x04
#define MODBUS_WRITE_DATA  0x06

typedef void (*modbus_response_function)(u8 *data, u8 size);

struct modbus_state
{
    void (*process_timer_15_expiry)(void);
    void (*process_timer_35_expiry)(void);
    void (*transmit)(u8 *data, u16 len, modbus_response_function fn);
    void (*process_tx_finished)(void);
    void (*process_rx_character)(u8 ch);
    void (*process_response_timeout)();
};

extern void set_modbus_starting_state(void);
extern void set_modbus_idle_state(void);
extern void set_modbus_transmitting_state(u8 address, modbus_response_function fn);
extern void set_modbus_awaiting_response_state(u8 address, modbus_response_function fn);

extern void start_15_timer(void);
extern void start_35_timer(void);

extern void modbus_init();
extern void modbus_putchar(u8 ch);
extern void modbus_tx_data(u8 *data, u16 len);
extern result_t modbus_attempt_transmission(u8 *data, u16 len, modbus_response_function fn);

extern u16 crc_calculate(u8 *data, u16 len);
extern u8 crc_check(u8 *data, u16 len);


extern result_t start_response_timer(u8 address);
extern result_t cancel_response_timer(void);

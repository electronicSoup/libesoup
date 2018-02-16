/**
 * \file libesoup/comms/uart/uart.h
 *
 * \brief UART functionality for the electronicSoup Cinnamon Bun
 *
 * Copyright 2017 - 2018 electronicSoup Limited
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
 */
/**
 * \defgroup Uart Uart
 * @{
 * 
 */
#ifndef ES_LIB_UART_H
#define ES_LIB_UART_H

#ifdef SYS_UART

#define UART_8_DATABITS       8 /**< 8 Databits on serial connection */
#define UART_9_DATABITS       9 /**< 9 Databits on serial connection */

#define UART_PARITY_NONE      0 /**< No Parity on serial connection */
#define UART_PARITY_ODD       1 /**< Odd Parity on serial Connection */
#define UART_PARITY_EVEN      2 /**< Even Parity on serial Connection */

#define UART_ONE_STOP_BIT     1 /**< One Stop bit on serial Connection */
#define UART_TWO_STOP_BITS    2 /**< Two Stop bits on serial Connection */

#define UART_IDLE_LOW         0 /**< Serial connection idles high */
#define UART_IDLE_HIGH        1 /**< Serial connection idles low */ 
#if defined(XC16) || defined(__XC8)
#define UART_LITTLE_ENDIAN    0 /**< Serial connection is little endian */
#define UART_BIG_ENDIAN       1 /**< Serial connection is big endian */
#endif  // XC16 || __XC8

#define UART_BAD         0xff   /**< Dummy value for a bad uart identifier */

/**
 * \ingroup Uart
 * \brief  Structure defining a uart configuration
 * \var tx_pin Peripheral pin to use for transmission
 */
struct uart_data {
     ty_peripheral_pin tx_pin; /**< GPIO Pin to be used for the transmit pin of connection \ref */
     ty_peripheral_pin rx_pin; /**< GPIO Pin to be used for the transmit pin of connection \ref */
    uint8_t            uart;        /**< Uart identifier */
    uint16_t           uart_mode;   /**< Mode bits for the connection */
    uint32_t           baud;        /**< Baud rate for the connection */
    void               (*tx_finished)(void *);  /**< Function to be called when a requested transmission has finished */
    void               (*process_rx_char)(uint8_t, uint8_t); /**< Function to be called when a character is received */
};

/**
 * \ingroup Uart
 * \brief Calculate the bit field for the mode of operation for connection
 * 
 * \param mode  The calculated bitfield for the required mode of operation. 
 * \param databits Number of databits required. Either UART_8_DATABITS or 
 * UART_9_DATABITS
 * \param parity Either #define UART_PARITY_NONE, UART_PARITY_ODD or 
 * UART_PARITY_EVEN
 * \param stopbits Either UART_ONE_STOP_BIT or UART_TWO_STOP_BITS
 * \param rx_idle_level Either UART_IDLE_LOW or UART_IDLE_HIGH
 * \return Either ERR_BAD_INPUT_PARAMETER or SUCCESS 
 */
extern result_t uart_calculate_mode(uint16_t *mode, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level);

/**
 * \ingroup Uart
 * \brief Initialise the UART module, auto called by \ref libesoup_init() if \SYS_UART is defined
 * 
 */
extern void     uart_init(void);

#ifdef SYS_DEBUG_BUILD
extern uint16_t uart_buffer_count(struct uart_data *data);
#endif // SYS_DEBUG_BULD

/**
 * \ingroup Uart
 * \brief reserve one of the system UARTs for use. 
 * 
 * \param data structure containing details of the channel to be reserved \ref uart_data
 * \return \ref result_t
 */
extern result_t uart_reserve(struct uart_data *data);

/**
 * \ingroup Uart
 * \brief release a previously reserved system UARTs. 
 * 
 * \param data structure containing details of the reserved channel \ref uart_data
 * \return \ref result_t
 */
extern result_t uart_release(struct uart_data *data);

/**
 * \ingroup Uart
 * \brief transmit a buffer of data on a previously reserved system UARTs. 
 * 
 * \param data structure containing details of the reserved channel \ref uart_data
 * \param buffer array of bytes to be transmitted.
 * \param len length of the buffer to be transmitted
 * \return \ref result_t
 */
extern result_t uart_tx_buffer(struct uart_data *data, uint8_t *buffer, uint16_t len);

/**
 * \ingroup Uart
 * \brief transmit a single byte on a previously reserved system UARTs. 
 * 
 * \param data structure containing details of the reserved channel \ref uart_data
 * \param ch the byte to be transmitted.
 * \return \ref result_t
 */
extern result_t uart_tx_char(struct uart_data *data, char ch);

#endif // SYS_UART

#endif // ES_LIB_UART_H

/**
 * @}
 */

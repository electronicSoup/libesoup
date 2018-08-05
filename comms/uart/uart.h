/**
 * @file libesoup/comms/uart/uart.h
 *
 * @author John Whitmore
 *
 * @brief UART functionality for the electronicSoup Cinnamon Bun
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
 */
/**
 * @defgroup Uart Uart
 * @{
 * 
 */
#ifndef _ES_LIB_UART_H
#define _ES_LIB_UART_H

#include "libesoup_config.h"

#ifdef SYS_UART

#include "libesoup/gpio/gpio.h"
#include "libesoup/gpio/peripheral.h"

#define UART_8_DATABITS       8   /**< 8 Databits on serial connection */
#define UART_9_DATABITS       9   /**< 9 Databits on serial connection */

#define UART_PARITY_NONE      0   /**< No Parity on serial connection */
#define UART_PARITY_ODD       1   /**< Odd Parity on serial Connection */
#define UART_PARITY_EVEN      2   /**< Even Parity on serial Connection */

#define UART_ONE_STOP_BIT     1   /**< One Stop bit on serial Connection */
#define UART_TWO_STOP_BITS    2   /**< Two Stop bits on serial Connection */

#define UART_IDLE_LOW         0   /**< Serial connection idles high */
#define UART_IDLE_HIGH        1   /**< Serial connection idles low */ 
#if defined(XC16) || defined(__XC8)
#define UART_LITTLE_ENDIAN    0   /**< Serial connection is little endian */
#define UART_BIG_ENDIAN       1   /**< Serial connection is big endian */
#endif  // XC16 || __XC8

#define UART_BAD           0xff   /**< Dummy value for a bad uart identifier */

/**
 * @ingroup Uart
 * @struct  uart_data
 * @brief   Structure defining a uart configuration
 * 
 * Applicaton code should populate this structure with the required configuration
 * of the UART before a call to reserve a UART on the target micro-controller.
 * 
 */
struct uart_data {
	enum gpio_pin      tx_pin;                               ///< GPIO Pin for transmit (Possibly INVALID_PIN)
	enum gpio_pin      rx_pin;                               ///< GPIO Pin for receive  (Possibly INVALID_PIN)
	uint8_t            uindex;                               ///< Uart identifier (populated by libesoup code)
	uint16_t           uart_mode;                            ///< Mode bits for the connection (calculated with call to uart_calculate_mode() passing required serial parameters)
	uint32_t           baud;                                 ///< Baud rate for the connection
	void               (*tx_finished)(void *);               ///< Callback - transmission has finished (Possibly NULL)
	void               (*process_rx_char)(uint8_t);          ///< Callback - Character received (If an rx_pin is defined function to process received characters)
};

/**
 * @ingroup Uart
 * @brief   Calculate the bit field for the mode of operation for connection
 * 
 * Serial comms parameters are fairly standard at this stage, although you would
 * go a long time before you find something that's not 8N1 (8 Data bits, No Parity
 * and one stop bit. What is obviously is the Special Function Register (SFR) 
 * settings for the uC you're using and how those comms parameters translate to
 * SFR bit fields. This function takes the parameters and calculates the required
 * SFR bit fields.
 * 
 * @param mode          The calculated bitfield for the required mode of operation. 
 * @param databits      Number of databits required. Either UART_8_DATABITS or UART_9_DATABITS
 * @param parity        Either UART_PARITY_NONE, UART_PARITY_ODD or UART_PARITY_EVEN
 * @param stopbits      Either UART_ONE_STOP_BIT or UART_TWO_STOP_BITS
 * @param rx_idle_level Either UART_IDLE_LOW or UART_IDLE_HIGH
 * 
 * @return              Negative on error, or 0 on Success
 */
extern result_t uart_calculate_mode(uint16_t *mode, uint8_t databits, uint8_t parity, uint8_t stopbits, uint8_t rx_idle_level);

#ifdef SYS_DEBUG_BUILD
/**
 * @ingroup  debug_build
 * @brief    (#ifdef SYS_DEBUG_BUILD) - returns current count of bytes in the Transmit buffer of given uart.
 * 
 * Simple debugging function to return the current count of bytes in the
 * transmit buffer so the filling and emptying of the buffer can be tracked
 * if required.
 * 
 * @param udata         Pointer to uart_data structure for which count is required.
 * 
 * @return              Negative - Error (bad input parameter)
 *                      Positive - Number of bytes in transmit buffer of UART
 */
extern result_t uart_tx_buffer_count(struct uart_data *udata);
#endif // SYS_DEBUG_BULD

/**
 * @ingroup Uart
 * @brief   reserve a system UARTs for use. 
 * 
 * The struct uart_data *udata contains all the relevant information on a comms
 * channel. Using this reserve function and release a single uC UART channel could
 * but multiplexed between a number of sensors. Reserve the UART, transmit and 
 * recieve and then release the UART. Then change the rx_pin and tx_pin of the
 * udata structure and repeat.
 * 
 * @param udata         Pointer to struct uart_data containing uart configuration \ref uart_data
 * @return              Negative on error,
 *                      Positive or 0 - SUCCESS
 */
extern result_t uart_reserve(struct uart_data *udata);

/**
 * @ingroup Uart
 * @brief   release a previously reserved system UART.  
 * 
 * @param udata         pointer structure containing details of the reserved channel \ref uart_data
 * @return              Negative - Error
 *                      Positive - Success
 */
extern result_t uart_release(struct uart_data *udata);

/**
 * @ingroup Uart
 * @brief   transmit a buffer of data on a previously reserved system UARTs. 
 * 
 * @param udata         structure containing details of the previously reserved channel \ref uart_data
 * @param buffer        array of bytes to be transmitted.
 * @param len           length of the buffer to be transmitted
 * @return              Negative - Error
 *                      Positive - Number of bytes added to tx buffers for transmission
 */
extern result_t uart_tx_buffer(struct uart_data *udata, uint8_t *buffer, uint16_t len);

/**
 * @ingroup Uart
 * @brief   transmit a single byte on a previously reserved system UARTs.
 * 
 * @param   udata       structure containing details of the previously reserved channel \ref uart_data
 * @param   ch          the byte to be transmitted.
 * @return              Negative - Error
 *                      Zero     - Success
 */
extern result_t uart_tx_char(struct uart_data *udata, char ch);

#endif // SYS_UART

#endif // _ES_LIB_UART_H

/**
 * @}
 */

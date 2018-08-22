/**
 * @file libesoup/comms/modbus/modbus.h
 *
 * @author John Whitmore
 *
 * Function prototypes for using modbus Comms.
 * 
 * http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf
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
 */
#ifndef _MODBUS_H
#define _MODBUS_H

#include "libesoup_config.h"

#ifdef SYS_MODBUS

#include "libesoup/comms/uart/uart.h"

/**
 * @typedef  modbus_id
 * @brief    This is an overlay type to the result_t return code type.
 *
 * Modbus channel identifier is returned by the 
 */
typedef int16_t modbus_id;

/**
 * @ingroup MODBUS
 * @typedef modbus_response_function - callback function to process received
 *                                     MODBUS frames.
 * 
 * @input   modbus_id chan  MODBUS Channel identifier, of the channel over which
 *                          the frame was received.
 * @input   uint8_t  *frame MODBUS Frame to be processed.
 * @input   uint8_t   len   Length of the received MODBUS frame.
 */
typedef void (*modbus_response_function)(modbus_id chan,
					 uint8_t *frame,
					 uint8_t len);
/**
 * @ingroup MODBUS
 * @fn      modbus_master_reserve
 * 
 * @brief   Function to reserve a MODBUS channel for Master communications.
 *
 * @param   struct uart_data *uart
 *                          Serial communications settings for the channel.
 *                          Baud rate, Rx & Tx pins etc.
 *
 * @param   void (*idle_callback)(modbus_id, uint8_t)
 *                          If the Application wants to be informed when the
 *                          MODBUS channel is idle, and ready to transmit a
 *                          frame, then a callback should be passed to the
 *                          reserve function.
 * 
 * @return  Modbus Channel Identifier, or error on fail.
 */
extern modbus_id modbus_master_reserve(struct uart_data *uart,
				       void (*idle_callback)(modbus_id, uint8_t));

/**
 * @ingroup MODBUS
 * @fn      modbus_slave_reserve
 * 
 * @brief   Function to reserve a MODBUS channel for Slave communications.
 *
 * @param   struct uart_data *uart
 *                          Serial communications settings for the channel.
 *                          Baud rate, Rx & Tx pins etc.
 *
 * @param   void (*idle_callback)(modbus_id, uint8_t)
 *                          If the Application wants to be informed when the
 *                          MODBUS channel is idle, and ready to transmit a
 *                          frame, then a callback should be passed to the
 *                          reserve function.
 * 
 * @param   modbus_response_function frame_callback
 *                          Function to process MODBUS Frames received on the 
 *                          channel.
 * 
 * @return  Modbus Channel Identifier, or error on fail.
 */
extern modbus_id modbus_slave_reserve(struct uart_data *uart,
                                      void (*idle_callback)(modbus_id, uint8_t),
                                      modbus_response_function frame_callback);

/**
 * @ingroup MODBUS
 * @fn      modbus_release
 * 
 * @brief   API Function to relase a MODBUS Channel, which is no longer required
 *          by the application.
 *
 * @param   modbus_id       Identifer of the MODBUS channel to be released.
 *
 * @return  result_t        SUCCESS
 *                          -ERR_BAD_INPUT_PARAMETER
 */
extern result_t  modbus_release(modbus_id id);

extern result_t  modbus_read_config(modbus_id chan,
				    uint8_t modbus_address,
				    uint16_t mem_address,
				    modbus_response_function callback);
#endif //  SYS_MODBUS
#endif //  _MODBUS_H

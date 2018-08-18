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
typedef void (*modbus_response_function)(modbus_id chan, uint8_t *frame, uint8_t len);

/**
 * @ingroup MODBUS
 * @fn      modbus_master_reserve
 * 
 * @brief Enumeration for units of time. Used to specify durations for timers.
 * 
 * 
 * Idle callback function is called so that the application knows when the
 * Modbus channel is free to transmit.
 */
extern result_t modbus_master_reserve(struct uart_data *uart, void (*idle_callback)(modbus_id, uint8_t));
extern result_t modbus_release(uint8_t modbus_id);

extern result_t modbus_read_config(modbus_id chan, uint8_t modbus_address, uint16_t mem_address, modbus_response_function callback);

#endif //  SYS_MODBUS
#endif //  _MODBUS_H

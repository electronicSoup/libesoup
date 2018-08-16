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

#define MODBUS_READ_CONFIG     0x03
#define MODBUS_READ_DATA       0x04
#define MODBUS_WRITE_CONFIG    0x06
#define MODBUS_WRITE_MULTIPLE  0x10
#define MODBUS_ID_REQUEST      0x11

typedef int16_t modbus_id;

/*
 * Idle callback function is called so that the application knows when the
 * Modbus channel is free to transmit.
 */
extern result_t modbus_reserve(struct uart_data *uart, void (*idle_callback)(modbus_id, uint8_t));
extern result_t modbus_release(struct uart_data *uart);

#endif //  SYS_MODBUS
#endif //  _MODBUS_H

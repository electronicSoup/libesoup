/**
 *
 * \file es_lib/logger/serial_port.h
 *
 * Definitions for configuration of the Serial Port
 *
 * Copyright 2014 John Whitmore <jwhitmore@electronicsoup.com>
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
#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

/*
 *  serial_init()
 *
 * This function is only defined for Code compiled for the Microchip
 * microcontroller so the switch MCP must be defined as part of your
 * Project.
 * 
 * In additon the system.h config file should define the baud rate to
 * be used and the physical pin configuration of the 3 pin serial port:
 * GndRxTx or GndTxRx
 *
 * for example : #define SERIAL_BAUD 19200
 *
 * And either : #define SERIAL_PORT_GndTxRx or #define SERIAL_PORT_GndRxTx
 *
 */
#if defined(MCP)
extern void serial_init(void);
#endif

#endif // SERIAL_PORT_H

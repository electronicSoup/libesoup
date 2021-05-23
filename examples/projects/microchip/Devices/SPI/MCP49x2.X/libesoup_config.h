/**
 *
 * @file libesoup/examples/projects/microchip/SerialLogging.X/libesoup_config.h
 *
 * @author John Whitmore
 *
 * Copyright 2020 electronicSoup Limited
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _LIBESOUP_CONFIG_H
#define _LIBESOUP_CONFIG_H

#define SYS_EXAMPLE_DEVICE_MCP29x2

#define SYS_CLOCK_FREQ (7370000/2)

#define SYS_TEST_BUILD

#define SYS_HW_TIMERS
#define SYS_SW_TIMERS
#ifdef SYS_SW_TIMERS
#define SYS_NUMBER_OF_SW_TIMERS    10
#define SYS_SW_TIMER_TICK_ms        5
#endif

#define SYS_SERIAL_LOGGING

#if defined(SYS_SERIAL_LOGGING)

//#define SYS_SERIAL_PORT_GndTxRx
#define SYS_SERIAL_PORT_GndRxTx
#define SYS_LOG_LEVEL               LOG_DEBUG
#define SYS_UART1
#define SYS_SERIAL_LOGGING_BAUD     19200
#define SYS_UART_TX_BUFFER_SIZE     300

#endif // defined(SYS_SERIAL_LOGGING)

#define SYS_SPI1

/*
 * Include a board file
 */
#include "board.h"

/*
 *******************************************************************************
 *
 * Project Specific Defines
 *
 *******************************************************************************
 */

#endif // _LIBESOUP_CONFIG_H
